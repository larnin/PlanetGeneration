#ifndef BLOCK_H
#define BLOCK_H
#pragma once

#include <vector>
#include "SpherePoint.h"

template <typename T>
struct SphereBlock
{
	inline SphereBlock(const SpherePoint & _pos, const T & _value = T()) : pos(_pos) , data(_value) { }

	SpherePoint pos;
	T data;

	std::vector<unsigned int> triangles;
};

struct SphereTriangle
{
	inline SphereTriangle(unsigned int b1, unsigned int b2, unsigned int b3)
		: block1(b1)
		, block2(b2)
		, block3(b3)
	{ }

	unsigned int block1;
	unsigned int block2;
	unsigned int block3;

	bool operator==(const SphereTriangle & other);
};

struct SphereLine
{
	inline SphereLine(unsigned int b1, unsigned int b2)
		: block1(b1)
		, block2(b2)
	{}

	unsigned int block1;
	unsigned int block2;

	bool operator==(const SphereLine & other);
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

struct RivierPoint
{
	inline RivierPoint(unsigned int _index, float _width) : index(_index), width(_width) {}

	unsigned int index;
	float width;
};

//#include "block.tpp"

#endif // BLOCK_H
