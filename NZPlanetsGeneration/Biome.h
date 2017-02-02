#ifndef BIOME_H
#define BIOME_H
#pragma once

#include <Nazara/Core/Color.hpp>
#include <vector>

enum class BiomeType
{
	GROUND,
	OCEAN,
	LAKE,
	NONE
};

class RandomColor
{
public:
	RandomColor(const Nz::Color & color, float deltaHue = 0, float deltaSaturation = 0, float deltaValue = 0);

	inline void setDeltaHue(float hue) { m_dh = hue; }
	inline void setDeltaSaturation(float saturation) { m_ds = saturation; }
	inline void setDeltaValue(float value) { m_dv = value; }

	void RandomColor::setColor(const Nz::Color & color);
	Nz::Color getColor() const;

	template <typename T>
	Nz::Color getRandomColor(T & generator) const;

	inline void setAlpha(Nz::UInt8 alpha) { m_a = alpha; }
	inline Nz::UInt8 alpha() const { return m_a; }

private:
	float m_h;
	float m_s;
	float m_v;
	Nz::UInt8 m_a;
	float m_dh;
	float m_ds;
	float m_dv;
};

class Biome
{
public:
	Biome();
	Biome(float temperature, float moisture, BiomeType type, const RandomColor & color);

	inline Nz::Color getColor() const { return m_color.getColor(); }

	template <typename T>
	inline Nz::Color getRandomColor(T & generator) const { return m_color.getRandomColor(); }

	inline float temperature() const { return m_temperature; }
	inline float moisture() const { return m_moisture; }
	inline BiomeType type() const { return m_type; }

private:
	RandomColor m_color;

	float m_temperature;
	float m_moisture;

	BiomeType m_type;
};

#include "Biome.inl"

#endif //BIOME_H