#include "render.h"

sf::VertexArray drawBlocks(const SphereSurface<float> & m, float yaw, float pitch, const sf::Color & color, const sf::Color & pointColor)
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
        array.append(sf::Vertex(project(toVector3(point)*(m.radius()+it->data), yaw, pitch), color));
    }

    return array;
}

sf::VertexArray drawTrianglesAndSides(const SphereSurface<float> & m, float yaw, float pitch, const sf::Color & triangleColor, const sf::Color & lineColor)
{
    sf::VertexArray array(sf::Lines);
    for(auto it(m.trianglesBegin()) ; it != m.trianglesEnd() ; it++)
    {
        const Block<float> & b1(*std::next(m.blocksBegin(), it->block1));
        const Block<float> & b2(*std::next(m.blocksBegin(), it->block2));
        const Block<float> & b3(*std::next(m.blocksBegin(), it->block3));

        sf::Vector2f a(project(toVector3(b1.pos)*(m.radius()+b1.data), yaw, pitch));
        sf::Vector2f b(project(toVector3(b2.pos)*(m.radius()+b2.data), yaw, pitch));
        sf::Vector2f c(project(toVector3(b3.pos)*(m.radius()+b3.data), yaw, pitch));
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

