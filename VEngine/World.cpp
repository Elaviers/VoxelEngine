#include "World.hpp"
#include <ELSys/Time.hpp>

World::World() :
	_generator(Time::GetRandSeed())
{
}

byte World::Get(const int32 pos[3])
{	
	return _cellProvider.GetCell(Vector3T<int32>(Vector3(pos[0], pos[1], pos[2]) / Cell::SIZE).Floor())
	->Get(Vector3T<byte>(pos[0] % Cell::SIZE, pos[1] % Cell::SIZE, pos[2] % Cell::SIZE));
}

void World::Set(const int32 pos[3], byte id)
{
	_cellProvider.GetCell((Vector3T<int32>(Vector3(pos[0], pos[1], pos[2]) / Cell::SIZE).Floor()))
		->Set(Vector3T<byte>(pos[0] % Cell::SIZE, pos[1] % Cell::SIZE, pos[2] % Cell::SIZE), id);
}

void World::Update(const Frustum& cameraFrustum, float deltaSeconds)
{

	/* old, use regions
	Vector3T<int32> cameraCell((cameraFrustum.position / VCELLSIZE).Floor());
	Vector3T<int32> bottomLeft = cameraCell - Vector3T<int32>(CellSimRadius, CellSimRadius, CellSimRadius);

	if (!_visibleCells[0] || (*_visibleCells[(_DIAMCUBED) / 2])->GetCoords() != cameraCell)
	{
		_cellMutex.lock();

		for (int x = 0; x < CellSimDiameter; ++x)
			for (int y = 0; y < CellSimDiameter; ++y)
				for (int z = 0; z < CellSimDiameter; ++z)
				{
					Vector3T<int32> cellCoords(bottomLeft.x + x, bottomLeft.y + y, bottomLeft.z + z);

					WorldCell*& cell = _cells[cellCoords.GetData()];
					if (!cell)
						cell = _NewCell(cellCoords.GetData());

					//todo: order visibleCells from centre
					_visibleCells[x * (CellSimDiameter * CellSimDiameter) + y * CellSimDiameter + z] = &cell;

					Vector3 wcell = cell->GetCoords() * VCELLSIZE;
					cell->bVisible = cameraFrustum.OverlapsAABB(wcell, wcell + Vector3(VCELLSIZE, VCELLSIZE, VCELLSIZE));
				}

		for (int i = 0; i < THREAD_COUNT; ++i)
			_threads[i].Restart();

		_cellMutex.unlock();
	}
	else
	{
		for (WorldCell** cell : _visibleCells)
		{
			if (cell && *cell)
			{
				Vector3 wcell = (*cell)->GetCoords() * VCELLSIZE;
				(*cell)->bVisible = cameraFrustum.OverlapsAABB(wcell, wcell + Vector3(VCELLSIZE, VCELLSIZE, VCELLSIZE));
			}
		}
	}
	*/
}

void World::Draw() const
{

	/* old, regions are used now
	for (WorldCell** cell : _visibleCells)
	{
		if (cell && (*cell)->bVisible)
		{
			(*cell)->Draw();
		}
	}
	*/
}
