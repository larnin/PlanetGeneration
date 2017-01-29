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
	BlockInfo() : height(0), moisture(0), biomeIndex(0) {}
	BlockInfo(float _height, float _moisture, unsigned int _biomeIndex)
		: height(_height)
		, moisture(_moisture)
		, biomeIndex(_biomeIndex)
	{}

	float height;
	float moisture;
	unsigned int biomeIndex;
};

//#include "block.tpp"

#endif // BLOCK_H
