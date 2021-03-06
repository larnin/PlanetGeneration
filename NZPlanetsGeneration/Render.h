#ifndef RENDER_H
#define RENDER_H
#pragma once

#include "SphereSurface.h"
#include "Block.h"
#include "Planet.h"
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Math/Vector3.hpp>

template <typename T>
Nz::ModelRef render(const SphereSurface<T> & surface);

template <typename T>
Nz::Vector3f offset(const SphereBlock<T> & point, float radius) { return Nz::Vector3f::Zero(); }

Nz::Vector3f offset(const SphereBlock<float> & point, float radius);
Nz::Vector3f offset(const SphereBlock<BlockInfo> & point, float radius);

template <typename T>
unsigned int indexCount(const SphereSurface<T> & surface);

Nz::ModelRef render(const Planet & p);

#include "Render.inl"

#endif // RENDER_H