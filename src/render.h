#ifndef RENDER_H
#define RENDER_H

#include "spheresurface.h"
#include <SFML/Graphics/VertexArray.hpp>

template <typename T>
sf::VertexArray drawBlocks(const SphereSurface<T> & m, float yaw, float pitch, const sf::Color & color, const sf::Color & pointColor)
{
    sf::VertexArray array(sf::Points);

    for(auto it(m.pointsBegin()) ; it != m.pointsEnd() ; it++)
    {
        SpherePoint point(it->pos);
        array.append(sf::Vertex(project(toVector3(point, m.radius()), yaw, pitch), pointColor));
    }

    for(auto it(m.blocksBegin()) ; it != m.blocksEnd() ; it++)
    {
        SpherePoint point(it->pos);
        array.append(sf::Vertex(project(toVector3(point,m.radius()), yaw, pitch), color));
    }

    return array;
}

template <typename T>
sf::VertexArray drawTrianglesAndSides(const SphereSurface<T> & m, float yaw, float pitch, const sf::Color & triangleColor, const sf::Color & lineColor)
{
    sf::VertexArray array(sf::Lines);
    for(auto it(m.trianglesBegin()) ; it != m.trianglesEnd() ; it++)
    {
        sf::Vector2f a(project(toVector3(std::next(m.blocksBegin(), it->block1)->pos, m.radius()), yaw, pitch));
        sf::Vector2f b(project(toVector3(std::next(m.blocksBegin(), it->block2)->pos, m.radius()), yaw, pitch));
        sf::Vector2f c(project(toVector3(std::next(m.blocksBegin(), it->block3)->pos, m.radius()), yaw, pitch));
        array.append(sf::Vertex(a, triangleColor));
        array.append(sf::Vertex(b, triangleColor));
        array.append(sf::Vertex(b, triangleColor));
        array.append(sf::Vertex(c, triangleColor));
        array.append(sf::Vertex(c, triangleColor));
        array.append(sf::Vertex(a, triangleColor));
    }
    for(auto it(m.bordersBegin()) ; it != m.bordersEnd() ; it++)
    {
        sf::Vector2f a(project(toVector3(std::next(m.pointsBegin(), it->point1)->pos, m.radius()), yaw, pitch));
        sf::Vector2f b(project(toVector3(std::next(m.pointsBegin(), it->point2)->pos, m.radius()), yaw, pitch));
        array.append(sf::Vertex(a, lineColor));
        array.append(sf::Vertex(b, lineColor));
    }

    return array;
}

sf::VertexArray drawBlocks(const SphereSurface<float> & m, float yaw, float pitch, const sf::Color & color, const sf::Color & pointColor);
sf::VertexArray drawTrianglesAndSides(const SphereSurface<float> & m, float yaw, float pitch, const sf::Color & triangleColor, const sf::Color & lineColor);

#endif // RENDER_H
