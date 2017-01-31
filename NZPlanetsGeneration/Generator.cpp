#include "generator.h"
#include "sphericaldistribution.h"
#include <random>
#include <vector>
#include <deque>
#include <array>
#include <iostream>
#include <cassert>
#include <algorithm>

template <typename T>
SphereSurface<float> perlinPass(T engine, float max, unsigned int pointCount)
{
	std::uniform_real_distribution<float> d(-max, max);
	SphericalDistribution<float> pitchDistrib;
	std::uniform_real_distribution<float> yawDistrib(0, 2 * float(M_PI));

	SphereSurface<float> s;

	for (unsigned int i(0); i < pointCount; i++)
		s.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)), d(engine));

	relax(s);
	s.buildMap();
	return s;
}

SphereSurface<float> perlin(const PerlinData & data)
{
	std::mt19937 engine(data.seed);
	if (data.passCount == 0)
		return SphereSurface<float>();

	std::vector<SphereSurface<float>> surfaces;
	unsigned int pointCount(data.pointCount);
	float value(data.amplitude);
	for (unsigned int i(0); i < data.passCount; i++)
	{
		surfaces.push_back(perlinPass(engine, value, pointCount));
		std::cout << "pass " << i << " done / " << pointCount << " points" << std::endl;
		value /= data.passDivisor;
		pointCount *= data.passPointMultiplier;
	}

	std::cout << "Stack pass ..." << std::endl;

	SphereSurface<float> s(surfaces.back());
	for (auto it2(s.blocksBegin()); it2 != s.blocksEnd(); it2++)
		it2->data = 0;

	surfaces.pop_back();
	unsigned int index(0);
	for (const auto & surface : surfaces)
	{
		for (auto it(surface.trianglesBegin()); it != surface.trianglesEnd(); it++)
		{
			const SphereBlock<float> & b1(*std::next(surface.blocksBegin(), it->block1));
			const SphereBlock<float> & b2(*std::next(surface.blocksBegin(), it->block2));
			const SphereBlock<float> & b3(*std::next(surface.blocksBegin(), it->block3));

			Nz::Vector3f pos1(toVector3(b1.pos));
			Nz::Vector3f pos2(toVector3(b2.pos));
			Nz::Vector3f pos3(toVector3(b3.pos));

			for (auto it2(s.blocksBegin()); it2 != s.blocksEnd(); it2++)
			{
				Nz::Vector3f pos(toVector3(it2->pos));
				auto i(intersect(pos1, pos2, pos3, Nz::Vector3f(0, 0, 0), pos));
				if (!i.first)
					continue;

				Nz::Vector3f p(proportions(pos1, pos2, pos3, i.second));
				it2->data += p.x*b1.data + p.y*b2.data + p.z*b3.data;
			}
		}
		std::cout << index++ << " stacked" << std::endl;
	}
	std::cout << "end" << std::endl;
	return s;
}

void placeWaterBiome(Planet & p, unsigned int startPoint, unsigned int oceanBiomeIndex, unsigned int lakeBiomeIndex, unsigned int maxLakeSize, float waterLevel)
{
	std::deque<unsigned int> toUpdateList;
	toUpdateList.push_back(startPoint);
	std::vector<unsigned int> points;

	while (!toUpdateList.empty())
	{
		unsigned int index = toUpdateList.front();

		auto point(std::next(p.blocksBegin(), index));
		for (unsigned int triangleIndex : point->triangles)
		{
			auto triangle(std::next(p.trianglesBegin(), triangleIndex));
			std::array<unsigned int, 3> indexs{ triangle->block1, triangle->block2, triangle->block3 };
			for (auto i : indexs)
			{
				auto block(std::next(p.blocksBegin(), i));
				if (block->data.height > waterLevel)
					continue;

				if (std::find(toUpdateList.begin(), toUpdateList.end(), i) == toUpdateList.end() && std::find(points.begin(), points.end(), i) == points.end())
					toUpdateList.push_back(i);
			}
		}

		points.push_back(index);
		toUpdateList.pop_front();
	}

	unsigned int biomeIndex(lakeBiomeIndex);
	if (points.size() > maxLakeSize)
		biomeIndex = oceanBiomeIndex;
	for (unsigned int point : points)
		std::next(p.blocksBegin(), point)->data.biomeIndex = biomeIndex;
}

void createElevation(Planet & p)
{
	std::deque<unsigned int> toUpdateList;
	std::vector<unsigned int> points;

	float unit = sqrt(std::distance(p.blocksBegin(), p.blocksEnd()));

	for (unsigned int index(0); index < p.blockCount(); index++)
	{
		auto point(p.block(index));
		if (p.biome(point.data.biomeIndex).type() != BiomeType::NONE)
			continue;

		bool toAdd(false);
		for (unsigned int triangleIndex : point.triangles)
		{
			auto& triangle(p.triangle(triangleIndex));
			std::array<unsigned int, 3> indexs{ triangle.block1, triangle.block2, triangle.block3 };
			for (auto i : indexs)
			{
				auto& block(p.block(i));
				if (p.biome(block.data.biomeIndex).type() == BiomeType::OCEAN)
				{
					toAdd = true;
					toUpdateList.push_back(i); //add water near of ground
				}
			}
		}

		if (toAdd)
			toUpdateList.push_back(index); //add ground near of water
	}

	if (toUpdateList.empty())
	{
		unsigned int index(std::distance(p.blocksBegin(), std::min_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })));
		p.block(index).data.height = 0;
		toUpdateList.push_back(index);
		points.push_back(index);
	}

	while (!toUpdateList.empty())
	{
		unsigned int index = toUpdateList.front();

		auto& point(p.block(index));
		const Biome& currentBiome(p.biome(point.data.biomeIndex));

		std::vector<unsigned int> connectedPoints;
		std::vector<unsigned int> validConnectedPoints;
		for (unsigned int triangleIndex : point.triangles)
		{
			auto& triangle(p.triangle(triangleIndex));
			std::array<unsigned int, 3> indexs{ triangle.block1, triangle.block2, triangle.block3 };
			for (unsigned int i : indexs)
			{
				if (std::find(connectedPoints.begin(), connectedPoints.end(), i) != connectedPoints.end())
					continue;
				connectedPoints.push_back(i);
				if (std::find(points.begin(), points.end(), i) != points.end())
					validConnectedPoints.push_back(i);
			}
		}

		bool found(false);
		bool contactWithOtherBiome(false);
		unsigned int bestIndex(0);
		SphereBlock<BlockInfo>* bestBlock(nullptr);
		float bestElevation(0);
		Nz::Vector3f pos(toVector3(point.pos));

		for (unsigned int i : validConnectedPoints)
		{
			if (i == index)
				continue;

			auto & block(p.block(i));
			const auto & biome(p.biome(block.data.biomeIndex));
			float length((pos - toVector3(block.pos)).GetSquaredLength());

			if (currentBiome.type() == BiomeType::OCEAN)
			{
				float elevation(block.data.height - sqrt(length)*unit);
				if (!found || elevation > bestElevation)
				{
					found = true;
					bestBlock = &block;
					bestIndex = i;
					bestElevation = elevation;
					continue;
				}
			}
			else if (currentBiome.type() == BiomeType::NONE)
			{
				float elevation(block.data.height + sqrt(length)*unit);
				if (!found || elevation < bestElevation)
				{
					found = true;
					bestBlock = &block;
					bestIndex = i;
					bestElevation = elevation;
					continue;
				}
			}
			else if (currentBiome.type() == BiomeType::LAKE)
			{
				if (!found || block.data.height < bestElevation)
				{
					found = true;
					bestBlock = &block;
					bestIndex = i;
					bestElevation = block.data.height;
					continue;
				}
			}
		}

		for (unsigned int i : connectedPoints)
		{
			if (i == index)
				continue;

			if (std::find(validConnectedPoints.begin(), validConnectedPoints.end(), 1) != validConnectedPoints.end())
				continue;

			auto & block(p.block(index));
			const auto & biome(p.biome(block.data.biomeIndex));
			float length((pos - toVector3(block.pos)).GetSquaredLength());

			if (currentBiome.type() == BiomeType::OCEAN)
			{
				if (biome.type() == BiomeType::NONE)
				{
					if (!found || !contactWithOtherBiome || length < (pos - toVector3(bestBlock->pos)).GetSquaredLength())
					{
						found = true;
						bestBlock = &block;
						bestIndex = i;
						bestElevation = -sqrt(length)*unit / 2;
					}
					contactWithOtherBiome = true;
					continue;
				}
			}
			else if (currentBiome.type() == BiomeType::NONE)
			{
				if (biome.type() == BiomeType::OCEAN)
				{
					if (!found || !contactWithOtherBiome || length < (pos - toVector3(bestBlock->pos)).GetSquaredLength())
					{
						found = true;
						bestBlock = &block;
						bestIndex = i;
						bestElevation = sqrt(length)*unit / 2;
					}
					contactWithOtherBiome = true;
					continue;
				}
			}
		}

		point.data.height = bestElevation;
		for (unsigned int i : connectedPoints)
		{
			if (std::find(points.begin(), points.end(), i) == points.end() && std::find(toUpdateList.begin(), toUpdateList.end(), i) == toUpdateList.end())
			{
				toUpdateList.push_back(i);
				continue;
			}
			auto& other(p.block(i));
			auto& biome(p.biome(other.data.biomeIndex));
			if (biome.type() == BiomeType::LAKE)
			{
				if (other.data.height > point.data.height)
					toUpdateList.push_back(i);
			}
			else if (biome.type() == BiomeType::OCEAN)
			{
				if (point.data.height - (pos - toVector3(other.pos)).GetLength()*unit > other.data.height)
					toUpdateList.push_back(i);
			}
			else
			{
				if (point.data.height + (pos - toVector3(other.pos)).GetLength()*unit < other.data.height)
					toUpdateList.push_back(i);
			}
		}

		if (std::find(points.begin(), points.end(), index) == points.end())
			points.push_back(index);
		toUpdateList.pop_front();
	}
}

void placeRivers(Planet & p, unsigned int count, unsigned int seed)
{
	std::mt19937 engine(seed);
	std::uniform_int_distribution<unsigned int> d(0, p.blockCount());

	for (unsigned int i(0); i < count; i++)
	{
		River r;
		for (unsigned int i(0); i < 10; i++)
		{
			unsigned int index(d(engine));
			auto & block(p.block(index));
			auto & biome(p.biome(block.data.biomeIndex));
			if (biome.type() == BiomeType::NONE)
			{
				r.push_back(index);
				break;
			}
		}
		if (r.empty())
			continue;

		while (true)
		{
			unsigned int bestIndex(0);
			float bestHeight(std::numeric_limits<float>::max());
			std::vector<unsigned int> connectedPoints;
			for (const auto & triangle : p.block(r.back()).triangles)
			{
				const auto & t(p.triangle(triangle));
				std::array<unsigned int, 3> indexs{ t.block1, t.block2, t.block3 };
				for (unsigned int i : indexs)
				{
					if (std::find(connectedPoints.begin(), connectedPoints.end(), i) != connectedPoints.end())
						continue;
					connectedPoints.push_back(i);
				}
			}

			for (auto i : connectedPoints)
			{
				const auto & b(p.block(i));
				if (b.data.height < bestHeight)
				{
					bestIndex = i;
					bestHeight = b.data.height;
				}
			}
			if (bestHeight > p.block(r.back()).data.height)
				break;
			r.push_back(bestIndex);
			const auto & b(p.block(bestIndex));
			if (p.biome(b.data.biomeIndex).type() == BiomeType::OCEAN || p.biome(b.data.biomeIndex).type() == BiomeType::LAKE)
				break;
		}
		p.addRiver(r);
	}
}

void createMoisture(Planet & p)
{
	std::deque<unsigned int> toUpdateList;
	std::vector<unsigned int> points;

	for (unsigned int i(0); i < p.riverCount(); i++)
		for (unsigned int index : p.river(i))
			if (std::find(toUpdateList.begin(), toUpdateList.end(), index) == toUpdateList.end())
				toUpdateList.push_back(index);

	while (!toUpdateList.empty())
	{
		unsigned int index = toUpdateList.front();

		std::vector<unsigned int> connectedPoints;
		for (const auto & triangle : p.block(index).triangles)
		{
			const auto & t(p.triangle(triangle));
			std::array<unsigned int, 3> indexs{ t.block1, t.block2, t.block3 };
			for (unsigned int i : indexs)
			{
				if (std::find(connectedPoints.begin(), connectedPoints.end(), i) != connectedPoints.end())
					continue;
				if (std::find(points.begin(), points.end(), i) != points.end())
					continue;
				connectedPoints.push_back(i);
			}
		}

		bool found(false);
		float bestMoisture(std::numeric_limits<float>::max());
		auto & block(p.block(index));
		Nz::Vector3f pos(toVector3(block.pos));
		for (unsigned int i : connectedPoints)
		{
			const auto & b(p.block(i));
			const auto & biome(p.biome(b.data.biomeIndex));
			if (biome.type() != BiomeType::NONE)
				continue;
			float moisture(b.data.moisture + (toVector3(b.pos) - pos).GetLength());
			if (moisture < bestMoisture)
			{
				found = true;
				bestMoisture = moisture;
			}
		}

		if (found)
			block.data.moisture = bestMoisture;
		else block.data.moisture = 0;

		points.push_back(index);
		toUpdateList.pop_front();

		
	}
	float maxMoisture = std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.moisture < b.data.moisture; })->data.moisture;
	float minMoisture = std::min_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.moisture < b.data.moisture; })->data.moisture;

	for (unsigned int i(0) ; i < p.blockCount() ; i++)
	{
		auto & block(p.block(i));
		if (std::find(points.begin(), points.end(), i) == points.end())
			block.data.moisture = maxMoisture;

		block.data.moisture = 1 - ((block.data.moisture - minMoisture) / (maxMoisture - minMoisture));
	}
}

void createBiomes(Planet & p)
{
	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto & block(p.block(i));
		const auto & biome(p.biome(block.data.biomeIndex));
		if (biome.type() != BiomeType::NONE)
			continue;

		block.data.biomeIndex = p.nearestBiomeID(block.data.height, block.data.moisture);
	}
}

Planet createWorld(WorldMakerData data)
{
	//compute preconditions
	assert(!data.biomes.empty());
	auto oceanBiomeIt(std::find_if(data.biomes.begin(), data.biomes.end(), [](const auto & b) {return b.type() == BiomeType::OCEAN; }));
	unsigned int oceanBiomeIndex(std::distance(data.biomes.begin(), oceanBiomeIt));
	if (oceanBiomeIt == data.biomes.end())
		data.biomes.push_back(Biome(0, 0, BiomeType::OCEAN, RandomColor(Nz::Color::Black)));
	auto lakeBiomeIt(std::find_if(data.biomes.begin(), data.biomes.end(), [](const auto & b) {return b.type() == BiomeType::LAKE; }));
	unsigned int lakeBiomeIndex(std::distance(data.biomes.begin(), lakeBiomeIt));
	if(lakeBiomeIt == data.biomes.end())
		data.biomes.push_back(Biome(0, 0, BiomeType::LAKE, RandomColor(Nz::Color::Black)));

	if (oceanBiomeIt == data.biomes.end() || lakeBiomeIt == data.biomes.end())
		data.haveWater = false;
	if (data.waterLevel < 0.1)
		data.waterLevel = 0.1;
	if (data.waterLevel > 0.9)
		data.waterLevel = 0.9;
	if (!data.haveWater)
	{
		data.waterLevel = 0.5f;
		data.maxLakeSize = 0;
	}

	data.biomes.push_back(Biome(0, 0, BiomeType::NONE, RandomColor(Nz::Color::Red)));
	unsigned int noBiomeIndex(data.biomes.size() - 1);
	//-----

	PerlinData perlinData(data.seed);
	perlinData.passCount = 2;
	perlinData.passDivisor = 1000;
	perlinData.passPointMultiplier = data.pointsCount / data.carvingLevel;
	perlinData.pointCount = data.carvingLevel;
	perlinData.amplitude = 0.1f;
	SphereSurface<float> noise(perlin(perlinData));
	
	Planet p(Planet::clone(noise, data.biomes, BlockInfo(0, 0, noBiomeIndex)));
	for (unsigned int i(0); i < std::distance(p.blocksBegin(), p.blocksEnd()); i++)
		std::next(p.blocksBegin(), i)->data.height = std::next(noise.blocksBegin(), i)->data;

	float realWaterHeight;

	{
		std::vector<unsigned int> sortedNoizePointsByElevation;
		for (unsigned int i(0); i < (unsigned int)(std::distance(noise.blocksBegin(), noise.blocksEnd())); i++)
			sortedNoizePointsByElevation.push_back(i);
		std::sort(sortedNoizePointsByElevation.begin(), sortedNoizePointsByElevation.end(), [&noise](unsigned int a, unsigned int b)
		{
			return std::next(noise.blocksBegin(), a)->data < std::next(noise.blocksBegin(), b)->data;
		});
		unsigned int index(data.waterLevel*sortedNoizePointsByElevation.size());
		if (index >= sortedNoizePointsByElevation.size())
			index = sortedNoizePointsByElevation.size() - 1;
		realWaterHeight = std::next(noise.blocksBegin(), sortedNoizePointsByElevation[index])->data;
	}

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (it->data.height < realWaterHeight && it->data.biomeIndex == noBiomeIndex)
			placeWaterBiome(p, std::distance(p.blocksBegin(), it), oceanBiomeIndex, lakeBiomeIndex, data.maxLakeSize, realWaterHeight);
	}

	createElevation(p);
	float min = std::min_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })->data.height;
	float max = std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })->data.height;

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (!data.haveWater)
		{
			it->data.height = std::pow((it->data.height - min) / (max - min), data.elevationAmplification)*data.maxHeight;
			it->data.biomeIndex = noBiomeIndex;
		}
		else
		{
			if (it->data.height < 0)
			{
				it->data.height /= -min;
				it->data.height = -std::pow(-it->data.height, data.waterDepthAmplification);
				it->data.height *= data.maxDepth;

			}
			else
			{
				it->data.height /= max;
				it->data.height = std::pow(it->data.height, data.elevationAmplification);
				it->data.height *= data.maxHeight;
			}
		}
		
	}

	placeRivers(p, data.rivierCount, data.seed);

	createMoisture(p);

	createBiomes(p);

	return p;
}