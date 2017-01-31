#include "Biome.h"
#include <Nazara/Math/Vector2.hpp>
#include <limits>
#include <cassert>

RandomColor::RandomColor(const Nz::Color & color, float deltaHue, float deltaSaturation, float deltaValue)
	: m_h(0)
	, m_s(0)
	, m_v(0)
	, m_dh(deltaHue)
	, m_ds(deltaSaturation)
	, m_dv(deltaValue)
{
	setColor(color);
}

void RandomColor::setColor(const Nz::Color & color)
{
	Nz::Color::ToHSV(color, &m_h, &m_s, &m_v);
	m_a = color.a;
}

Nz::Color RandomColor::getColor() const
{
	Nz::Color c(Nz::Color::FromHSV(m_h, m_s, m_v));
	c.a = m_a;
	return c;
}

Biome::Biome()
	: m_color(Nz::Color())
	, m_elevation(0)
	, m_moisture(0)
	, m_type(BiomeType::NONE)
{

}

Biome::Biome(float elevation, float moisture, BiomeType type, const RandomColor & color)
	: m_color(color)
	, m_elevation(elevation)
	, m_moisture(moisture)
	, m_type(type)
{

}