//#include "Perlin3D.h"

#include <random>

template <typename T>
Matrix3f Perlin3D::perlinPass(unsigned int size, T & generator, float max)
{
	Matrix3f mat(Nz::Vector3ui(size, size, size));

	std::uniform_real_distribution<float> d(-max, max);

	for (auto & v : mat)
		v = d(generator);

	return mat;
}