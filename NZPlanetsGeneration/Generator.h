
#ifndef GENERATOR_H
#define GENERATOR_H
#pragma once

#include "SphereSurface.h"

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

#endif // GENERATOR_H