#ifndef VECT2CONVERT_H
#define VECT2CONVERT_H

#include "spherepoint.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>

inline float angle(const sf::Vector2f & vect)
{
    return atan2(vect.y,vect.x);
}

inline float norm(const sf::Vector2f & vect)
{
    return sqrt(vect.x*vect.x +vect.y*vect.y);
}

inline sf::Vector2f normalise(const sf::Vector2f & vect)
{
    return vect/norm(vect);
}

inline float norm(const sf::Vector3f & vect)
{
    return sqrt(vect.x*vect.x +vect.y*vect.y + vect.z*vect.z);
}

inline float sqrNorm(const sf::Vector3f & vect)
{
    return vect.x*vect.x + vect.y*vect.y + vect.z*vect.z;
}

inline sf::Vector3f normalise(const sf::Vector3f & vect)
{
    return vect/norm(vect);
}

inline sf::Vector2f toVect(float norm, float angle)
{
    return sf::Vector2f(cos(angle)*norm, sin(angle)*norm);
}

inline sf::Vector2f rotate(const sf::Vector2f & vect, float rotateAngle)
{
    return toVect(norm(vect), angle(vect) + rotateAngle);
}

inline sf::Vector3f toVector3(const SpherePoint & point, float radius = 1)
{
    sf::Vector3f pos(std::cos(point.yaw)*radius, std::sin(point.yaw)*radius, 0);
    pos.x *= sin(point.pitch);
    pos.y *= sin(point.pitch);
    pos.z = cos(point.pitch)*radius;
    return pos;
}

inline SpherePoint toSpherePoint(const sf::Vector3f & pos)
{
    return SpherePoint(angle(sf::Vector2f(pos.x, pos.y)), angle(sf::Vector2f(pos.z, norm(sf::Vector2f(pos.x, pos.y)))));
}

inline sf::Vector2f project(const sf::Vector3f & pos, float yaw, float pitch)
{
    SpherePoint s(toSpherePoint(pos));
    s.yaw += yaw;
    sf::Vector3f rotatedPos(toVector3(s, norm(pos)));
    return sf::Vector2f(rotatedPos.x, rotatedPos.y*cos(pitch)+rotatedPos.z*sin(pitch));

    /*sf::Vector2f projection;
    projection.x = pos.x*cos(yaw)+pos.y*sin(yaw);
    projection.y = pos.y*cos(pitch)+pos.z*sin(pitch);
    return projection;*/
}

#endif // VECT2CONVERT_H
