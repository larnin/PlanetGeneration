#include "Block.h"
#include <algorithm>

bool Triangle::operator==(const Triangle & other)
{
	std::vector<unsigned int> t1{ block1, block2, block3 };
	std::vector<unsigned int> t2{ other.block1, other.block2, other.block3 };
	std::sort(t1.begin(), t1.end());
	std::sort(t2.begin(), t2.end());
	return t1[0] == t2[0] && t1[1] == t2[1] && t1[2] == t2[2];
}

bool Border::operator ==(const Border & other)
{
	return (point1 == other.point1 && point2 == other.point2)
		|| (point1 == other.point2 && point2 == other.point1);
}
