#ifndef SPHERESURFACE_H
#define SPHERESURFACE_H

#include "block.h"
#include <vector>
#include <SFML/System/Vector3.hpp>
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
    bool builded() const { return m_builded; }

    typename std::vector<Block<T>>::const_iterator blocksBegin() const { return m_blocks.begin(); }
    typename std::vector<Block<T>>::iterator blocksBegin() { return m_blocks.begin(); }
    typename std::vector<Block<T>>::const_iterator blocksEnd() const { return m_blocks.end(); }
    typename std::vector<Block<T>>::iterator blocksEnd() { return m_blocks.end(); }

    std::vector<Border>::const_iterator bordersBegin() const { return m_borders.begin(); }
    std::vector<Border>::iterator bordersBegin() { return m_borders.begin(); }
    std::vector<Border>::const_iterator bordersEnd() const { return m_borders.end(); }
    std::vector<Border>::iterator bordersEnd() { return m_borders.end(); }

    std::vector<Point>::const_iterator pointsBegin() const { return m_points.begin(); }
    std::vector<Point>::iterator pointsBegin() { return m_points.begin(); }
    std::vector<Point>::const_iterator pointsEnd() const { return m_points.end(); }
    std::vector<Point>::iterator pointsEnd(){ return m_points.end(); }

    std::vector<Triangle>::const_iterator trianglesBegin() const { return m_triangles.begin(); }
    std::vector<Triangle>::iterator trianglesBegin() { return m_triangles.begin(); }
    std::vector<Triangle>::const_iterator trianglesEnd() const { return m_triangles.end(); }
    std::vector<Triangle>::iterator trianglesEnd() { return m_triangles.end(); }

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

#include <spheresurface.tpp>

#endif // SPHERESURFACE_H
