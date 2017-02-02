#include "Planet.h"
#include <algorithm>

Planet::Planet(float radius, std::vector<Biome> biomes)
	: SphereSurface<BlockInfo>(radius)
	, m_biomes(biomes)
{

}

bool Planet::isOnRiver(unsigned int blockID) const
{
	return std::find_if(m_rivers.begin(), m_rivers.end(), [blockID](const auto & river) 
	{
		return std::find_if(river.begin(), river.end(), [blockID](auto id) {return id == blockID; }) != river.end(); 
	}) != m_rivers.end();
}

unsigned int Planet::nearestBiomeID(float temperature, float moisture)
{
	Nz::Vector2f pos(temperature, moisture);

	unsigned int nearestBiome(0);
	float minDist(std::numeric_limits<float>::max());
	for (unsigned int i(0); i < m_biomes.size(); i++)
	{
		if (m_biomes[i].type() != BiomeType::GROUND)
			continue;

		float dist = pos.SquaredDistance(Nz::Vector2f(m_biomes[i].temperature(), m_biomes[i].moisture()));
		if (dist < minDist)
		{
			minDist = dist;
			nearestBiome = i;
		}
	}
	return nearestBiome;
}