#pragma once
#include "Cell.hpp"
#include "Maths.hpp"
#include <ELMaths/Random.hpp>

class WorldGenerator
{
private:
	const uint32 _seed;

public:
	WorldGenerator(uint32 seed) : _seed(seed) {}
	~WorldGenerator() {}

	template <int SIZE>
	void GenerateCell(Cell<SIZE>& cell) const
	{
		for (int x = 0; x < SIZE; ++x)
			for (int y = 0; y < SIZE; ++y)
				for (int z = 0; z < SIZE; ++z)
				{
					float p = Maths::Perlin(_seed,
						Vector3(
							cell.GetCoords().x + (float)x / (float)SIZE,
							cell.GetCoords().y + (float)y / (float)SIZE,
							cell.GetCoords().z + (float)z / (float)SIZE));

					cell.Set(Vector3T<byte>(x, y, z), p < -0.1f);
				}
	}
};

