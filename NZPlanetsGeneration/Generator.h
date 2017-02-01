
#ifndef GENERATOR_H
#define GENERATOR_H
#pragma once

#include "SphereSurface.h"
#include "Biome.h"
#include "Planet.h"
#include <vector>

struct PerlinData
{
	PerlinData(unsigned int _seed)
		: seed(_seed)
		, passCount(6)
		, pointCount(20)
		, passPointMultiplier(2)
		, passDivisor(2)
		, amplitude(1)
	{}

	unsigned int seed;
	unsigned int passCount;
	unsigned int pointCount;
	float passPointMultiplier;
	float passDivisor;
	float amplitude;
};

SphereSurface<float> perlin(const PerlinData & data);

struct WorldMakerData
{
	WorldMakerData(unsigned int _seed, unsigned int _pointCount, unsigned int _carvingLevel)
		: seed(_seed)
		, pointsCount(_pointCount)
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

	unsigned int seed;
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

Planet createWorld(WorldMakerData data);

#endif // GENERATOR_H