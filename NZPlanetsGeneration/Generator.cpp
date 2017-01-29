#include "generator.h"
#include "sphericaldistribution.h"
#include <random>
#include <vector>
#include <iostream>

template <typename T>
SphereSurface<float> perlinPass(T engine, float max, unsigned int pointCount)
{
	std::uniform_real_distribution<float> d(-max, max);
	SphericalDistribution<float> pitchDistrib;
	std::uniform_real_distribution<float> yawDistrib(0, 2 * float(M_PI));

	SphereSurface<float> s;

	for (unsigned int i(0); i < pointCount; i++)
		s.addBlock(SpherePoint(yawDistrib(engine), pitchDistrib(engine)), d(engine));

	relax(s);
	s.buildMap();
	return s;
}

SphereSurface<float> perlin(const PerlinData & data)
{
	std::mt19937 engine(data.seed);
	if (data.passCount == 0)
		return SphereSurface<float>();

	std::vector<SphereSurface<float>> surfaces;
	unsigned int pointCount(data.pointCount);
	float value(data.amplitude);
	for (unsigned int i(0); i < data.passCount; i++)
	{
		surfaces.push_back(perlinPass(engine, value, pointCount));
		std::cout << "pass " << i << " done / " << pointCount << " points" << std::endl;
		value /= data.passDivisor;
		pointCount *= data.passPointMultiplier;
	}

	std::cout << "Stack pass ..." << std::endl;

	SphereSurface<float> s(surfaces.back());
	for (auto it2(s.blocksBegin()); it2 != s.blocksEnd(); it2++)
		it2->data = 0;

	surfaces.pop_back();
	unsigned int index(0);
	for (const auto & surface : surfaces)
	{
		for (auto it(surface.trianglesBegin()); it != surface.trianglesEnd(); it++)
		{
			const SphereBlock<float> & b1(*std::next(surface.blocksBegin(), it->block1));
			const SphereBlock<float> & b2(*std::next(surface.blocksBegin(), it->block2));
			const SphereBlock<float> & b3(*std::next(surface.blocksBegin(), it->block3));

			Nz::Vector3f pos1(toVector3(b1.pos));
			Nz::Vector3f pos2(toVector3(b2.pos));
			Nz::Vector3f pos3(toVector3(b3.pos));

			for (auto it2(s.blocksBegin()); it2 != s.blocksEnd(); it2++)
			{
				Nz::Vector3f pos(toVector3(it2->pos));
				auto i(intersect(pos1, pos2, pos3, Nz::Vector3f(0, 0, 0), pos));
				if (!i.first)
					continue;

				Nz::Vector3f p(proportions(pos1, pos2, pos3, i.second));
				it2->data += p.x*b1.data + p.y*b2.data + p.z*b3.data;
			}
		}
		std::cout << index++ << " stacked" << std::endl;
	}
	std::cout << "end" << std::endl;
	return s;
}
