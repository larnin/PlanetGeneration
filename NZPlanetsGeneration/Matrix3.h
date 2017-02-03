#ifndef MATRIX3_H
#define MATRIX3_H
#pragma once

#include <Nazara/Math/Vector3.hpp>
#include <cassert>

template <typename T>
class Matrix3
{
public:
	inline Matrix(Nz::Vector3ui size, T defaultValue = T())
		: m_datas(size.x*size.y*size.z, defaultValue)
		, m_size(size)
	{}

	inline typename std::vector<T>::reference operator()(const Nz::Vector3ui & pos)
	{
		assert(pos.x < m_size.x && pos.y < m_size.y && pos.z < m_size.z);
		return m_datas[pos.x + (pos.y + pos.z * m_size.y) * m_size.x];
	}

	inline typename std::vector<T>::const_reference operator ()(const Nz::Vector3ui & pos) const
	{
		assert(pos.x < m_size.x && pos.y < m_size.y && pos.z < m_size.z);
		return m_datas[pos.x + (pos.y + pos.z * m_size.y) * m_size.x];
	}

	inline Nz::Vector3ui getSize() const { return m_size; }

	inline typename std::vector<T>::iterator begin() { return m_datas.begin(); }
	inline typename std::vector<T>::iterator end() { return m_datas.end(); }
	inline typename std::vector<T>::const_iterator begin() const { return m_datas.begin(); }
	inline typename std::vector<T>::const_iterator end() const { return m_datas.end(); }

private:
	std::vector<T> m_datas;
	Nz::Vector3ui m_size;
};


#endif // !MATRIX3_H