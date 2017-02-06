#pragma once

#include "Planet.h"
#include <Nazara/Math/Vector3.hpp>
#include <vector>
#include <random>

enum class NoiseType
{
	SIMPLEX,
	PERLIN
};

struct WorldMakerData
{
	inline WorldMakerData(unsigned int _pointCount, float _scale)
		: pointsCount(_pointCount)
		, subdivisons(4)
		, scale(_scale)
		, haveWater(true)
		, maxLakeSize(_pointCount / 100)
		, waterLevel(0.5f)
		, rivierCount(10)
		, elevationAmplification(1.0f)
		, waterDepthAmplification(1.0f)
		, maxHeight(0.1f)
		, maxDepth(0.1f)
		, planetSize(1.0f)
	{}

	unsigned int pointsCount;
	unsigned int subdivisons;
	float scale;
	std::vector<Biome> biomes;
	bool haveWater;
	unsigned int maxLakeSize;
	float waterLevel;
	unsigned int rivierCount;
	float elevationAmplification;
	float waterDepthAmplification;
	float maxHeight;
	float maxDepth;
	float planetSize;

};

class Generator
{
public:
	Generator(const WorldMakerData & datas);

	inline void setDatas(const WorldMakerData & datas) { if(!m_generating) m_datas = datas; }

	Planet create(unsigned int seed);

private:
	void initializeData(unsigned int seed);
	Planet initializePlanet(unsigned int seed);
	void makePerlin(Planet & p);
	float realWaterHeight(const Planet & p) const;
	void placeWaterBiomes(Planet & p, float waterHeight) const;
	void indexPoints(const Planet & p);
	void createElevation(Planet & p) const;
	void adaptElevation(Planet & p) const;
	void createRivers(Planet & p, unsigned int seed) const;
	void createMoisture(Planet & p) const;
	void createTemperature(Planet & p) const;
	void createBiomes(Planet & p) const;
	void cleanData();

	WorldMakerData m_datas;
	bool m_generating;

	unsigned int m_oceanBiomeIndex;
	unsigned int m_lakeBiomeIndex;
	unsigned int m_noBiomeIndex;

	std::vector<Nz::Vector3f> m_points;
	std::mt19937 m_generator;
};

