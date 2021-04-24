#include "Maths.hpp"
#include <ELMaths/Random.hpp>

float _PLerp(float from, float to, float alpha)
{
	if (alpha <= 0.f)
		return from;
	else if (alpha >= 1.f)
		return to;
	
	alpha = alpha * alpha * alpha * (alpha * (alpha * 6 - 15) + 10);
	return from + (to - from) * alpha;
}

Vector2 Maths::PerlinUnitVector(uint32 seed, int32 ix, int32 iy)
{
	int32 unique = ix * iy + ix + iy;
	float rad = Random(seed * unique).NextFloat() * Maths::PI_F * 2.f;
	return Vector2(Maths::Cos(rad), Maths::Sin(rad));
}

Vector3 Maths::PerlinUnitVector(uint32 seed, int32 ix, int32 iy, int32 iz)
{
	int32 unique = ix * iy * iz + ix + iy + iz;

	Random rand(seed * unique);
	float rad = rand.NextFloat() * Maths::PI_F * 2.f;
	float z = rand.NextFloat() * 2.f - 1.f;
	float t = Maths::SquareRoot(1.f - z * z);
	return Vector3(t * Maths::Cos(rad), t * Maths::Sin(rad), z);
}

__forceinline float pDot(const Vector2& v, uint32 seed, int32 cellX, int32 cellY)
{ 
	return Vector2(v.x - (float)cellX, v.y - (float)cellY).Dot(Maths::PerlinUnitVector(seed, cellX, cellY));
}

__forceinline float pDot(const Vector3& v, uint32 seed, int32 cellX, int32 cellY, int32 cellZ)
{
	return Vector3(v.x - (float)cellX, v.y - (float)cellY, v.z - (float)cellZ).Dot(Maths::PerlinUnitVector(seed, cellX, cellY, cellZ));
}

float Maths::Perlin(uint32 seed, const Vector2& v)
{
	Vector2 floor = v.Floor();
	Vector2 sub = v - floor;
	Vector2T<int32> cell = floor;
	Vector2T<int32> cell2 = cell + 1;

	float x1 = _PLerp(pDot(v, seed, cell.x, cell.y), pDot(v, seed, cell2.x, cell.y), sub.x);
	float x2 = _PLerp(pDot(v, seed, cell.x, cell2.y), pDot(v, seed, cell2.x, cell2.y), sub.x);
	return _PLerp(x1, x2, sub.y);
}

float Maths::Perlin(uint32 seed, const Vector3& v)
{
	Vector3 floor = v.Floor();
	Vector3 sub = v - floor;
	Vector3T<int32> cell = floor;
	Vector3T<int32> cell2 = cell + 1;
	
	float x1 = _PLerp(pDot(v, seed, cell.x, cell.y, cell.z), pDot(v, seed, cell2.x, cell.y, cell.z), sub.x);
	float x2 = _PLerp(pDot(v, seed, cell.x, cell2.y, cell.z), pDot(v, seed, cell2.x, cell2.y, cell.z), sub.x);
	float y1 = _PLerp(x1, x2, sub.y);

	x1 = _PLerp(pDot(v, seed, cell.x, cell.y, cell2.z), pDot(v, seed, cell2.x, cell.y, cell2.z), sub.x);
	x2 = _PLerp(pDot(v, seed, cell.x, cell2.y, cell2.z), pDot(v, seed, cell2.x, cell2.y, cell2.z), sub.x);
	float y2 = _PLerp(x1, x2, sub.y);

	return _PLerp(y1, y2, sub.z);
}
