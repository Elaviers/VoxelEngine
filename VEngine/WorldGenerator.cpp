#include "WorldGenerator.hpp"

float halfBlock = 0.5f / (float)Cell::SIZE;

void WorldGenerator::GenerateCell(Cell& cell) const
{
	for (int x = 0; x < Cell::SIZE; ++x)
		for (int y = 0; y < Cell::SIZE; ++y)
			for (int z = 0; z < Cell::SIZE; ++z)
			{
				float p = Maths::Perlin(_seed,
					Vector3(
						cell.GetCoords().x + halfBlock + (float)x / (float)Cell::SIZE,
						cell.GetCoords().y + halfBlock + (float)y / (float)Cell::SIZE,
						cell.GetCoords().z + halfBlock + (float)z / (float)Cell::SIZE));

				cell.LazySet(Vector3T<byte>(x, y, z), p > 0.2f);
			}

	cell.MarkGenerated();
	cell.MarkForMeshUpdate();
}
