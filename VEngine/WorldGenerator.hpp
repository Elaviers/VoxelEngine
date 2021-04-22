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

	void GenerateCell(Cell& cell) const;
};

