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

	std::cout << points.size() << std::endl;
}

Planet createWorld(WorldMakerData data)
{
	//compute preconditions
	assert(!data.biomes.empty());
	auto oceanBiomeIt(std::find_if(data.biomes.begin(), data.biomes.end(), [](const auto & b) {return b.type() == BiomeType::OCEAN; }));
	auto lakeBiomeIt(std::find_if(data.biomes.begin(), data.biomes.end(), [](const auto & b) {return b.type() == BiomeType::LAKE; }));

	if (oceanBiomeIt == data.biomes.end() || lakeBiomeIt == data.biomes.end())
		data.haveWater = false;
	if (data.waterLevel < 0)
		data.waterLevel = 0;
	if (data.waterLevel > 1)
		data.waterLevel = 1;

	unsigned int oceanBiomeIndex(std::distance(data.biomes.begin(), oceanBiomeIt));
	unsigned int lakeBiomeIndex(std::distance(data.biomes.begin(), lakeBiomeIt));

	data.biomes.push_back(Biome(0, 0, Nz::Color::Black, BiomeType::NONE));
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

	if (data.haveWater)
	{
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
	}

	return p;
}