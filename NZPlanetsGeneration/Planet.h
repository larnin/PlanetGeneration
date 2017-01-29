#ifndef PLANET_H
#define PLANET_H
#pragma once

#include "SphereSurface.h"
#include "Block.h"
#include "Biome.h"
#include <vector>

struct WorldMakerData;

class Planet : public SphereSurface<BlockInfo>
{
public:
	Planet(float radius, std::vector<Biome> biomes);
	virtual ~Planet() = default;

	template <typename T>
	static Planet clone(SphereSurface<T> s, const std::vector<Biome> & biomes, BlockInfo block = BlockInfo());

	inline const Biome & biome(unsigned int index) const { return m_biomes[index]; }

private:
	std::vector<Biome> m_biomes;
	std::vector<std::vector<unsigned int>> m_riviers;
};

#include "Planet.inl"

#endif //PLANET_H 