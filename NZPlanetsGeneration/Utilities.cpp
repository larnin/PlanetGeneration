#include "Utilities.h"
#include "VectConvert.h"

bool isNormalOut(const Nz::Vector3f & pos1, const Nz::Vector3f & pos2, const Nz::Vector3f & pos3)
{
	Nz::Vector3f normal(Nz::Vector3f::CrossProduct(pos2 - pos1, pos3 - pos1));
	Nz::Vector3f dir(pos1 + pos2 + pos3);
	return Nz::Vector3f::DotProduct(dir, normal) > 0;
}


Nz::Vector3f cross(const Nz::Vector3f & a, const Nz::Vector3f & b)
{
	return Nz::Vector3f(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

float dot(const Nz::Vector3f & a, const Nz::Vector3f & b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Nz::Vector3f triangleOmega(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c)
{
	Nz::Vector3f ac(c - a);
	Nz::Vector3f ab(b - a);
	Nz::Vector3f abXac(cross(ab, ac));
	float sqrabXac = 2 * sqrNorm(abXac);
	if (abs(sqrabXac) <= std::numeric_limits<float>::epsilon())
		sqrabXac = 2*std::numeric_limits<float>::epsilon();

	Nz::Vector3f toCircumsphereCenter((cross(abXac, ab)*sqrNorm(ac) + cross(ac, abXac)*sqrNorm(ab)) / sqrabXac);

	return a + toCircumsphereCenter;
}

std::pair<bool, Nz::Vector3f> intersect(const Nz::Vector3f & v0, const Nz::Vector3f & v1, const Nz::Vector3f & v2, const Nz::Vector3f & p, const Nz::Vector3f & d)
{
	Nz::Vector3f e1(v1 - v0);
	Nz::Vector3f e2(v2 - v0);
	Nz::Vector3f h(cross(d, e2));
	float a(dot(e1, h));

	if (a == 0)
		return std::make_pair(false, Nz::Vector3f());

	float f(1 / a);
	Nz::Vector3f s(p - v0);
	float u(f*dot(s, h));
	if (u < 0 || u > 1)
		return std::make_pair(false, Nz::Vector3f());

	Nz::Vector3f q(cross(s, e1));
	float v(f*dot(d, q));
	if (v < 0 || u+v > 1)
		return std::make_pair(false, Nz::Vector3f());

	float t(f*dot(e2, q));
	if (t < 0)
		return std::make_pair(false, Nz::Vector3f());

	return std::make_pair(true, p + d*t);
}

Nz::Vector3f proportions(const Nz::Vector3f & a, const Nz::Vector3f & b, const Nz::Vector3f & c, const Nz::Vector3f & pos)
{
	Nz::Vector3f distancesLine(pointLineDistance(b, c, pos), pointLineDistance(a, c, pos), pointLineDistance(a, b, pos));
	return Nz::Vector3f(distancesLine.x / (distancesLine.x + norm(pos - a)), distancesLine.y / (distancesLine.y + norm(pos - b)), distancesLine.z / (distancesLine.z + norm(pos - c)));
}

float pointLineDistance(const Nz::Vector3f & line1, const Nz::Vector3f & line2, const Nz::Vector3f & pos)
{
	return norm(cross(pos - line1, pos - line2)) / norm(line2 - line1);
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