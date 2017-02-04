#include "Generator.h"
#include "Perlin3D.h"
#include "SphericalDistribution.h"
#include <stdexcept>
#include <deque>

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

	initializeData();

	std::cout << "Initialized : " << c.GetSeconds() << std::endl;

	Planet p(initializePlanet(seed));

	std::cout << "Planet created : " << c.GetSeconds() << std::endl;

	indexPoints(p);

	std::cout << "Points indexed : " << c.GetSeconds() << std::endl;

	makePerlin(seed, p);

	std::cout << "Perlin elevation : " << c.GetSeconds() << std::endl;

	placeWaterBiomes(p, realWaterHeight(p));

	std::cout << "Water done : " << c.GetSeconds() << std::endl;

	createElevation(p);

	std::cout << "Elevation done : " << c.GetSeconds() << std::endl;

	adaptElevation(p);

	std::cout << "Elevation adapted : " << c.GetSeconds() << std::endl;

	createRivers(p, seed);

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

void Generator::initializeData()
{
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
		m_datas.waterLevel = 0.1;
	if (m_datas.waterLevel > 0.9)
		m_datas.waterLevel = 0.9;
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

	std::mt19937 engine(seed);
	SphericalDistribution<float> pitchDistrib;
	std::uniform_real_distribution<float> yawDistrib(0, 2 * float(M_PI));

	for (unsigned int i(0); i < m_datas.pointsCount; i++)
		p.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)), BlockInfo(0, 0, 0, m_noBiomeIndex));
	p.buildMap();
	return p;
}

void Generator::makePerlin(unsigned int seed, Planet & p) const
{
	PerlinData data(seed);
	data.amplitude = 1;
	data.baseSize = m_datas.carvingLevel;
	data.passCount = 3;
	data.passDivisor = 3;
	Perlin3D perlin(Nz::Vector3f::Zero(), Nz::Vector3f(1.1f, 1.1f, 1.1f), data);

	for (unsigned int i(0); i < p.blockCount(); i++)
		p.block(i).data.height = perlin(m_points[i]);
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
	unsigned int index(m_datas.waterLevel*sortedNoizePointsByElevation.size());
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

void Generator::createElevation(Planet & p) const
{
	assert(m_points.size() == p.blockCount());

	for (auto it(p.blocksBegin()); it != p.blocksEnd(); it++)
		it->data.height = 0;

	std::vector<unsigned int> points;

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
			
			if (std::find(points.begin(), points.end(), i2) != points.end())
				continue;

			float bestHeight2(std::numeric_limits<float>::lowest());
			for (unsigned int i3 : p.connectedBlocks(i2))
			{
				auto& block3(p.block(i3));
				if (p.biome(block3.data.biomeIndex).type() != BiomeType::NONE)
					continue;
				float height(-(m_points[i2] - m_points[i3]).GetLength() / 2);
				if (height > bestHeight2)
					bestHeight2 = height;
			}

			points.push_back(i2);
		}
		if (toAdd)
		{
			block.data.height = bestHeight;
			points.push_back(i);
		}
	}

	std::vector<unsigned int> toUpdateList;
	for (unsigned int i : points)
		for (unsigned int i2 : p.connectedBlocks(i))
			if (std::find(toUpdateList.begin(), toUpdateList.end(), i2) == toUpdateList.end() && std::find(points.begin(), points.end(), i2) == points.end())
				toUpdateList.push_back(i2);
	assert(!toUpdateList.empty());

	while (!toUpdateList.empty())
	{
		unsigned int i(toUpdateList.front());
		auto& b(p.block(i));
		auto& biome(p.biome(b.data.biomeIndex));

		bool found(false);
		float bestHeight(0);
		bool doNotMoveThis(false);

		auto connectedBlocks(p.connectedBlocks(i));
		connectedBlocks.erase(std::remove_if(connectedBlocks.begin(), connectedBlocks.end(), [&points](unsigned int i)
		{
			return std::find(points.begin(), points.end(), i) == points.end();
		}), connectedBlocks.end());

		for (unsigned int i2 : connectedBlocks)
		{
			auto& b2(p.block(i2));
			auto & biome2(p.biome(b2.data.biomeIndex));

			if (biome.type() == BiomeType::LAKE)
			{
				if (!found || b2.data.height < bestHeight)
				{
					found = true;
					bestHeight = b2.data.height;
				}
			}
			else if (biome.type() == BiomeType::OCEAN)
			{
				if (biome2.type() != BiomeType::OCEAN)
				{
					doNotMoveThis = true;
					break;
				}

				float height(b2.data.height - (m_points[i] - m_points[i2]).GetLength());
				if (!found || height > bestHeight)
				{
					found = true;
					bestHeight = height;
				}
			}
			else //NONE
			{
				if (biome2.type() == BiomeType::OCEAN)
				{
					doNotMoveThis = true;
					break;
				}

				float height(b2.data.height + (m_points[i] - m_points[i2]).GetLength());
				if (!found || height < bestHeight)
				{
					found = true;
					bestHeight = height;
				}
			}
		}

		if (doNotMoveThis)
		{
			if (std::find(points.begin(), points.end(), i) == points.end())
				points.push_back(i);
			toUpdateList.erase(toUpdateList.begin());
			continue;
		}
		b.data.height = bestHeight;

		for (unsigned int i2 : connectedBlocks)
		{
			auto& b2(p.block(i2));
			auto & biome2(p.biome(b2.data.biomeIndex));

			if (biome2.type() == BiomeType::LAKE)
			{
				if (biome.type() != BiomeType::LAKE)
					continue;
				if (b2.data.height > b.data.height)
					toUpdateList.push_back(i2);
			}
			else if (biome2.type() == BiomeType::OCEAN)
			{
				float height(b.data.height - (m_points[i] - m_points[i2]).GetLength());
				if (height > b2.data.height)
					toUpdateList.push_back(i2);
			}
			else //NONE
			{
				float height(b.data.height + (m_points[i] - m_points[i2]).GetLength());
				if (height < b2.data.height)
					toUpdateList.push_back(i2);
			}
		}

		for (unsigned int i2 : p.connectedBlocks(i))
			if (std::find(toUpdateList.begin(), toUpdateList.end(), i2) == toUpdateList.end() && std::find(points.begin(), points.end(), i2) == points.end())
				toUpdateList.push_back(i2);

		if(std::find(points.begin(), points.end(), i) == points.end())
			points.push_back(i);
		toUpdateList.erase(toUpdateList.begin());
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

void Generator::createRivers(Planet & p, unsigned int seed) const
{
	std::mt19937 engine(seed);
	std::uniform_int_distribution<unsigned int> d(0, p.blockCount());

	for (unsigned int i(0); i < m_datas.rivierCount; i++)
	{
		River r;
		for (unsigned int i(0); i < 10; i++)
		{
			unsigned int index(d(engine));
			if (p.biome(p.block(index).data.biomeIndex).type() == BiomeType::NONE)
			{
				r.push_back(index);
				break;
			}
		}
		if (r.empty())
			continue;

		while (true)
		{
			const auto& b(p.block(r.back()));
			const auto& biome(p.biome(b.data.biomeIndex));
			if (biome.type() == BiomeType::OCEAN || biome.type() == BiomeType::LAKE)
				break;

			unsigned int bestIndex(0);
			float bestHeight(std::numeric_limits<float>::max());
			for (unsigned int i : p.connectedBlocks(r.back()))
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
			r.push_back(bestIndex);
			if (std::find(r.begin(), r.end(), bestIndex) != r.end())
				break;
		}
		p.addRiver(r);
	}
}

void Generator::createMoisture(Planet & p) const
{
	std::vector<unsigned int> points;
	std::vector<unsigned int> toUpdateList;

	for (unsigned int i(0); i < p.riverCount(); i++)
		for (unsigned int index : p.river(i))
		{
			if (std::find(points.begin(), points.end(), index) != points.end())
				continue;
			points.push_back(index);
			for (unsigned int index2 : p.connectedBlocks(index))
				if (std::find(toUpdateList.begin(), toUpdateList.end(), index2) == toUpdateList.end() && std::find(points.begin(), points.end(), index2) == points.end())
					toUpdateList.push_back(i);
		}

	while (!toUpdateList.empty())
	{
		unsigned int index(toUpdateList.front());

		auto& block(p.block(index));
		if (p.biome(block.data.biomeIndex).type() != BiomeType::NONE)
		{
			toUpdateList.erase(toUpdateList.begin());
			continue;
		}

		bool found(false);
		float bestMoisture(std::numeric_limits<float>::max());
		for (unsigned int i : p.connectedBlocks(index))
		{
			if (std::find(points.begin(), points.end(), i) != points.end())
			{
				const auto& block2(p.block(i));
				float moisture(block2.data.moisture + (m_points[index] - m_points[i]).GetLength());
				if (moisture < bestMoisture)
				{
					bestMoisture = moisture;
					found = true;
				}
			}
			else if (std::find(toUpdateList.begin(), toUpdateList.end(), i) == toUpdateList.end())
				toUpdateList.push_back(i);
		}
		if (found)
		{
			block.data.moisture = bestMoisture;
		}

		toUpdateList.erase(toUpdateList.begin());
		points.push_back(index);
	}

	float maxMoisture = std::max_element(p.blocksBegin(), p.blocksEnd(), [](const auto & a, const auto & b) {return a.data.moisture < b.data.moisture; })->data.moisture;

	for (unsigned int i(0); i < p.blockCount(); i++)
	{
		auto& block(p.block(i));
		if (std::find(points.begin(), points.end(), i) == points.end())
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