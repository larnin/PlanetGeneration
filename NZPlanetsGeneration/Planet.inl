//#include "Planet.h"

template <typename T>
Planet Planet::clone(SphereSurface<T> s, const std::vector<Biome> & biomes, BlockInfo block)
{
	Planet p(s.radius(), biomes);

	for (auto it(s.blocksBegin()); it != s.blocksEnd(); it++)
	{
		const SphereBlock<T> & point(*it);
		p.addBlock(point.pos, block);
		p.m_blocks.back().triangles = point.triangles;
	}

	for (auto it(s.trianglesBegin()); it != s.trianglesEnd(); it++)
		p.m_triangles.push_back(*it);

	p.m_builded = s.builded();

	return p;
}

template <typename T, typename U>
static Planet Planet::clone(SphereSurface<T> s, const std::vector<Biome> & biomes, U predicat)
{
	Planet p(s.radius(), biomes);

	for (auto it(s.blocksBegin()); it != s.blocksEnd(); it++)
	{
		const SphereBlock<T> & point(*it);
		p.addBlock(point.pos, predicat(point.data));
		p.m_blocks.back().triangles = point.triangles;
	}

	for (auto it(s.trianglesBegin()); it != s.trianglesEnd(); it++)
		p.m_triangles.push_back(*it);

	p.m_builded = s.builded();

	return p;
}