#ifndef PERLIN3D_H
#define PERLIN3D_H
#pragma once

#include "Matrix3.h"
#include <Nazara/Math/Vector3.hpp>
#include <vector>

struct PerlinData2
{
	PerlinData2(unsigned int _seed)
		: seed(_seed)
		, passCount(6)
		, baseSize(2)
		, passPointMultiplier(2)
		, passDivisor(2)
		, amplitude(1)
	{}

	unsigned int seed;
	unsigned int passCount;
	unsigned int baseSize;
	float passPointMultiplier;
	float passDivisor;
	float amplitude;
};

class Perlin3D
{
	using Matrix3f = Matrix3<float>;
public:
	Perlin3D(const Nz::Vector3f & center, const Nz::Vector3f & size, const PerlinData2 & data);

	float operator()(const Nz::Vector3f & pos);

private:
	float at(const Matrix3f & mat, const Nz::Vector3f & pos) const;

	Nz::Vector3f m_center;
	Nz::Vector3f m_size;

	Matrix3<float> m_matrix;
};

#endif // ! PERLIN3D_H