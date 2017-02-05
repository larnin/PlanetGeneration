#include "Utilities.h"

float angle(const Nz::Vector2f & vect)
{
	return atan2(vect.y, vect.x);
}

Nz::Vector2f toVect(float norm, float angle)
{
	return Nz::Vector2f(cos(angle)*norm, sin(angle)*norm);
}

Nz::Vector2f rotate(const Nz::Vector2f & vect, float rotateAngle)
{
	return toVect(vect.GetLength(), angle(vect) + rotateAngle);
}

Nz::Vector3f toVector3(const SpherePoint & point, float radius)
{
	Nz::Vector3f pos(std::cos(point.yaw)*radius, std::sin(point.yaw)*radius, 0);
	pos.x *= sin(point.pitch);
	pos.y *= sin(point.pitch);
	pos.z = cos(point.pitch)*radius;
	return pos;
}

SpherePoint toSpherePoint(const Nz::Vector3f & pos)
{
	return SpherePoint(angle(Nz::Vector2f(pos.x, pos.y)), angle(Nz::Vector2f(pos.z, Nz::Vector2f(pos.x, pos.y).GetLength())));
}

Nz::Vector2f project(const Nz::Vector3f & pos, float yaw, float pitch)
{
	SpherePoint s(toSpherePoint(pos));
	s.yaw += yaw;
	Nz::Vector3f rotatedPos(toVector3(s, pos.GetLength()));
	return Nz::Vector2f(rotatedPos.x, rotatedPos.y*cos(pitch) + rotatedPos.z*sin(pitch));
}

bool isNormalOut(const Nz::Vector3f & pos1, const Nz::Vector3f & pos2, const Nz::Vector3f & pos3)
{
	Nz::Vector3f normal(Nz::Vector3f::CrossProduct(pos2 - pos1, pos3 - pos1));
	Nz::Vector3f dir(pos1 + pos2 + pos3);
	return Nz::Vector3f::DotProduct(dir, normal) > 0;
}

Nz::Vector3f triangleOmega(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c)
{
	Nz::Vector3f ac(c - a);
	Nz::Vector3f ab(b - a);
	Nz::Vector3f abXac(Nz::Vector3f::CrossProduct(ab, ac));
	float sqrabXac = 2 * abXac.GetSquaredLength();
	if (abs(sqrabXac) <= std::numeric_limits<float>::epsilon())
		sqrabXac = 2*std::numeric_limits<float>::epsilon();

	Nz::Vector3f toCircumsphereCenter((Nz::Vector3f::CrossProduct(abXac, ab)*ac.GetSquaredLength() + Nz::Vector3f::CrossProduct(ac, abXac)*ab.GetSquaredLength()) / sqrabXac);

	return a + toCircumsphereCenter;
}

std::pair<bool, Nz::Vector3f> intersect(const Nz::Vector3f & v0, const Nz::Vector3f & v1, const Nz::Vector3f & v2, const Nz::Vector3f & p, const Nz::Vector3f & d)
{
	Nz::Vector3f e1(v1 - v0);
	Nz::Vector3f e2(v2 - v0);
	Nz::Vector3f h(Nz::Vector3f::CrossProduct(d, e2));
	float a(Nz::Vector3f::DotProduct(e1, h));

	if (a == 0)
		return std::make_pair(false, Nz::Vector3f());

	float f(1 / a);
	Nz::Vector3f s(p - v0);
	float u(f*Nz::Vector3f::DotProduct(s, h));
	if (u < 0 || u > 1)
		return std::make_pair(false, Nz::Vector3f());

	Nz::Vector3f q(Nz::Vector3f::CrossProduct(s, e1));
	float v(f*Nz::Vector3f::DotProduct(d, q));
	if (v < 0 || u+v > 1)
		return std::make_pair(false, Nz::Vector3f());

	float t(f*Nz::Vector3f::DotProduct(e2, q));
	if (t < 0)
		return std::make_pair(false, Nz::Vector3f());

	return std::make_pair(true, p + d*t);
}

Nz::Vector3f proportions(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c, const Nz::Vector3f & pos)
{
	Nz::Vector3f distancesLine(pointLineDistance(b, c, pos), pointLineDistance(a, c, pos), pointLineDistance(a, b, pos));
	return Nz::Vector3f(distancesLine.x / (distancesLine.x + (pos - a).GetLength())
					  , distancesLine.y / (distancesLine.y + (pos - b).GetLength())
					  , distancesLine.z / (distancesLine.z + (pos - c).GetLength()));
}

float pointLineDistance(const Nz::Vector3f & line1, const Nz::Vector3f & line2, const Nz::Vector3f & pos)
{
	return (Nz::Vector3f::CrossProduct(pos - line1, pos - line2)).GetLength() / (line2 - line1).GetLength();
}

bool isLeft(const Nz::Vector3f & dir, const Nz::Vector3f line, const Nz::Vector3f & normal)
{
	return (Nz::Vector3f::DotProduct(Nz::Vector3f::CrossProduct(line, normal), dir) < 0);
}

bool pointOnTetrahedron(Nz::Vector3f a,Nz::Vector3f b, Nz::Vector3f c, Nz::Vector3f d, Nz::Vector3f point)
{
	Nz::Vector3f center((a + b + c + d) / 4);
	a -= center;
	b -= center;
	c -= center;
	d -= center;
	point -= center;

	auto lambda([](const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c, Nz::Vector3f point) -> bool
	{
		Nz::Vector3f center((a + b + c) / 3);
		point -= center;
		Nz::Vector3f normal(Nz::Vector3f::CrossProduct(b - a, c - a));
		bool pointSameSideThanNormal(Nz::Vector3f::DotProduct(normal, point) > 0);
		bool normalOut(Nz::Vector3f::DotProduct(center, normal) > 0);
		return normalOut != pointSameSideThanNormal;
	});

	return lambda(a, b, c, point) && lambda(a, b, d, point) && lambda(a, c, d, point) && lambda(b, c, d, point);
}

Nz::Vector2f colorToUV(const Nz::Color & c)
{
	Nz::Vector2f pos(c.r, c.g);
	Nz::Vector2f bPos(float(c.b % 16), float(c.b / 16));
	return (pos + bPos * 256 + Nz::Vector2f(0.5f, 0.5f)) / 4096;
}

float linearInterpolation(float a, float b, float x)
{
	return a * (1 - x) + b * x;
}

float cosInterpolation(float a, float b, float x)
{
	return linearInterpolation(a, b, (1 - cos(x*float(M_PI))) / 2);
}

float cos2DInterpolation(float a, float b, float c, float d, float x, float y)
{
	return cosInterpolation(cosInterpolation(a, b, x), cosInterpolation(c, d, x), y);
}

float cos3DInterpolation(float a, float b, float c, float d, float e, float f, float g, float h, float x, float y, float z)
{
	return cosInterpolation(cos2DInterpolation(a, b, c, d, x, y), cos2DInterpolation(e, f, g, h, x, y), z);
}