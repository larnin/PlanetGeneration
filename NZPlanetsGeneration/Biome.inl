//#include "Biome.h"

#include <random>

template <typename T>
Nz::Color RandomColor::getRandomColor(T & generator) const
{
	float h(m_h + std::uniform_real_distribution<float>(-m_dh, m_dh)(generator));
	float s(m_s + std::uniform_real_distribution<float>(-m_ds, m_ds)(generator));
	float v(m_v + std::uniform_real_distribution<float>(-m_dv, m_dv)(generator));
	Nz::Color c(Nz::Color::FromHSV(h, s, v));
	c.a = m_a;
	return c;
}