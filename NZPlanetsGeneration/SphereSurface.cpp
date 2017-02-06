
#include "SphereSurface.h"

unsigned int vertexForEdge(Lookup& lookup, std::vector<Nz::Vector3f>& vertices, unsigned int first, unsigned int second)
{
	Lookup::key_type key(first, second);
	if (key.first>key.second)
		std::swap(key.first, key.second);

	auto inserted = lookup.insert({ key, vertices.size() });
	if (inserted.second)
	{
		auto& edge0 = vertices[first];
		auto& edge1 = vertices[second];
		auto point = (edge0 + edge1).Normalize();
		vertices.push_back(point);
	}

	return inserted.first->second;
}

std::vector<SphereTriangle> subdivide(std::vector<Nz::Vector3f>& vertices, const std::vector<SphereTriangle>& triangles)
{
	Lookup lookup;
	std::vector<SphereTriangle> result;

	for (auto&& each : triangles)
	{
		std::array<unsigned int, 3> mid;
		std::array<unsigned int, 3> points{ each.block1, each.block2 , each.block3 };
		for (int edge = 0; edge<3; ++edge)
		{
			mid[edge] = vertexForEdge(lookup, vertices,
				points[edge], points[(edge + 1) % 3]);
		}

		result.push_back({ points[0], mid[0], mid[2] });
		result.push_back({ points[1], mid[1], mid[0] });
		result.push_back({ points[2], mid[2], mid[1] });
		result.push_back({ mid[0], mid[1], mid[2] });
	}

	return result;
}
