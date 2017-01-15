#ifndef UTILITIES_H
#define UTILITIES_H

#include <SFML/System/Vector3.hpp>
#include <utility>
#include "spherepoint.h"

sf::Vector3f cross(const sf::Vector3f & a , const sf::Vector3f & b);
float dot(const sf::Vector3f & a, const sf::Vector3f & b);

sf::Vector3f triangleOmega(const sf::Vector3f & a, const sf::Vector3f & b, const sf::Vector3f & c);

std::pair<bool, sf::Vector3f> intersect(const sf::Vector3f & v0, const sf::Vector3f & v1, const sf::Vector3f & v2, const sf::Vector3f & p, const sf::Vector3f & d);

sf::Vector3f proportions(const sf::Vector3f & a, const sf::Vector3f & b, const sf::Vector3f & c, const sf::Vector3f & pos);

float pointLineDistance(const sf::Vector3f & line1, const sf::Vector3f & line2, const sf::Vector3f & pos);


#endif // UTILITIES_H
