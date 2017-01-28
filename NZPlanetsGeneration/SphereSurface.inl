//#include "spheresurface.h"

#include "VectConvert.h"
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

/*template <typename T>
void SphereSurface<T>::buildMap()
{
	if (m_builded)
		return;

	m_triangles.clear();

	for (auto it(m_blocks.begin()); it != m_blocks.end(); it++)
	{
		Nz::Vector3f pos1(toVector3(it->pos));
		for (auto it2(std::next(it)); it2 != m_blocks.end(); it2++)
		{
			Nz::Vector3f pos2(toVector3(it2->pos));
			for (auto it3(std::next(it2)); it3 != m_blocks.end(); it3++)
			{
				Nz::Vector3f pos3(toVector3(it3->pos));
				Nz::Vector3f center(triangleOmega(pos1, pos2, pos3));
				float sqrRadius(sqrNorm(center - pos1));
				bool isOn = false;
				for (auto it4(m_blocks.begin()); it4 != m_blocks.end(); it4++)
				{
					if (it4 == it || it4 == it2 || it4 == it3)
						continue;
					if (sqrNorm(toVector3(it4->pos) - center) <= sqrRadius)
					{
						isOn = true;
						break;
					}
				}
				if (isOn)
					continue;

				unsigned int b1(std::distance(m_blocks.begin(), it));
				unsigned int b2(std::distance(m_blocks.begin(), it2));
				unsigned int b3(std::distance(m_blocks.begin(), it3));

				if (!isNormalOut(pos1, pos2, pos3))
					std::swap(b2, b3);

				SphereTriangle t(b1, b2, b3);
				m_triangles.push_back(t);
			}
		}
	}

	m_builded = true;
}*/

template <typename T>
void SphereSurface<T>::buildMap()
{
	if (m_builded)
		return;

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
						break;
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

template <typename T>
SphereSurface<T> relaxation(const SphereSurface<T> & m, float diviser)
{
	SphereSurface<T> s(m.radius());
	for (auto it(m.blocksBegin()); it != m.blocksEnd(); it++)
	{
		Nz::Vector3f pos(toVector3(it->pos, s.radius()));
		for (auto it2(m.blocksBegin()); it2 != m.blocksEnd(); it2++)
		{
			if (it == it2)
				continue;
			Nz::Vector3f pos2(toVector3(it2->pos, s.radius()));
			pos -= pos2 / (norm(pos2 - pos)*diviser);
		}
		s.addBlock(toSpherePoint(pos));
	}

	return s;
}

