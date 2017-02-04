#include "Perlin3D.h"
#include "Utilities.h"
#include <random>
#include <vector>
#include <iostream>

Perlin3D::Perlin3D(const Nz::Vector3f & center, const Nz::Vector3f & size, const PerlinData2 & data)
	: m_center(center)
	, m_size(size)
	, m_matrix(Nz::Vector3ui(0, 0, 0))
{
	generate(data);
}

float Perlin3D::operator()(const Nz::Vector3f & pos)
{
	return at(m_matrix, pos);
}

float Perlin3D::at(const Matrix3f & mat, Nz::Vector3f pos) const
{
	auto size(mat.getSize());

	pos -= (m_center - m_size);
	pos = Nz::Vector3f(pos.x / (2 * m_size.x), pos.y / (2 * m_size.y), pos.z / (2 * m_size.z));

	return relativeAt(mat, pos);
}

float Perlin3D::relativeAt(const Matrix3f & mat, const Nz::Vector3f & pos) const
{
	Nz::Vector3i min(pos.x * mat.getSize().x, pos.y * mat.getSize().y, pos.z * mat.getSize().z);
	Nz::Vector3i max(min + Nz::Vector3i(1, 1, 1));
	min = clamp(min, Nz::Vector3i(0, 0, 0), Nz::Vector3i(mat.getSize()) - Nz::Vector3i(1, 1, 1));
	max = clamp(max, Nz::Vector3i(0, 0, 0), Nz::Vector3i(mat.getSize()) - Nz::Vector3i(1, 1, 1));
	Nz::Vector3f offset(clamp(pos - Nz::Vector3f(min), Nz::Vector3f(0, 0, 0), Nz::Vector3f(1, 1, 1)));

	return cos3DInterpolation(mat(Nz::Vector3ui(min)), mat(Nz::Vector3ui(max.x, min.y, min.z)), mat(Nz::Vector3ui(min.x, max.y, min.z)), mat(Nz::Vector3ui(max.x, max.y, min.z))
							, mat(Nz::Vector3ui(min.x, min.y, max.z)), mat(Nz::Vector3ui(max.x, min.y, max.z)), mat(Nz::Vector3ui(min.x, max.y, max.z)), mat(Nz::Vector3ui(max))
							, offset.x, offset.y, offset.z);
}

void Perlin3D::generate(const PerlinData2 & data)
{
	std::mt19937 engine(data.seed);

	unsigned int pointsCount(data.baseSize);
	float max(1);

	std::vector<Matrix3f> mats;

	for (unsigned int i(0); i < data.passCount; i++)
	{
		mats.push_back(perlinPass(pointsCount, engine, max));
		max /= data.passDivisor;
		pointsCount = pointsCount * 2 - 1;
	}

	m_matrix = mats.back();
	mats.pop_back();

	for (unsigned int i(0); i < m_matrix.getSize().x; i++)
		for (unsigned int j(0); j < m_matrix.getSize().y; j++)
			for (unsigned int k(0); k < m_matrix.getSize().z; k++)
			{
				Nz::Vector3f pos((float)i / m_matrix.getSize().x, (float)j / m_matrix.getSize().y, (float)k / m_matrix.getSize().z);
				for (const auto & m : mats)
					m_matrix(Nz::Vector3ui(i, j, k)) += relativeAt(m, pos);
			}
}