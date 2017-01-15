#include "utilities.h"
#include "vectconvert.h"
#include <iostream>

sf::Vector3f cross(const sf::Vector3f & a , const sf::Vector3f & b)
{
    return sf::Vector3f(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

float dot(const sf::Vector3f & a, const sf::Vector3f & b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

sf::Vector3f triangleOmega(const sf::Vector3f & a, const sf::Vector3f & b, const sf::Vector3f & c)
{
    sf::Vector3f ac(c-a);
    sf::Vector3f ab(b-a);
    sf::Vector3f abXac(cross(ab, ac));

    sf::Vector3f toCircumsphereCenter((cross(abXac,ab)*sqrNorm(ac) + cross(ac, abXac)*sqrNorm(ab))/(2*sqrNorm(abXac)));

    return a + toCircumsphereCenter;
}

std::pair<bool, sf::Vector3f> intersect(const sf::Vector3f & v0, const sf::Vector3f & v1, const sf::Vector3f & v2, const sf::Vector3f & p, const sf::Vector3f & d)
{
    sf::Vector3f e1(v1-v0);
    sf::Vector3f e2(v2-v0);
    sf::Vector3f h(cross(d, e2));
    float a(dot(e1, h));

    if(a == 0)
        return std::make_pair(false, sf::Vector3f());

    float f(1/a);
    sf::Vector3f s(p-v0);
    float u(f*dot(s, h));
    if(u < 0 || u > 1)
        return std::make_pair(false, sf::Vector3f());

    sf::Vector3f q(cross(s, e1));
    float v(f*dot(d, q));
    if(v < 0 || v > 1)
        return std::make_pair(false, sf::Vector3f());

    float t(f*dot(e2, q));
    if(t < 0)
        return std::make_pair(false, sf::Vector3f());

    return std::make_pair(true, p+d*t);
}

sf::Vector3f proportions(const sf::Vector3f & a, const sf::Vector3f & b, const sf::Vector3f & c, const sf::Vector3f & pos)
{
    sf::Vector3f distancesLine(pointLineDistance(b, c, pos), pointLineDistance(a, c, pos), pointLineDistance(a, b, pos));
    return sf::Vector3f(distancesLine.x/(distancesLine.x+norm(pos-a)), distancesLine.y/(distancesLine.y+norm(pos-b)), distancesLine.z/(distancesLine.z+norm(pos-c)));
}

float pointLineDistance(const sf::Vector3f & line1, const sf::Vector3f & line2, const sf::Vector3f & pos)
{
    return norm(cross(pos-line1, pos-line2))/norm(line2-line1);
}
