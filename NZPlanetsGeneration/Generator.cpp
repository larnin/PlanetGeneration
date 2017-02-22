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

	/*std::mt19937 engine(seed);
	SphericalDistribution<float> pitchDistrib;
	std::uniform_real_distribution<float> yawDistrib(0, 2 * float(M_PI));

	for (unsigned int i(0); i < m_datas.pointsCount; i++)
		p.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)), BlockInfo(0, 0, 0, m_noBiomeIndex));
	p.buildMap();*/
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
			
			/*if (std::find(points.begin(), points.end(), i2) != points.end())
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

			points.push_back(i2);*/
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
					/*if (b.data.biomeIndex != m_noBiomeIndex)
						continue;*/
					//float height(sqrt((m_points[index] - m_points[point]).GetSquaredLength())*perlinValues[perlinIndex]);
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
				std::cout << minimum << " " << p.block(i).data.height << std::endl;

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
	
	/*for (unsigned int i(perlinValues.size()) ; i < points.size() ; i++)
		perlinValues.push_back(perlin.Get(m_points[points[i]].x, m_points[points[i]].y, m_points[points[i]].z, m_datas.groundScale)*0.4f + 0.5f);*/
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

	//std::vector<unsigned int> toUpdateList;
	//for (unsigned int i : points)
	//	for (unsigned int i2 : p.connectedBlocks(i))
	//		if (std::find(toUpdateList.begin(), toUpdateList.end(), i2) == toUpdateList.end() && std::find(points.begin(), points.end(), i2) == points.end())
	//			toUpdateList.push_back(i2);
	//assert(!toUpdateList.empty());

	//while (!toUpdateList.empty())
	//{
	//	unsigned int i(toUpdateList.front());
	//	auto& b(p.block(i));
	//	auto& biome(p.biome(b.data.biomeIndex));

	//	bool found(false);
	//	float bestHeight(0);
	//	bool doNotMoveThis(false);

	//	auto connectedBlocks(p.connectedBlocks(i));
	//	connectedBlocks.erase(std::remove_if(connectedBlocks.begin(), connectedBlocks.end(), [&points](unsigned int i)
	//	{
	//		return std::find(points.begin(), points.end(), i) == points.end();
	//	}), connectedBlocks.end());

	//	for (unsigned int i2 : connectedBlocks)
	//	{
	//		auto& b2(p.block(i2));
	//		auto & biome2(p.biome(b2.data.biomeIndex));

	//		if (biome.type() == BiomeType::LAKE)
	//		{
	//			if (!found || b2.data.height < bestHeight)
	//			{
	//				found = true;
	//				bestHeight = b2.data.height;
	//			}
	//		}
	//		else if (biome.type() == BiomeType::OCEAN)
	//		{
	//			if (biome2.type() != BiomeType::OCEAN)
	//			{
	//				doNotMoveThis = true;
	//				break;
	//			}

	//			float height(b2.data.height - (m_points[i] - m_points[i2]).GetLength());
	//			if (!found || height > bestHeight)
	//			{
	//				found = true;
	//				bestHeight = height;
	//			}
	//		}
	//		else //NONE
	//		{
	//			if (biome2.type() == BiomeType::OCEAN)
	//			{
	//				doNotMoveThis = true;
	//				break;
	//			}

	//			float height(b2.data.height + (m_points[i] - m_points[i2]).GetLength());
	//			if (!found || height < bestHeight)
	//			{
	//				found = true;
	//				bestHeight = height;
	//			}
	//		}
	//	}

	//	if (doNotMoveThis)
	//	{
	//		if (std::find(points.begin(), points.end(), i) == points.end())
	//			points.push_back(i);
	//		toUpdateList.erase(toUpdateList.begin());
	//		continue;
	//	}
	//	b.data.height = bestHeight;

	//	for (unsigned int i2 : connectedBlocks)
	//	{
	//		auto& b2(p.block(i2));
	//		auto & biome2(p.biome(b2.data.biomeIndex));

	//		if (biome2.type() == BiomeType::LAKE)
	//		{
	//			if (biome.type() != BiomeType::LAKE)
	//				continue;
	//			if (b2.data.height > b.data.height)
	//				toUpdateList.push_back(i2);
	//		}
	//		else if (biome2.type() == BiomeType::OCEAN)
	//		{
	//			float height(b.data.height - (m_points[i] - m_points[i2]).GetLength());
	//			if (height > b2.data.height)
	//				toUpdateList.push_back(i2);
	//		}
	//		else //NONE
	//		{
	//			float height(b.data.height + (m_points[i] - m_points[i2]).GetLength());
	//			if (height < b2.data.height)
	//				toUpdateList.push_back(i2);
	//		}
	//	}

	//	for (unsigned int i2 : p.connectedBlocks(i))
	//		if (std::find(toUpdateList.begin(), toUpdateList.end(), i2) == toUpdateList.end() && std::find(points.begin(), points.end(), i2) == points.end())
	//			toUpdateList.push_back(i2);

	//	if(std::find(points.begin(), points.end(), i) == points.end())
	//		points.push_back(i);
	//	toUpdateList.erase(toUpdateList.begin());
	//}
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
	std::mt19937 engine(m_generator);
	std::uniform_int_distribution<unsigned int> d(0, p.blockCount()-1);

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
			if (std::find(r.begin(), r.end(), bestIndex) != r.end())
			{
				r.push_back(bestIndex);
				break;
			}
			r.push_back(bestIndex);
		}
		p.addRiver(r);
	}
}

void Generator::createMoisture(Planet & p) const
{
	std::vector<unsigned int> points;

	for (unsigned int i(0); i < p.riverCount(); i++)
		for (unsigned int index : p.river(i))
		{
			if (std::find(points.begin(), points.end(), index) != points.end())
				continue;
			points.push_back(index);
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