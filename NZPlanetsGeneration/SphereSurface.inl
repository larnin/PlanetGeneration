//#include "spheresurface.h"

#include "VectConvert.h"
#include "Utilities.h"
#include <Nazara/Math/Vector3.hpp>
#include <cmath>
#include <algorithm>
#include <limits>

template <typename T>
SphereSurface<T>::SphereSurface(float radius)
	: m_radius(radius)
	, m_builded(false)
{

}

template <typename T>
void SphereSurface<T>::addBlock(const SpherePoint & pos)
{
	m_blocks.emplace_back(pos);
	m_builded = false;
}

template <typename T>
void SphereSurface<T>::buildMap()
{
	if (m_builded)
		return;

	m_borders.clear();
	m_points.clear();
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

				Triangle t(b1, b2, b3);
				m_triangles.push_back(t);
				m_points.emplace_back(toSpherePoint(center));
				Point p = m_points.back();
				unsigned int pID = m_points.size() - 1;
				it->points.push_back(pID);
				it2->points.push_back(pID);
				it3->points.push_back(pID);
				p.blocks.push_back(b1);
				p.blocks.push_back(b2);
				p.blocks.push_back(b3);
			}
		}
	}

	m_builded = true;
}

template <typename T>
void SphereSurface<T>::buildMap2()
{
	unsigned int b1(0);
	unsigned int b2(0);
	unsigned int b3(0);
	bool firstTriangleFound(false);

	Nz::Vector3f pos1(toVector3(m_blocks.front().pos));
	for (auto it(std::next(m_blocks.begin())); it != m_blocks.end(); it++)
	{
		Nz::Vector3f pos2(toVector3(it2->pos));
		for (auto it2(std::next(m_blocks.begin(); it2 != m_blocks.end; it++)))
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

			b2 = std::distance(m_blocks.begin(), it);
			b3 = std::distance(m_blocks.begin(), it2);

			firstTriangleFound = true;
		}
		if (firstTriangleFound)
			break;
	}

	auto & blocks(m_blocks);
	std::vector<unsigned int> uncomputedPoints;
	for (unsigned int i(0); i < m_blocks.size(); i++)
		if (i != b1 || i != b2 || i != b3)
			uncomputedPoints.push_back(i);

	std::vector<unsigned int> convexHull;
	convexHull.push_back(b1);
	convexHull.push_back(b2);
	convexHull.push_back(b3);

	while (uncomputedPoints.size() > 0)
	{
		//get the point at the min distance of the hull
		//connect that point at the two nearest point of the hull (the two points that support the nearest line)
		// add the triangle to the hull and made it convex
		// if the convex hull is modified, add the modified triangle
		//repeat
	}
}

template <typename T>
template <typename U>
SphereSurface<U> SphereSurface<T>::clone(U defaultValue)
{
	SphereSurface<U> surface;
	surface.m_builded = m_builded;

	for (Block<T> bT : m_blocks)
	{
		surface.addBlock(bT.pos);
		Block<U>& bU(surface.m_blocks.back());
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

