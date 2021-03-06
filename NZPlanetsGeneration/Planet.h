#ifndef PLANET_H
#define PLANET_H
#pragma once

#include "SphereSurface.h"
#include "Block.h"
#include "Biome.h"
#include <vector>

using River = std::vector<RivierPoint>;

class Planet : public SphereSurface<BlockInfo>
{
public:
	Planet(float radius, std::vector<Biome> biomes);
	virtual ~Planet() = default;

	template <typename T>
	static Planet clone(SphereSurface<T> s, const std::vector<Biome> & biomes, BlockInfo block = BlockInfo());
	template <typename T, typename U>
	static Planet clone(SphereSurface<T> s, const std::vector<Biome> & biomes, U predicat); // predicat must define BlockInfo operator()(T)

	inline const Biome & biome(unsigned int index) const { return m_biomes[index]; }

	inline void addRiver(const River & r) { m_rivers.push_back(r); }
	inline unsigned int riverCount() const { return m_rivers.size(); }
	inline const River & river(unsigned int id) const { return m_rivers[id]; }
	inline River & river(unsigned int id) { return m_rivers[id]; }

	bool isOnRiver(unsigned int blockID) const;
	unsigned int riverIndexAt(unsigned int blockID) const;

	unsigned int nearestBiomeID(float temperature, float moisture);

private:
	std::vector<Biome> m_biomes;
	std::vector<River> m_rivers;
};

#include "Planet.inl"

#endif //PLANET_H 