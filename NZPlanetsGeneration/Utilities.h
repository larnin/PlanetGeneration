#ifndef UTILITIES_H
#define UTILITIES_H
#pragma once

#include <Nazara/Math/Vector3.hpp>
#include <utility>
#include "spherepoint.h"

Nz::Vector3f cross(const Nz::Vector3f & a, const Nz::Vector3f & b);
float dot(const Nz::Vector3f & a, const Nz::Vector3f & b);

Nz::Vector3f triangleOmega(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c);

std::pair<bool, Nz::Vector3f> intersect(const Nz::Vector3f & v0, const Nz::Vector3f & v1, const Nz::Vector3f & v2, const Nz::Vector3f & p, const Nz::Vector3f & d);

Nz::Vector3f proportions(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c, const Nz::Vector3f & pos);

float pointLineDistance(const Nz::Vector3f & line1, const Nz::Vector3f & line2, const Nz::Vector3f & pos);

bool isNormalOut(const Nz::Vector3f & pos1, const Nz::Vector3f & pos2, const Nz::Vector3f & pos3);


#endif // UTILITIES_H
