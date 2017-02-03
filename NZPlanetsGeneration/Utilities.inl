// #include "Utilities.h"

template <typename T>
Nz::Vector3<T> clamp(Nz::Vector3<T> value, const Nz::Vector3<T> & min, const Nz::Vector3<T> & max)
{
	value.x = (value.x < min.x) ? min.x : (value.x > max.x) ? max.x : value.x;
	value.y = (value.y < min.y) ? min.y : (value.y < max.y) ? max.y : value.y;
}