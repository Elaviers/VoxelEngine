#pragma once
#include <ELMaths/Vector2.hpp>
#include <ELMaths/Vector3.hpp>

namespace Maths
{
	Vector2 PerlinUnitVector(uint32 seed, int32 x, int32 y);
	Vector3 PerlinUnitVector(uint32, int32 x, int32 y, int32 z);

	float Perlin(uint32 seed, const Vector2&);
	float Perlin(uint32 seed, const Vector3&);

}
