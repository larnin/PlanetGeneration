#pragma once

#include "Planet.h"
#include <Nazara/Math/Vector3.hpp>
#include <vector>

struct WorldMakerData2
{
	inline WorldMakerData2(unsigned int _pointCount, unsigned int _carvingLevel)
		: pointsCount(_pointCount)
		, carvingLevel(_carvingLevel)
		, haveWater(true)
		, maxLakeSize(_pointCount / 100)
		, waterLevel(0.5f)
		, rivierCount(10)
		, elevationAmplification(1.0f)
		, waterDepthAmplification(1.0f)
		, maxHeight(0.1f)
		, maxDepth(0.1f)
	{}

	unsigned int pointsCount;
	unsigned int carvingLevel;
	std::vector<Biome> biomes;
	bool haveWater;
	unsigned int maxLakeSize;
	float waterLevel;
	unsigned int rivierCount;
	float elevationAmplification;
	float waterDepthAmplification;
	float maxHeight;
	float maxDepth;
};

class Generator2
{
public:
	Generator2(const WorldMakerData2 & datas);

	inline void setDatas(const WorldMakerData2 & datas) { if(!m_generating) m_datas = datas; }

	Planet create(unsigned int seed);

private:
	void initializeData();
	Planet makePerlin(unsigned int seed) const;
	float realWaterHeight(const Planet & p) const;
	void placeWaterBiomes(Planet & p, float waterHeight) const;
	void indexPoints(const Planet & p);
	void createElevation(Planet & p) const;
	void adaptElevation(Planet & p) const;
	void createRivers(Planet & p, unsigned int seed) const;
	void createMoisture(Planet & p) const;
	void createTemperature(Planet & p) const;
	void createBiomes(Planet & p) const;

	WorldMakerData2 m_datas;
	bool m_generating;

	unsigned int m_oceanBiomeIndex;
	unsigned int m_lakeBiomeIndex;
	unsigned int m_noBiomeIndex;

	std::vector<Nz::Vector3f> m_points;
};
