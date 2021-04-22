#pragma once
#include <ELCore/Buffer.hpp>
#include "CellQueue.hpp"
#include "World.hpp"

class Cell;

class Region
{
private:
	Vector3T<int> _size; //Half-size
	Vector3T<int> _dimensions; //Usable cells
	Vector3T<int> _fullDimensions; //Usable cells + border

	World* _world;
	Buffer<CellHandle> _cells;
	Buffer<CellHandle> _transitionHandles;
	std::mutex _cellMutex;

	size_t _xSkip;
	size_t _ySkip;

	CellQueue* _queue;

	void _LoadCells_NoLock(const Vector3T<int32>& location);

	void _QueueAllCells();

public:
	Region(World& world) : _world(&world) {}

	void SetWorkQueue(CellQueue* queue) { _queue = queue; queue->SetSize(_cells.GetSize()); }

	World& GetWorld() { return *_world; }

	void UpdateCell(Cell* cell);

	Vector3T<int32> GetCentreCellCoords();
	void LoadCells(const Vector3T<int32>& location);
	
	const Vector3T<int>& GetSize() const { return _size; }
	void SetSize(const Vector3T<int>& size);
	
	void UpdateRenderableCells(const Frustum& view);
	void Draw();

	CellHandle GetNextCell(Cell* cell, int increments = 1);
};
