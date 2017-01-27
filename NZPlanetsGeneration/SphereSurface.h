#ifndef SPHERESURFACE_H
#define SPHERESURFACE_H
#pragma once

#include "Block.h"
#include <vector>
#include <cassert>

template <typename T>
class SphereSurface
{
public:
	SphereSurface(float radius = 1);
	SphereSurface(SphereSurface &&) = default;
	SphereSurface & operator=(SphereSurface &&) = default;

	SphereSurface(const SphereSurface & other) = default;
	SphereSurface & operator=(const SphereSurface & other) = default;
	~SphereSurface() = default;

	float radius() const { return m_radius; }
	void setRadius(float radius) { m_radius = radius; }

	void addBlock(const SpherePoint & pos);

	void buildMap();
	void buildMap2();
	bool builded() const { return m_builded; }

	typename std::vector<Block<T>>::const_iterator blocksBegin() const { return m_blocks.begin(); }
	typename std::vector<Block<T>>::iterator blocksBegin() { return m_blocks.begin(); }
	typename std::vector<Block<T>>::const_iterator blocksEnd() const { return m_blocks.end(); }
	typename std::vector<Block<T>>::iterator blocksEnd() { return m_blocks.end(); }

	std::vector<Border>::const_iterator bordersBegin() const { return m_borders.begin(); }
	std::vector<Border>::iterator bordersBegin() { return m_borders.begin(); }
	std::vector<Border>::const_iterator bordersEnd() const { if (!builded()) return m_borders.begin(); return m_borders.end(); }
	std::vector<Border>::iterator bordersEnd() { if (!builded()) return m_borders.begin(); return m_borders.end(); }

	std::vector<Point>::const_iterator pointsBegin() const { return m_points.begin(); }
	std::vector<Point>::iterator pointsBegin() { return m_points.begin(); }
	std::vector<Point>::const_iterator pointsEnd() const { if (!builded()) return m_points.begin(); return m_points.end(); }
	std::vector<Point>::iterator pointsEnd() { if (!builded()) return m_points.begin(); return m_points.end(); }

	std::vector<Triangle>::const_iterator trianglesBegin() const { return m_triangles.begin(); }
	std::vector<Triangle>::iterator trianglesBegin() { return m_triangles.begin(); }
	std::vector<Triangle>::const_iterator trianglesEnd() const { if (!builded()) return m_triangles.end(); return m_triangles.end(); }
	std::vector<Triangle>::iterator trianglesEnd() { if (!builded()) return m_triangles.end(); return m_triangles.end(); }

	template <typename U>
	SphereSurface<U> clone(U defaultValue = U());

private:

	float m_radius;

	std::vector<Block<T>> m_blocks;
	std::vector<Border> m_borders;
	std::vector<Point> m_points;
	std::vector<Triangle> m_triangles;

	bool m_builded;
};

template <typename T>
SphereSurface<T> relaxation(const SphereSurface<T> & m, float diviser = 1);

#include "SphereSurface.inl"

#endif // SPHERESURFACE_H
