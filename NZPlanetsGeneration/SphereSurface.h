#ifndef SPHERESURFACE_H
#define SPHERESURFACE_H
#pragma once

#include "Block.h"
#include <Nazara/Math/Vector3.hpp>
#include <vector>
#include <cassert>
#include <map>

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

	void addBlock(const SpherePoint & pos, T value = T());

	void buildMap();
	//void buildMap2();
	bool builded() const { return m_builded; }

	unsigned int blockCount() const { return m_blocks.size(); }
	unsigned int triangleCount() const { return m_triangles.size(); }

	typename std::vector<SphereBlock<T>>::const_iterator blocksBegin() const { return m_blocks.begin(); }
	typename std::vector<SphereBlock<T>>::iterator blocksBegin() { return m_blocks.begin(); }
	typename std::vector<SphereBlock<T>>::const_iterator blocksEnd() const { return m_blocks.end(); }
	typename std::vector<SphereBlock<T>>::iterator blocksEnd() { return m_blocks.end(); }
	SphereBlock<T> & block(unsigned int index) { return m_blocks[index]; }
	const SphereBlock<T> & block(unsigned int index) const { return m_blocks[index]; }
	std::vector<unsigned int> connectedBlocks(unsigned int id) const;

	std::vector<SphereTriangle>::const_iterator trianglesBegin() const { return m_triangles.begin(); }
	std::vector<SphereTriangle>::iterator trianglesBegin() { return m_triangles.begin(); }
	std::vector<SphereTriangle>::const_iterator trianglesEnd() const { if (!builded()) return m_triangles.end(); return m_triangles.end(); }
	std::vector<SphereTriangle>::iterator trianglesEnd() { if (!builded()) return m_triangles.end(); return m_triangles.end(); }
	SphereTriangle & triangle(unsigned int index) { return m_triangles[index]; }
	const SphereTriangle & triangle(unsigned int index) const { return m_triangles[index]; }

	template <typename U>
	SphereSurface<U> clone(U defaultValue = U());

protected:
	void addTriangle(unsigned int a, unsigned int b, unsigned int c);

	float m_radius;

	std::vector<SphereBlock<T>> m_blocks;
	std::vector<SphereTriangle> m_triangles;

	bool m_builded;

	template<typename U>
	friend void makeRegular(SphereSurface<U> & surface, unsigned int steps, U value);
};

template<typename T>
SphereSurface<T> relax(const SphereSurface<T> & s);

template<typename T>
void makeRegular(SphereSurface<T> & surface, unsigned int steps, T value = T());

using Lookup = std::map<std::pair<unsigned int, unsigned int>, unsigned int>;
unsigned int vertexForEdge(Lookup& lookup, std::vector<Nz::Vector3f>& vertices, unsigned int first, unsigned int second);
std::vector<SphereTriangle> subdivide(std::vector<Nz::Vector3f>& vertices, const std::vector<SphereTriangle>& triangles);

#include "SphereSurface.inl"

#endif // SPHERESURFACE_H
