#include "Generator.h"
#include "SphericalDistribution.h"
#include <stdexcept>
#include <deque>
#include <Nazara/Noise/Perlin.hpp>

#include <iostream>
#include <Nazara/Core/Clock.hpp>

Generator::Generator(const WorldMakerData & datas)
	: m_datas(datas)
	, m_generating(false)
	, m_oceanBiomeIndex(0)
	, m_lakeBiomeIndex(0)
	, m_noBiomeIndex(0)
{

}

Planet Generator::create(unsigned int seed)
{
	if (m_generating)
		throw std::runtime_error("The generator still work");
	m_generating = true;
	
	Nz::Clock c;

	initializeData(seed);

	std::cout << "Initialized : " << c.GetSeconds() << std::endl;

	Planet p(initializePlanet(seed));

	std::cout << "Planet created : " << c.GetSeconds() << std::endl;
	std::cout << "Points : " << p.blockCount() << std::endl;

	indexPoints(p);

	std::cout << "Points indexed : " << c.GetSeconds() << std::endl;

	makePerlin(p);

	std::cout << "Perlin elevation : " << c.GetSeconds() << std::endl;

	placeWaterBiomes(p, realWaterHeight(p));

	std::cout << "Water done : " << c.GetSeconds() << std::endl;

	createElevation(p);

	std::cout << "Elevation done : " << c.GetSeconds() << std::endl;

	adaptElevation(p);

	std::cout << "Elevation adapted : " << c.GetSeconds() << std::endl;

	createRivers(p);

	std::cout << "Rivers done : " << c.GetSeconds() << std::endl;

	createMoisture(p);

	std::cout << "Moisture done : " << c.GetSeconds() << std::endl;

	createTemperature(p);

	std::cout << "Temperature done : " << c.GetSeconds() << std::endl;

	createBiomes(p);

	std::cout << "Biomes done : " << c.GetSeconds() << std::endl;

	cleanData();
	m_generating = false;

	std::cout << "Total : " << c.GetSeconds() << std::endl;

	return p;
}

void Generator::initializeData(unsigned int seed)
{
	m_generator = std::mt19937(seed);
	m_points.clear();

	auto oceanBiomeIt(std::find_if(m_datas.biomes.begin(), m_datas.biomes.end(), [](const auto & b) {return b.type() == BiomeType::OCEAN; }));
	m_oceanBiomeIndex = std::distance(m_datas.biomes.begin(), oceanBiomeIt);
	if (oceanBiomeIt == m_datas.biomes.end())
	{
		m_datas.biomes.push_back(Biome(0, 0, BiomeType::OCEAN, RandomColor(Nz::Color::Black)));
		m_datas.haveWater = false;
	}
	auto lakeBiomeIt(std::find_if(m_datas.biomes.begin(), m_datas.biomes.end(), [](const auto & b) {return b.type() == BiomeType::LAKE; }));
	m_lakeBiomeIndex = std::distance(m_datas.biomes.begin(), lakeBiomeIt);
	if (lakeBiomeIt == m_datas.biomes.end())
	{
		m_datas.biomes.push_back(Biome(0, 0, BiomeType::LAKE, RandomColor(Nz::Color::Black)));
		m_datas.haveWater = false;
	}

	if (m_datas.waterLevel < 0.1)
		m_datas.waterLevel = 0.1f;
	if (m_datas.waterLevel > 0.9)
		m_datas.waterLevel = 0.9f;
	if (!m_datas.haveWater)
	{
		m_datas.waterLevel = 0.5f;
		m_datas.maxLakeSize = 0;
	}

	m_datas.biomes.push_back(Biome(0, 0, BiomeType::NONE, RandomColor(Nz::Color::Red)));
	m_noBiomeIndex = m_datas.biomes.size() - 1;

	assert(!m_datas.biomes.empty());
}

Planet Generator::initializePlanet(unsigned int seed)
{
	Planet p(m_datas.planetSize, m_datas.biomes);

	makeRegular(p, m_datas.subdivisons, BlockInfo(0, 0, 0, m_noBiomeIndex));
	return p;
}

void Generator::makePerlin(Planet & p)
{
	Nz::Perlin noise(m_generator());
	Nz::Perlin noise2(m_generator());
	Nz::Perlin noise3(m_generator());

	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		p.block(i).data.height = noise.Get(m_points[i].x, m_points[i].y, m_points[i].z, m_datas.scale);
		p.block(i).data.height += noise2.Get(m_points[i].x, m_points[i].y, m_points[i].z, m_datas.scale*2)/2;
		p.block(i).data.height += noise3.Get(m_points[i].x, m_points[i].y, m_points[i].z, m_datas.scale*4)/4;
	}

}

float Generator::realWaterHeight(const Planet & p) const
{
	std::vector<unsigned int> sortedNoizePointsByElevation;
	for (unsigned int i(0); i < p.blockCount(); i++)
		sortedNoizePointsByElevation.push_back(i);
	std::sort(sortedNoizePointsByElevation.begin(), sortedNoizePointsByElevation.end(), [&p](unsigned int a, unsigned int b)
	{
		return p.block(a).data.height < p.block(b).data.height;
	});
	unsigned int index((unsigned int)(m_datas.waterLevel*sortedNoizePointsByElevation.size()));
	if (index >= sortedNoizePointsByElevation.size())
		index = sortedNoizePointsByElevation.size() - 1;
	return p.block(sortedNoizePointsByElevation[index]).data.height;
}

void Generator::placeWaterBiomes(Planet & p, float waterHeight) const
{
	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (it->data.height > waterHeight || it->data.biomeIndex != m_noBiomeIndex)
			continue;

		std::vector<unsigned int> toUpdateList;
		toUpdateList.push_back(std::distance(p.blocksBegin(), it));
		std::vector<unsigned int> points;

		while (!toUpdateList.empty())
		{
			unsigned int posInList(toUpdateList.size() - 1);
			unsigned int index(toUpdateList.back());
			for (unsigned int i : p.connectedBlocks(index))
			{
				auto block(p.block(i));
				if (block.data.height > waterHeight || block.data.biomeIndex != m_noBiomeIndex)
					continue;
				if (std::find(toUpdateList.begin(), toUpdateList.end(), i) == toUpdateList.end() && std::find(points.begin(), points.end(), i) == points.end())
					toUpdateList.push_back(i);
			}
			points.push_back(index);
			std::swap(toUpdateList[posInList], toUpdateList.back());
			toUpdateList.pop_back();
		}

		unsigned int biomeIndex(m_lakeBiomeIndex);
		if (points.size() > m_datas.maxLakeSize)
			biomeIndex = m_oceanBiomeIndex;
		for (unsigned int point : points)
			p.block(point).data.biomeIndex = biomeIndex;
	}
}

void Generator::indexPoints(const Planet & p)
{
	m_points.clear();

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
		m_points.push_back(toVector3(it->pos));
}

void Generator::createElevation(Planet & p)
{
	assert(m_points.size() == p.blockCount());

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
		it->data.height = 0;

	std::vector<unsigned int> points;

	// --- coasts---
	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto& block(p.block(i));
		if (p.biome(block.data.biomeIndex).type() != BiomeType::NONE)
			continue;
		bool toAdd(false);
		float bestHeight(std::numeric_limits<float>::max());

		for (unsigned int i2 : p.connectedBlocks(i))
		{
			auto& block2(p.block(i2));
			if (p.biome(block2.data.biomeIndex).type() != BiomeType::OCEAN)
				continue;
			toAdd = true;
			float height((m_points[i] - m_points[i2]).GetLength() / 2);
			if (height < bestHeight)
				bestHeight = height;
		}
		if (toAdd)
		{
			block.data.height = bestHeight;
			points.push_back(i);
		}
	}

	// --- lakes ---
	std::vector<float> perlinValues;
	Nz::Perlin perlin(m_generator());
	for (unsigned int point : points)
		perlinValues.push_back(perlin.Get(m_points[point].x, m_points[point].y, m_points[point].z, m_datas.groundScale)*0.5f + 0.5f);

	bool lakeHeightChanged(false);
	do
	{
		lakeHeightChanged = false;
		std::vector<unsigned int> updatedLake;

		for (unsigned int i(0); i < p.blockCount(); i++)
		{
			if (p.block(i).data.biomeIndex != m_lakeBiomeIndex)
				continue;
			if (std::find(updatedLake.begin(), updatedLake.end(), i) != updatedLake.end())
				continue;
			bool onPoints(std::find(points.begin(), points.end(), i) != points.end());

			std::vector<unsigned int> lake;
			std::vector<unsigned int> toUpdate{ i };
			float minimum(std::numeric_limits<float>::max());
			while (!toUpdate.empty())
			{
				unsigned int index(toUpdate.front());
				for (unsigned int connected : p.connectedBlocks(index))
				{
					if (p.block(connected).data.biomeIndex != m_lakeBiomeIndex)
						continue;
					if (std::find(toUpdate.begin(), toUpdate.end(), connected) != toUpdate.end() || std::find(lake.begin(), lake.end(), connected) != lake.end())
						continue;
					toUpdate.push_back(connected);
				}
				unsigned int perlinIndex(0);
				for (unsigned int point : points)
				{
					auto & b(p.block(point));
					float height(p.block(point).data.height + sqrt((m_points[index] - m_points[point]).GetSquaredLength())*perlinValues[perlinIndex]);
					if (height < minimum)
						minimum = height;

					perlinIndex++;
				}
				toUpdate.erase(toUpdate.begin());
				lake.push_back(index);
			}

			if (!onPoints || minimum < p.block(i).data.height)
			{
				lakeHeightChanged = true;

				for (unsigned point : lake)
				{
					p.block(point).data.height = minimum;
					updatedLake.push_back(point);
					if (!onPoints)
					{
						perlinValues.push_back(perlin.Get(m_points[point].x, m_points[point].y, m_points[point].z, m_datas.groundScale)*0.4f + 0.5f);
						points.push_back(point);
					}

				}
			}
			else
			{
				for (unsigned point : lake)
					updatedLake.push_back(point);
			}
		}
	}while (lakeHeightChanged);

	// --- elevation ---
	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		if (std::find(points.begin(), points.end(), i) != points.end())
			continue;
		auto & b(p.block(i));

		float dist(std::numeric_limits<float>::max());
		unsigned int bestID(0);

		unsigned int index(0);
		for (unsigned int point : points)
		{
			if (!(b.data.biomeIndex != m_noBiomeIndex && p.block(point).data.biomeIndex == m_lakeBiomeIndex))
			{
				float d;
				if (b.data.biomeIndex == m_noBiomeIndex)
					d = p.block(point).data.height + sqrt((m_points[i] - m_points[point]).GetSquaredLength())*perlinValues[index];
				else d = (m_points[i] - m_points[point]).GetSquaredLength()*perlinValues[index];
				if (d < dist)
				{
					dist = d;
					bestID = point;
				}
			}
			index++;
		}

		if(b.data.biomeIndex == m_oceanBiomeIndex)
			b.data.height = p.block(bestID).data.height - sqrt(dist);
		else b.data.height = dist;
	}

	Nz::Perlin elevationNoise(m_generator());
	Nz::Perlin elevationNoise2(m_generator());
	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto & b(p.block(i));
		if (b.data.biomeIndex == m_lakeBiomeIndex)
			continue;
		b.data.height *= perlin.Get(m_points[i].x, m_points[i].y, m_points[i].z, m_datas.elevationNoiseScale) *m_datas.elevationNoiseAmplitude + 1;
		b.data.height *= perlin.Get(m_points[i].x, m_points[i].y, m_points[i].z, m_datas.elevationNoiseScale * 2) *m_datas.elevationNoiseAmplitude / 2 + 1;
	}
}

void Generator::adaptElevation(Planet & p) const
{
	float min(std::min_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })->data.height);
	float max(std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })->data.height);

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (!m_datas.haveWater)
		{
			it->data.height = std::pow((it->data.height - min) / (max - min), m_datas.elevationAmplification)*m_datas.maxHeight;
			it->data.biomeIndex = m_noBiomeIndex;
		}
		else if (it->data.height < 0)
		{
			it->data.height /= -min;
			it->data.height = -std::pow(-it->data.height, m_datas.waterDepthAmplification);
			it->data.height *= m_datas.maxDepth;

		}
		else
		{
			it->data.height /= max;
			it->data.height = std::pow(it->data.height, m_datas.elevationAmplification);
			it->data.height *= m_datas.maxHeight;
		}
	}
}

void Generator::createRivers(Planet & p)
{
	struct RiverConnexion
	{
		RiverConnexion(unsigned int _from, unsigned int _to, unsigned int _index) : from(_from), to(_to), index(_index) {}

		unsigned int from;
		unsigned int to;
		unsigned int index;
	};

	std::mt19937 engine(m_generator());
	std::uniform_int_distribution<unsigned int> d(0, p.blockCount() - 1);
	std::vector<RiverConnexion> connexions; 

	for (unsigned int i(0); i < m_datas.rivierCount; i++)
	{
		River r;
		for (unsigned int i(0); i < 10; i++)
		{
			unsigned int index(d(engine));
			if (p.isOnRiver(index))
				continue;
			if (p.biome(p.block(index).data.biomeIndex).type() == BiomeType::NONE)
			{
				r.emplace_back(index, 0);
				break;
			}
		}
		if (r.empty())
			continue;

		while (true)
		{
			unsigned int index(r.back().index);
			const auto& b(p.block(index));
			const auto& biome(p.biome(b.data.biomeIndex));
			if (biome.type() == BiomeType::OCEAN)
				break;
			if (biome.type() == BiomeType::LAKE)
			{
				float lowerHeight(std::numeric_limits<float>::max());
				unsigned int lowerIndex(0);
				unsigned int connectedIndex(0);
				for (unsigned int lake : connectedPointsOfBiome(p, index))
					for (unsigned int i : p.connectedBlocks(lake))
					{
						float height(p.block(i).data.height);
						if (height < lowerHeight)
						{
							lowerHeight = height;
							lowerIndex = i;
							connectedIndex = lake;
						}
					}
				if (p.isOnRiver(connectedIndex))
					index = connectedIndex;
				else
				{
					p.addRiver(r);
					float width(r.back().width);
					r.clear();
					r.emplace_back(connectedIndex, width);
					connexions.emplace_back(p.riverCount() - 1, p.riverCount(), connectedIndex);
				}
			}
			if (p.isOnRiver(index))
			{
				unsigned int blockOnRiverIndex(index);
				unsigned int riverIndex(p.riverIndexAt(blockOnRiverIndex));
				bool stillOnRiver(true);
				while (stillOnRiver)
				{
					bool found(false);
					for (auto & point : p.river(riverIndex))
					{
						if (point.index == r.back().index)
							found = true;
						if (found)
							point.width += r.back().width;
					}
					auto it(std::find_if(connexions.begin(), connexions.end(), [riverIndex](const auto & c) {return c.from == riverIndex; }));
					if (it != connexions.end())
					{
						riverIndex = it->to;
						blockOnRiverIndex = it->index;
					}
					else stillOnRiver = false;
				}
				break;
				connexions.emplace_back(p.riverCount(), p.riverIndexAt(index), index);
			}
			unsigned int bestIndex(0);
			float bestHeight(std::numeric_limits<float>::max());
			for (unsigned int i : p.connectedBlocks(r.back().index))
			{
				const auto& b2(p.block(i));
				if (b2.data.height < bestHeight)
				{
					bestIndex = i;
					bestHeight = b2.data.height;
				}
			}
			if (bestHeight > b.data.height)
				break;
			r.emplace_back(bestIndex, r.back().width + (m_points[r.back().index] - m_points[bestIndex]).GetLength());
			if (std::find_if(r.begin(), r.end(), [bestIndex](const auto & point) {return point.index == bestIndex; }) != r.end())
				break;
		}
		p.addRiver(r);
	}
}

void Generator::createMoisture(Planet & p) const
{
	std::vector<unsigned int> points;

	for (unsigned int i(0); i < p.riverCount(); i++)
		for (const auto & point : p.river(i))
		{
			if (std::find(points.begin(), points.end(), point.index) != points.end())
				continue;
			points.push_back(point.index);
		}

	std::vector<unsigned int> computedPoints(points);

	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto & b(p.block(i));
		if (b.data.biomeIndex == m_oceanBiomeIndex)
			continue;

		if (std::find(points.begin(), points.end(), i) != points.end())
			continue;

		float dist(std::numeric_limits<float>::max());
		unsigned int bestID(0);

		for (unsigned int point : points)
		{
			float d((m_points[i] - m_points[point]).GetSquaredLength());
			if (d < dist)
			{
				dist = d;
				bestID = point;
			}
		}

		b.data.moisture = p.block(bestID).data.moisture + sqrt(dist);
		computedPoints.push_back(i);
	}

	float maxMoisture = std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.moisture < b.data.moisture; })->data.moisture;

	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto& block(p.block(i));
		if (std::find(computedPoints.begin(), computedPoints.end(), i) == computedPoints.end())
			block.data.moisture = maxMoisture;
		block.data.moisture = 1 - (block.data.moisture / maxMoisture);
	}
}

void Generator::createTemperature(Planet & p) const
{
	float max(std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.height < b.data.height; })->data.height);

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (it->data.height <= 0)
			it->data.temperature = 0;
		else it->data.temperature = it->data.height / max;
	}
}

void Generator::createBiomes(Planet & p) const
{
	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
	{
		if (p.biome(it->data.biomeIndex).type() != BiomeType::NONE)
			continue;
		it->data.biomeIndex = p.nearestBiomeID(it->data.temperature, it->data.moisture);
	}
}

void Generator::cleanData()
{
	m_points.clear();
	m_points.shrink_to_fit();
	m_oceanBiomeIndex = 0;
	m_lakeBiomeIndex = 0;
	m_noBiomeIndex = 0;
}

std::vector<unsigned int> Generator::connectedPointsOfBiome(const Planet & p, unsigned int index)
{
	unsigned int biomeIndex(p.block(index).data.biomeIndex);
	std::vector<unsigned int> points;
	std::vector<unsigned int> toDo{ index };

	while (!toDo.empty())
	{
		unsigned int i(toDo.front());
		toDo.erase(toDo.begin());

		if (p.block(i).data.biomeIndex != biomeIndex)
			continue;
		for (auto nextBlock : p.connectedBlocks(i))
		{
			if (std::find(points.begin(), points.end(), nextBlock) != points.end() || std::find(toDo.begin(), toDo.end(), nextBlock) != toDo.end())
				continue;
			toDo.push_back(nextBlock);
		}
		points.push_back(i);
	}
	return points;
}