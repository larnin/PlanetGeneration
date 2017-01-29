#ifndef BIOME_H
#define BIOME_H
#pragma once

#include <Nazara/Core/Color.hpp>
#include <vector>

enum BiomeType
{
	GROUND,
	OCEAN,
	LAKE,
	NONE
};

class Biome
{
public:
	Biome();
	Biome(float elevation, float moisture, const Nz::Color & color, BiomeType type, float deltaHue = 0, float deltaSaturation = 0, float deltaValue = 0);

	void setColor(const Nz::Color & color);
	Nz::Color getColor() const;

	inline void setDeltaHue(float hue) { m_dh = hue; }
	inline void setDeltaSaturation(float saturation) { m_ds = saturation; }
	inline void setDeltaValue(float value) { m_dv = value; }
	
	template <typename T>
	Nz::Color getRandomColor(T & generator) const;

	inline float elevation() const { return m_elevation; }
	inline float moisture() const { return m_moisture; }
	inline BiomeType type() const { return m_type; }

private:
	float m_h;
	float m_s;
	float m_v;
	float m_dh;
	float m_ds;
	float m_dv;

	float m_elevation;
	float m_moisture;

	BiomeType m_type;
};

Biome nearest(const std::vector<Biome> & biomes, float elevation, float moisture);

#include "Biome.inl"

#endif //BIOME_H