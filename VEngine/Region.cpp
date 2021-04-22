#include "Region.hpp"

Vector3T<int32> Region::GetCentreCellCoords()
{
	_cellMutex.lock();
	Vector3T<int32> coords = _cells[_cells.GetSize() / 2]->GetCoords();
	_cellMutex.unlock();
	return coords;
}

void Region::UpdateCell(Cell* cell)
{
	_cellMutex.lock();
	Vector3T<int32> delta = cell->GetCoords() - _cells[0]->GetCoords();

	Vector3T<int32> abs = ((delta - _size) - 1).Abs();
	if (abs.x > _size.x || abs.y > _size.y || abs.z > _size.z)
	{
		_cellMutex.unlock();
		return;
	}

	size_t index = delta.x * _xSkip + delta.y * _ySkip + delta.z;

	CellHandle neighbours[6] = {
		_cells[index + 1],
		_cells[index - 1],
		_cells[index + _ySkip],
		_cells[index - _ySkip],
		_cells[index + _xSkip],
		_cells[index - _xSkip]
	};

	_cellMutex.unlock();

	Cell* neighbourAddresses[6] = {
		&*neighbours[0],
		&*neighbours[1],
		&*neighbours[2],
		&*neighbours[3],
		&*neighbours[4],
		&*neighbours[5]
	};

	while (true)
	{
		int validNeighbours = 0;

		for (int i = 0; i < 6; ++i)
		{
			if (neighbourAddresses[i]->IsGenerated())
			{
				++validNeighbours;
				continue;
			}

			if (neighbourAddresses[i]->TryLock())
			{
				_world->Generator().GenerateCell(*neighbourAddresses[i]);
				neighbourAddresses[i]->Unlock();
			}
		}

		if (validNeighbours >= 6)
			break;
	}

	cell->UpdateMesh((const Cell**)neighbourAddresses);
}

void Region::_LoadCells_NoLock(const Vector3T<int32>& location)
{
	CellProvider& provider = _world->CellProvider();
	Vector3T<int32> cellLocation;

	Vector3T<int> fsize = _size + 1;

	size_t tHandle = 0;
	for (CellHandle& cell : _cells)
	{
		if (cell)
		{
			Vector3T<int32> distances = (cell->GetCoords() - location).Abs();

			if (distances.x <= fsize.x && distances.y <= fsize.y && distances.z <= fsize.z)
				_transitionHandles[tHandle++] = cell;
		}
	}
	
	cellLocation.x = location.x - _size.x - 1;
	for (int32 x = 0; x < _fullDimensions.x; ++x, ++cellLocation.x)
	{
		cellLocation.y = location.y - _size.y - 1;
		for (int32 y = 0; y < _fullDimensions.y; ++y, ++cellLocation.y)
		{
			cellLocation.z = location.z - _size.z - 1;
			for (int32 z = 0; z < _fullDimensions.z; ++z, ++cellLocation.z)
				_cells[(size_t)x * _xSkip + (size_t)y * _ySkip + z] = provider.GetCell(cellLocation);
		}
	}

	while (tHandle > 0)
		_transitionHandles[--tHandle].Release();
}

void Region::LoadCells(const Vector3T<int32>& location)
{
	_cellMutex.lock();
	_LoadCells_NoLock(location);
	_cellMutex.unlock();

	_QueueAllCells();
}

void Region::SetSize(const Vector3T<int>& size)
{
	_size = size;
	_dimensions = size * 2 + 1;
	_fullDimensions = _dimensions + 2;

	_cellMutex.lock();
	_cells.SetSize((size_t)_fullDimensions.x * _fullDimensions.y * _fullDimensions.z);
	_transitionHandles.SetSize(_cells.GetSize());
	
	if (_queue)
		_queue->SetSize(_cells.GetSize());

	_xSkip = (size_t)_fullDimensions.y * _fullDimensions.z;
	_ySkip = (size_t)_fullDimensions.z;

	if (_cells[0])
		_LoadCells_NoLock(_cells[0]->GetCoords());
	
	_cellMutex.unlock();

	_QueueAllCells();
}

void Region::UpdateRenderableCells(const Frustum& view)
{
	static const Vector3T<int32> cellDimensions(Cell::SIZE, Cell::SIZE, Cell::SIZE);

	_cellMutex.lock();
	for (const CellHandle& h : _cells)
	{
		Vector3T<int32> worldLocation = h->GetCoords() * Cell::SIZE;
		h->SetVisible(view.OverlapsAABB(worldLocation, worldLocation + cellDimensions));
	}
	_cellMutex.unlock();
}

void Region::Draw()
{
	_cellMutex.lock();
	for (const CellHandle& h : _cells)
		h->Draw();
	_cellMutex.unlock();
}

void Region::_QueueAllCells()
{
	if (_queue)
	{
		_queue->Reset();

		Vector3T<int32> localCoords;
		int32 layer = 0;
		int32 maxLayer = Maths::Max(_size.GetData(), 3);

		int32 index = 0;
		_cellMutex.lock();
		const size_t indexOffset = _cells.GetSize() / 2;
		while (layer <= maxLayer)
		{
			_queue->Add(_cells[index + indexOffset]);

			if (localCoords.z < layer)
			{
				if (Maths::Abs(localCoords.x) == layer || Maths::Abs(localCoords.y) == layer)
				{
					++localCoords.z;
					++index;
					continue;
				}
				else
				{
					localCoords.z = layer;
				}
			}
			else if (localCoords.y < layer)
			{
				++localCoords.y;
				localCoords.z = -layer;
			}
			else if (localCoords.x < layer)
			{
				++localCoords.x;
				localCoords.y = -layer;
				localCoords.z = -layer;
			}
			else
			{
				++layer;
				localCoords.x = -layer;
				localCoords.y = -layer;
				localCoords.z = -layer;
			}

			index = localCoords.x * _xSkip + localCoords.y * _ySkip + localCoords.z;
		}

		_cellMutex.unlock();
	}
	else
		Debug::Error("Region does not have a valid work queue!");
}

CellHandle Region::GetNextCell(Cell* cell, int increments)
{
	if (_cells.GetSize() <= 0 || increments < 1)
		return CellHandle();

	
	if (!cell)
	{
		if (increments <= 1)
		{
			_cellMutex.lock();
			CellHandle handle = _cells[_cells.GetSize() / 2];
			_cellMutex.unlock();

			return handle;
		}

		_cellMutex.lock();
		cell = &*_cells[_cells.GetSize() / 2];
		_cellMutex.unlock();

		--increments;
	}

	const Vector3T<int32> centrePos = GetCentreCellCoords();
	Vector3T<int32> delta = cell->GetCoords() - centrePos;
	int32 layer = Maths::Max(delta.Abs().GetData(), 3);

	Vector3T<int32> nextCoords = delta;

	while (increments-- > 0)
	{
		if (nextCoords.z < layer)
		{
			if (Maths::Abs(nextCoords.x) == layer || Maths::Abs(nextCoords.y) == layer)
				++nextCoords.z;
			else
				nextCoords.z = layer;
		}
		else if (nextCoords.y < layer)
		{
			++nextCoords.y;
			nextCoords.z = -layer;
		}
		else if (nextCoords.x < layer)
		{
			++nextCoords.x;
			nextCoords.y = -layer;
			nextCoords.z = -layer;
		}
		else
		{
			++layer;

			if (layer > _size.x)
				return CellHandle();

			nextCoords.x = -layer;
			nextCoords.y = -layer;
			nextCoords.z = -layer;
		}

		nextCoords.x = nextCoords.x;
	}

	nextCoords.x += _size.x + 1;
	nextCoords.y += _size.y + 1;
	nextCoords.z += _size.z + 1;
	size_t index = (size_t)nextCoords.x * _xSkip + (size_t)nextCoords.y * _ySkip + nextCoords.z;

	_cellMutex.lock();
	CellHandle handle = _cells[index];
	_cellMutex.unlock();

	return handle;
}
