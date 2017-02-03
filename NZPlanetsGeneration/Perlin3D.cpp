#include "Perlin3D.h"

Perlin3D::Perlin3D(const Nz::Vector3f & center, const Nz::Vector3f & size, const PerlinData2 & data)
	: m_center(center)
	, m_size(size)
{

}

float Perlin3D::operator()(const Nz::Vector3f & pos)
{
	return at(m_matrix, pos);
}

float Perlin3D::at(const Matrix3f & mat, const Nz::Vector3f & pos) const
{

}