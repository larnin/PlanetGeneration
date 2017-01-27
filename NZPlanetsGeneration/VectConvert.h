#ifndef VECTCONVERT_H
#define VECTCONVERT_H
#pragma once

#include "SpherePoint.h"
#include <Nazara/Math/Vector2.hpp>
#include <Nazara/Math/Vector3.hpp>
#include <cmath>

inline float angle(const Nz::Vector2f & vect)
{
	return atan2(vect.y, vect.x);
}

inline float norm(const Nz::Vector2f & vect)
{
	return sqrt(vect.x*vect.x + vect.y*vect.y);
}

inline Nz::Vector2f normalise(const Nz::Vector2f & vect)
{
	return vect / norm(vect);
}

inline float norm(const Nz::Vector3f & vect)
{
	return sqrt(vect.x*vect.x + vect.y*vect.y + vect.z*vect.z);
}

inline float sqrNorm(const Nz::Vector3f & vect)
{
	return vect.x*vect.x + vect.y*vect.y + vect.z*vect.z;
}

inline Nz::Vector3f normalise(const Nz::Vector3f & vect)
{
	return vect / norm(vect);
}

inline Nz::Vector2f toVect(float norm, float angle)
{
	return Nz::Vector2f(cos(angle)*norm, sin(angle)*norm);
}

inline Nz::Vector2f rotate(const Nz::Vector2f & vect, float rotateAngle)
{
	return toVect(norm(vect), angle(vect) + rotateAngle);
}

inline Nz::Vector3f toVector3(const SpherePoint & point, float radius = 1)
{
	Nz::Vector3f pos(std::cos(point.yaw)*radius, std::sin(point.yaw)*radius, 0);
	pos.x *= sin(point.pitch);
	pos.y *= sin(point.pitch);
	pos.z = cos(point.pitch)*radius;
	return pos;
}

inline SpherePoint toSpherePoint(const Nz::Vector3f & pos)
{
	return SpherePoint(angle(Nz::Vector2f(pos.x, pos.y)), angle(Nz::Vector2f(pos.z, norm(Nz::Vector2f(pos.x, pos.y)))));
}

inline Nz::Vector2f project(const Nz::Vector3f & pos, float yaw, float pitch)
{
	SpherePoint s(toSpherePoint(pos));
	s.yaw += yaw;
	Nz::Vector3f rotatedPos(toVector3(s, norm(pos)));
	return Nz::Vector2f(rotatedPos.x, rotatedPos.y*cos(pitch) + rotatedPos.z*sin(pitch));
}

#endif // VECTCONVERT_H
