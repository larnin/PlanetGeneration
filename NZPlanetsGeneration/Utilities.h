#ifndef UTILITIES_H
#define UTILITIES_H
#pragma once

#include <Nazara/Math/Vector3.hpp>
#include <Nazara/Math/Vector2.hpp>
#include <utility>
#include <Nazara/Core/Color.hpp>
#include "spherepoint.h"

float angle(const Nz::Vector2f & vect);

Nz::Vector2f toVect(float norm, float angle);

Nz::Vector2f rotate(const Nz::Vector2f & vect, float rotateAngle);

Nz::Vector3f toVector3(const SpherePoint & point, float radius = 1);

SpherePoint toSpherePoint(const Nz::Vector3f & pos);

Nz::Vector2f project(const Nz::Vector3f & pos, float yaw, float pitch);

/*Nz::Vector3f cross(const Nz::Vector3f & a, const Nz::Vector3f & b);
float dot(const Nz::Vector3f & a, const Nz::Vector3f & b);*/

Nz::Vector3f triangleOmega(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c);

std::pair<bool, Nz::Vector3f> intersect(const Nz::Vector3f & v0, const Nz::Vector3f & v1, const Nz::Vector3f & v2, const Nz::Vector3f & p, const Nz::Vector3f & d);

Nz::Vector3f proportions(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c, const Nz::Vector3f & pos);

float pointLineDistance(const Nz::Vector3f & line1, const Nz::Vector3f & line2, const Nz::Vector3f & pos);

bool isNormalOut(const Nz::Vector3f & pos1, const Nz::Vector3f & pos2, const Nz::Vector3f & pos3);

bool isLeft(const Nz::Vector3f & dir, const Nz::Vector3f line, const Nz::Vector3f & normal);

bool pointOnTetrahedron(Nz::Vector3f a, Nz::Vector3f b, Nz::Vector3f c, Nz::Vector3f d, Nz::Vector3f point);

Nz::Vector2f colorToUV(const Nz::Color & c);

#endif // UTILITIES_H
