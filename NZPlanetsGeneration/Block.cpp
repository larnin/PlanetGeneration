#include "Block.h"
#include <algorithm>

bool SphereTriangle::operator==(const SphereTriangle & other)
{
	std::vector<unsigned int> t1{ block1, block2, block3 };
	std::vector<unsigned int> t2{ other.block1, other.block2, other.block3 };
	std::sort(t1.begin(), t1.end());
	std::sort(t2.begin(), t2.end());
	return t1[0] == t2[0] && t1[1] == t2[1] && t1[2] == t2[2];
}
