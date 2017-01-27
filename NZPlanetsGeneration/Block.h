#ifndef BLOCK_H
#define BLOCK_H
#pragma once

#include <vector>
#include "SpherePoint.h"

struct Border;
template <typename T>
struct Block;

struct Point
{
	Point(const SpherePoint & _pos) : pos(_pos) { }

	SpherePoint pos;
	std::vector<unsigned int> borders;
	std::vector<unsigned int> blocks;
};

struct Border
{
	Border(unsigned int b1, unsigned int b2, unsigned int p1, unsigned int p2)
		: block1(b1)
		, block2(b2)
		, point1(p1)
		, point2(p2)
	{ }

	unsigned int block1;
	unsigned int block2;
	unsigned int point1;
	unsigned int point2;

	bool operator ==(const Border & other);
};

template <typename T>
struct Block
{
	Block(const SpherePoint & _pos) : pos(_pos) { }

	SpherePoint pos;

	std::vector<unsigned int> points;
	std::vector<unsigned int> borders;
	T data;
};

struct Triangle
{
	Triangle(unsigned int b1, unsigned int b2, unsigned int b3)
		: block1(b1)
		, block2(b2)
		, block3(b3)
	{ }

	unsigned int block1;
	unsigned int block2;
	unsigned int block3;

	bool operator==(const Triangle & other);
};

struct BlockInfo
{
	BlockInfo() : height(0), moisture(0) {}

	float height;
	float moisture;
};

//#include "block.tpp"

#endif // BLOCK_H
