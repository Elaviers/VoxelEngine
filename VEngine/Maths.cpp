#include "Maths.hpp"
#include <ELMaths/Random.hpp>

Vector2 Maths::PerlinUnitVector(uint32 seed, int32 ix, int32 iy)
{
	int32 unique = ix * iy + ix + iy;
	float rad = Random(seed * unique).NextFloat() * Maths::PI_F * 2.f;
	return Vector2(Maths::Cosine(rad), Maths::Sine(rad));
}

Vector3 Maths::PerlinUnitVector(uint32 seed, int32 ix, int32 iy, int32 iz)
{
	int32 unique = ix * iy * iz + ix + iy + iz;

	Random rand(seed * unique);
	float rad = rand.NextFloat() * Maths::PI_F * 2.f;
	float z = rand.NextFloat() * 2.f - 1.f;
	float t = Maths::SquareRoot(1.f - z * z);
	return Vector3(t * Maths::Cosine(rad), t * Maths::Sine(rad), z);
}

inline float pDot(const Vector2& v, uint32 seed, int32 cellX, int32 cellY) 
{ 
	return Vector2(v.x - (float)cellX, v.y - (float)cellY).Dot(Maths::PerlinUnitVector(seed, cellX, cellY));
}

inline float pDot(const Vector3& v, uint32 seed, int32 cellX, int32 cellY, int32 cellZ)
{
	return Vector3(v.x - (float)cellX, v.y - (float)cellY, v.z - (float)cellZ).Dot(Maths::PerlinUnitVector(seed, cellX, cellY, cellZ));
}

float Maths::Perlin(uint32 seed, const Vector2& v)
{
	int32 cellX1 = Maths::Floor(v.x);
	int32 cellY1 = Maths::Floor(v.y);
	int32 cellX2 = cellX1 + 1;
	int32 cellY2 = cellY1 + 1;
	float wX = v.x - (float)cellX1;
	float wY = v.y - (float)cellY1;

	float x1 = Maths::Lerp(pDot(v, seed, cellX1, cellY1), pDot(v, seed, cellX2, cellY1), wX);
	float x2 = Maths::Lerp(pDot(v, seed, cellX1, cellY2), pDot(v, seed, cellX2, cellY2), wX);
	return Maths::Lerp(x1, x2, wY);
}

float Maths::Perlin(uint32 seed, const Vector3& v)
{
	int32 cellX1 = Maths::Floor(v.x);
	int32 cellY1 = Maths::Floor(v.y);
	int32 cellZ1 = Maths::Floor(v.z);
	int32 cellX2 = cellX1 + 1;
	int32 cellY2 = cellY1 + 1;
	int32 cellZ2 = cellZ1 + 1;
	float wX = v.x - (float)cellX1;
	float wY = v.y - (float)cellY1;
	float wZ = v.z - (float)cellZ1;

	float x1 = Maths::Lerp(pDot(v, seed, cellX1, cellY1, cellZ1), pDot(v, seed, cellX2, cellY1, cellZ1), wX);
	float x2 = Maths::Lerp(pDot(v, seed, cellX1, cellY2, cellZ1), pDot(v, seed, cellX2, cellY2, cellZ1), wX);
	float y1 = Maths::Lerp(x1, x2, wY);

	x1 = Maths::Lerp(pDot(v, seed, cellX1, cellY1, cellZ2), pDot(v, seed, cellX2, cellY1, cellZ2), wX);
	x2 = Maths::Lerp(pDot(v, seed, cellX1, cellY2, cellZ2), pDot(v, seed, cellX2, cellY2, cellZ2), wX);
	float y2 = Maths::Lerp(x1, x2, wY);

	return Maths::Lerp(y1, y2, wZ);
}
