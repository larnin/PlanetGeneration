//#include "spheresurface.h"

#include "Utilities.h"
#include <Nazara/Math/Vector3.hpp>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <limits>
#include <array>

template <typename T>
SphereSurface<T>::SphereSurface(float radius)
	: m_radius(radius)
	, m_builded(false)
{

}

template <typename T>
void SphereSurface<T>::addBlock(const SpherePoint & pos, T value = T())
{
	m_blocks.emplace_back(pos);
	m_blocks.back().data = value;
	m_builded = false;
}

template <typename T>
std::vector<unsigned int> SphereSurface<T>::connectedBlocks(unsigned int id)
{
	assert(id < m_blocks.size());

	std::vector<unsigned int> list;

	for (unsigned int i : m_blocks[id].triangles)
	{
		const auto & t(m_triangles[i]);
		for(unsigned int index : {t.block1, t.block2, t.block3})
			if (std::find(list.begin(), list.end(), index) == list.end())
				list.push_back(index);
	}

	return list;
}

template <typename T>
void SphereSurface<T>::buildMap()
{
	if (m_builded)
		return;

	for (auto & b : m_blocks)
		b.triangles.clear();

	std::vector<Nz::Vector3f> points;
	for (unsigned int i(0); i < m_blocks.size(); i++)
		points.push_back(toVector3(m_blocks[i].pos));

	unsigned int a, b, c, d;
	bool isOn = false;
	for (a = 0; a < points.size(); a++)
	{
		for (b = a + 1; b < points.size(); b++)
		{
			for (c = b + 1; c < points.size(); c++)
			{
				for (d = c + 1; d < points.size(); d++)
				{
					isOn = pointOnTetrahedron(points[a], points[b], points[c], points[d], Nz::Vector3f::Zero());
					if (isOn)
					{
						isOn = triangleOmega(points[a], points[b], points[c]).SquaredDistance(points[a]) <= points[a].SquaredDistance(points[d])
							&& triangleOmega(points[a], points[b], points[d]).SquaredDistance(points[a]) <= points[a].SquaredDistance(points[c])
							&& triangleOmega(points[a], points[c], points[d]).SquaredDistance(points[a]) <= points[a].SquaredDistance(points[b])
							&& triangleOmega(points[b], points[c], points[d]).SquaredDistance(points[b]) <= points[b].SquaredDistance(points[a]);
						if (isOn)
							break;
					}
				}
				if (isOn)
					break;
			}
			if (isOn)
				break;
		}
		if (isOn)
			break;
	}

	if (!isOn)
	{
		m_builded = true;
		return;
	}

	struct LocalTriangle
	{
		LocalTriangle(unsigned int _a, unsigned int _b, unsigned int _c)
			: a(_a), b(_b), c(_c) {}
		unsigned int a;
		unsigned int b;
		unsigned int c;
	};

	std::vector<LocalTriangle> triangles;
	triangles.emplace_back(a, b, c);
	triangles.emplace_back(a, b, d);
	triangles.emplace_back(a, c, d);
	triangles.emplace_back(b, c, d);

	std::vector<unsigned int> pointsToCompute;
	for (unsigned int i(0); i < points.size(); i++)
		if (i != a && i != b && i != c && i != d)
			pointsToCompute.push_back(i);

	while (!pointsToCompute.empty())
	{
		unsigned int index(pointsToCompute.back());
		pointsToCompute.pop_back();
		for (unsigned int tIndex(0); tIndex < triangles.size(); tIndex++)
		{
			LocalTriangle t(triangles[tIndex]);
			if (!intersect(points[t.a], points[t.b], points[t.c], Nz::Vector3f::Zero(), points[index]).first)
				continue;

			triangles.emplace_back(index, t.a, t.b);
			triangles.emplace_back(index, t.a, t.c);
			triangles.emplace_back(index, t.b, t.c);
			triangles.erase(std::next(triangles.begin(), tIndex));

			std::vector<LocalTriangle*> newTriangles{ &triangles[triangles.size() - 1], &triangles[triangles.size() - 2] , &triangles[triangles.size() - 3] };
			for (LocalTriangle* newTriangle : newTriangles)
			{
				auto other(std::find_if(triangles.begin(), triangles.end(), [newTriangle](const auto & t) 
				{
					std::array<unsigned int, 3> tPoints{ t.a, t.b, t.c };
					return std::find(tPoints.begin(), tPoints.end(), newTriangle->b) != tPoints.end() && std::find(tPoints.begin(), tPoints.end(), newTriangle->c) != tPoints.end();
				}));
				assert(other != triangles.end());

				Nz::Vector3f omega(triangleOmega(points[other->a], points[other->b], points[other->c]));
				if ((omega - points[index]).GetSquaredLength() > (omega - points[other->a]).GetSquaredLength())
					continue;

				if (other->a == newTriangle->c)
					other->a = newTriangle->a;
				else if (other->b == newTriangle->c)
					other->b = newTriangle->a;
				else other->c = newTriangle->a;
				if (other->a != newTriangle->a && other->a != newTriangle->b)
					newTriangle->b = other->a;
				else if(other->b != newTriangle->a && other->b != newTriangle->b)
					newTriangle->b = other->b;
				else newTriangle->b = other->c;

			}

			break;
		}
	}

	for (const auto & t : triangles)
		addTriangle(t.a, t.b, t.c);

	for (unsigned int i(0); i < triangles.size(); i++)
	{
		const LocalTriangle &t(triangles[i]);
		addTriangle(t.a, t.b, t.c);
		m_blocks[t.a].triangles.push_back(i);
		m_blocks[t.b].triangles.push_back(i);
		m_blocks[t.c].triangles.push_back(i);
	}

	m_builded = true;
}

template <typename T>
void SphereSurface<T>::addTriangle(unsigned int a, unsigned int b, unsigned int c)
{
	if (!isNormalOut(toVector3(m_blocks[a].pos), toVector3(m_blocks[b].pos), toVector3(m_blocks[c].pos)))
		std::swap(b, c);
	m_triangles.emplace_back(a, b, c);
}

template <typename T>
template <typename U>
SphereSurface<U> SphereSurface<T>::clone(U defaultValue)
{
	SphereSurface<U> surface;
	surface.m_builded = m_builded;

	for (SphereBlock<T> bT : m_blocks)
	{
		surface.addBlock(bT.pos);
		SphereBlock<U>& bU(surface.m_blocks.back());
		bU.data = defaultValue;
		if (m_builded)
		{
			bU.points = bT.points;
			bU.borders = bT.borders;
		}
	}

	if (m_builded)
	{
		surface.m_borders = m_borders;
		surface.m_points = m_points;
		surface.m_triangles = m_triangles;
	}

	return surface;
}

template<typename T>
SphereSurface<T> relax(const SphereSurface<T> & s)
{
	if (!s.builded())
		return s;

	SphereSurface<T> s2(s.radius());

	for (auto it(s.blocksBegin()); it != s.blocksEnd(); it++)
	{
		std::vector<unsigned int> connectedPoints;
		unsigned int index(std::distance(s.blocksBegin(), it));

		for (auto it2(s.trianglesBegin()); it2 != s.trianglesEnd(); it2++)
		{
			if (it2->block1 == index || it2->block2 == index || it2->block3 == index)
			{
				if (std::find(connectedPoints.begin(), connectedPoints.end(), it2->block1) == connectedPoints.end())
					connectedPoints.push_back(it2->block1);
				if (std::find(connectedPoints.begin(), connectedPoints.end(), it2->block2) == connectedPoints.end())
					connectedPoints.push_back(it2->block2);
				if (std::find(connectedPoints.begin(), connectedPoints.end(), it2->block3) == connectedPoints.end())
					connectedPoints.push_back(it2->block3);
			}
		}

		Nz::Vector3f pos(Nz::Vector3f::Zero());
		for (unsigned int point : connectedPoints)
			pos += toVector3(std::next(s.blocksBegin(), point)->pos);
		pos /= connectedPoints.size();

		s2.addBlock(toSpherePoint(pos), it->data);
	}

	s2.buildMap();

	return s2;
}

template<typename T>
void makeTriangles(std::vector<SphereBlock<T>>& blocks, std::vector<SphereTriangle>& triangles, unsigned int triangle, unsigned int block1, unsigned int block2, unsigned int block3)
{

}

template<typename T>
void makeRegular(SphereSurface<T> & surface, unsigned int steps, T value)
{
	auto & blocks(surface.m_blocks);
	auto & triangles(surface.m_triangles);

	triangles.clear();
	blocks.clear();

	blocks.emplace_back(SpherePoint(0, float(M_PI)), value);
	blocks.emplace_back(SpherePoint(0, float(M_PI) / 3), value);
	blocks.emplace_back(SpherePoint(2 * float(M_PI) / 3, float(M_PI) / 3), value);
	blocks.emplace_back(SpherePoint(4 * float(M_PI) / 3, float(M_PI) / 3), value);

	surface.addTriangle(0, 1, 2);
	surface.addTriangle(0, 1, 3);
	surface.addTriangle(0, 2, 3);
	surface.addTriangle(1, 2, 3);

	blocks[0].triangles = { 0, 1, 2 };
	blocks[1].triangles = { 0, 1, 3 };
	blocks[2].triangles = { 0, 2, 3 };
	blocks[3].triangles = { 1, 2, 3 };

	struct NextTriangle
	{
		NextTriangle(unsigned int _triangle, SphereLine _line, unsigned int _point)
			: triangle(_triangle), line(_line), point(_point) {}

		unsigned int triangle;
		SphereLine line;
		unsigned int point;
	};

	for (unsigned int i(0); i < steps; i++)
	{
		std::vector<NextTriangle> triangles;

		auto triangle t(triangles[0]);

		auto b1(toVector3(surface.block(t.block1).pos);
		auto b2(toVector3(surface.block(t.block2).pos);
		auto b3(toVector3(surface.block(t.block3).pos);

		blocks.emplace_back(toSpherePoint(b1 + b2), value);
		blocks.emplace_back(toSpherePoint(b2 + b3), value);
		blocks.emplace_back(toSpherePoint(b1 + b3), value);

		makeTriangles(blocks, triangles, 0, blocks.size() - 3, blocks.size() - 2, blocks.size() - 1);


	}

	surface.m_builded = true;
}