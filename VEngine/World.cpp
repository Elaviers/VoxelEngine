#include "World.hpp"
#include "VoxelPos.hpp"

World::World() :
	_cells(NewHandler(&_cellPool, &_CellPoolType::NewArray), DeleteHandler(&_cellPool, &_CellPoolType::DeleteHandler)),
	_visibleCells(),
	_generator(69)
{
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		_threads[i].SetStart(i);
		_threads[i].SetStep(THREAD_COUNT);
		_threads[i].SetWorld(this);
	}
}

const WorldCell* World::GetCellIfLoaded(const Vector3T<int32>& coords) const
{
	return _cells.Get(coords);
}

WorldCell& World::GetCell(const Vector3T<int32>& coords, bool generate)
{
	WorldCell* loadedCell = _cells.Get(coords);

	if (loadedCell == nullptr)
	{
		_cellMutex.lock();
		loadedCell = &_cells[coords];
		_cellMutex.unlock();

		loadedCell->SetCoords(coords);
	}

	if (generate && !loadedCell->bLoaded)
	{
		if (loadedCell->bGenerating)
			while (loadedCell->bGenerating);
		else
		{
			loadedCell->bGenerating = true;
			_generator.GenerateCell(*loadedCell);
			loadedCell->bLoaded = true;
			loadedCell->bGenerating = false;
		}
	}

	return *loadedCell;
}

const VoxelData* World::GetIfLoaded(const VoxelPos& pos) const
{
	const WorldCell* cell = GetCellIfLoaded((Vector3(pos) / VCELLSIZE).Floor());
	if (cell)
		return &cell->Get(Vector3T<byte>(pos.x % VCELLSIZE, pos.y % VCELLSIZE, pos.z % VCELLSIZE));

	return nullptr;
}

const VoxelData& World::Get(const VoxelPos& pos)
{	
	return GetCell((Vector3(pos) / VCELLSIZE).Floor()).Get(Vector3T<byte>(pos.x % VCELLSIZE, pos.y % VCELLSIZE, pos.z % VCELLSIZE));
}

void World::Set(const VoxelPos& pos, byte id)
{
	GetCell((Vector3(pos) / VCELLSIZE).Floor()).Set(Vector3T<byte>(pos.x % VCELLSIZE, pos.y % VCELLSIZE, pos.z % VCELLSIZE), id);
}

void World::Update(const Frustum& cameraFrustum, float deltaSeconds)
{
	Vector3T<int32> cameraCell((cameraFrustum.position / VCELLSIZE).Floor());
	Vector3T<int32> bottomLeft = cameraCell - Vector3T<int32>(CellSimRadius, CellSimRadius, CellSimRadius);

	if (!_visibleCells[0] || _visibleCells[(_DIAMCUBED) / 2]->GetCoords() != cameraCell)
	{
		for (int x = 0; x < CellSimDiameter; ++x)
			for (int y = 0; y < CellSimDiameter; ++y)
				for (int z = 0; z < CellSimDiameter; ++z)
				{
					Vector3T<int32> cellCoords(bottomLeft.x + x, bottomLeft.y + y, bottomLeft.z + z);
					
					//todo- GetCell is way too expensive!!
					WorldCell& cell = GetCell(cellCoords);
					_visibleCells[x * (CellSimDiameter * CellSimDiameter) + y * CellSimDiameter + z] = &cell;

					Vector3 wcell = cell.GetCoords() * VCELLSIZE;
					cell.bVisible = cameraFrustum.OverlapsAABB(wcell, wcell + Vector3(VCELLSIZE, VCELLSIZE, VCELLSIZE));
				}

		for (int i = 0; i < THREAD_COUNT; ++i)
			_threads[i].Restart();
	}
	else
	{
		for (WorldCell* cell : _visibleCells)
		{
			Vector3 wcell = cell->GetCoords() * VCELLSIZE;
			cell->bVisible = cameraFrustum.OverlapsAABB(wcell, wcell + Vector3(VCELLSIZE, VCELLSIZE, VCELLSIZE));
		}
	}
}

void World::Draw() const
{
	for (WorldCell* cell : _visibleCells)
	{
		if (cell && cell->bVisible)
		{
			cell->Draw();
		}
	}
}

void World::Thread::_main()
{
	static int indices[_DIAMCUBED];

	DO_ONCE_BEGIN;

	int mid = (_DIAMCUBED) / 2;
	indices[0] = mid;
	int i = 1;

	for (int r = 1; r <= CellSimRadius; ++r)
	{
		const int diam = r * 2 + 1;
		const int diamSq = diam * diam;
		const int iDiam = diam - 2;
				
		int start = mid - r - r * (CellSimDiameter) - r * _DIAMSQ;
		int stop = start + diam;

		//start plane
		for (int x = 0; x < diam; ++x)
		{
			for (; start < stop; ++start)
				indices[i++] = start;

			start += CellSimDiameter - diam;
			stop = start + diam;
		}

		start += _DIAMSQ - diam * CellSimDiameter;
		stop = start + diam;

		//inner planes:
		for (int x = 0; x < iDiam; ++x)
		{
			//start line
			for (; start < stop; ++start)
				indices[i++] = start;

			start += CellSimDiameter - diam;
			stop = start + diam;

			//middle lines:
			for (int y = 0; y < iDiam; ++y)
			{
				indices[i++] = start;
				indices[i++] = start + diam - 1;
				
				start += CellSimDiameter;
			}

			stop = start + diam;

			//end line
			for (; start < stop; ++start)
				indices[i++] = start;

			start += _DIAMSQ - (diam - 1) * CellSimDiameter - diam;
			stop = start + diam;
		}

		//end plane
		for (int x = 0; x < diam; ++x)
		{
			for (; start < stop; ++start)
				indices[i++] = start;

			start += CellSimDiameter - diam;
			stop = start + diam;
		}
	}

	DO_ONCE_END;

	while (_active)
	{
		while (_stopped);

		int cid = 0;
		for (int i = _start; i < _DIAMCUBED; i += _step)
		{
			if (!_active)
				return;

			if (_stopped)
				break;

			if (_restart)
			{
				_restart = false;
				break;
			}

			WorldCell* cell = _world->_visibleCells[indices[i]];
			if (cell == nullptr || cell->bGenerating || cell->bUpdating || !cell->bVisible)
				continue;

			if (!cell->bLoaded)
			{
				cell->bGenerating = true;
				_world->_generator.GenerateCell(*cell);
				cell->bLoaded = true;
				cell->bGenerating = false;
			}

			if (cell->NeedsUpdate() && !cell->bUpdating)
			{
				const WorldCell* surrounding[6] = {
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(0, 0, 1), true),
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(0, 0, -1), true),
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(0, 1, 0), true),
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(0, -1, 0), true),
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(1, 0, 0), true),
					&_world->GetCell(cell->GetCoords() + Vector3T<int32>(-1, 0, 0), true),
				};

				cell->bUpdating = true;
				cell->Update((const Cell<VCELLSIZE>**)surrounding);
				cell->bUpdating = false;
				cid++;
			}
		}

		if (cid)
			Debug::PrintLine(CSTR("Updated ", cid, " cells"));
	}
}
