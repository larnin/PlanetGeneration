#ifndef BLOCK_H
#define BLOCK_H
#pragma once

#include <vector>
#include "SpherePoint.h"

template <typename T>
struct SphereBlock
{
	SphereBlock(const SpherePoint & _pos) : pos(_pos) { }

	SpherePoint pos;

	T data;

	std::vector<unsigned int> triangles;
};

struct SphereTriangle
{
	SphereTriangle(unsigned int b1, unsigned int b2, unsigned int b3)
		: block1(b1)
		, block2(b2)
		, block3(b3)
	{ }

	unsigned int block1;
	unsigned int block2;
	unsigned int block3;

	bool operator==(const SphereTriangle & other);
};

struct BlockInfo
{
	inline BlockInfo() : height(0), moisture(0), temperature(0), biomeIndex(0) {}
	inline BlockInfo(float _height, float _moisture, float _temperature, unsigned int _biomeIndex)
		: height(_height)
		, moisture(_moisture)
		, temperature(_temperature)
		, biomeIndex(_biomeIndex)
	{}

	float height;
	float moisture;
	float temperature;
	unsigned int biomeIndex;
};

//#include "block.tpp"

#endif // BLOCK_H
