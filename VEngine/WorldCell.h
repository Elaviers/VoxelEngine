#pragma once
#include "Cell.hpp"

constexpr const int VCELLSIZE = 32;
constexpr const int CellSimRadius = 8;
constexpr const int CellSimDiameter = (CellSimRadius * 2 + 1);

class WorldCell : public Cell<VCELLSIZE>
{
public:
	std::atomic_bool bVisible = false;
	std::atomic_bool bGenerating = false;
	std::atomic_bool bUpdating = false;

	std::atomic_bool bLoaded = false;

	WorldCell() {}
	~WorldCell() {}
};
