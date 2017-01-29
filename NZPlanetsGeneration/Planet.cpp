#include "Planet.h"

Planet::Planet(float radius, std::vector<Biome> biomes)
	: SphereSurface<BlockInfo>(radius)
	, m_biomes(biomes)
{

}
