#include "Biome.h"
#include <Nazara/Math/Vector2.hpp>
#include <limits>
#include <cassert>

Biome::Biome()
	: m_h(0)
	, m_s(0)
	, m_v(0)
	, m_dh(0)
	, m_ds(0)
	, m_dv(0)
	, m_elevation(0)
	, m_moisture(0)
	, m_type(BiomeType::GROUND)
{

}

Biome::Biome(float elevation, float moisture, const Nz::Color & color, BiomeType type, float deltaHue, float deltaSaturation, float deltaValue)
	: m_h(0)
	, m_s(0)
	, m_v(0)
	, m_dh(deltaHue)
	, m_ds(deltaSaturation)
	, m_dv(deltaValue)
	, m_elevation(elevation)
	, m_moisture(moisture)
	, m_type(type)
{
	setColor(color);
}

void Biome::setColor(const Nz::Color & color)
{
	Nz::Color::ToHSV(color, &m_h, &m_s, &m_v);
}

Nz::Color Biome::getColor() const
{
	return Nz::Color::FromHSV(m_h, m_s, m_v);
}

Biome nearest(const std::vector<Biome> & biomes, float elevation, float moisture)
{
	assert(!biomes.empty());

	Nz::Vector2f pos(elevation, moisture);

	unsigned int nearestBiome(0);
	float minDist(std::numeric_limits<float>::max());
	for (unsigned int i(0); i < biomes.size(); i++)
	{
		if (biomes[i].type() != BiomeType::GROUND)
			continue;

		float dist = pos.SquaredDistance(Nz::Vector2f(biomes[i].elevation(), biomes[i].moisture()));
		if (dist < minDist)
		{
			minDist = dist;
			nearestBiome = i;
		}
	}
	return biomes[nearestBiome];
}