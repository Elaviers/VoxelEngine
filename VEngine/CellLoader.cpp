#include "CellLoader.hpp"

void CellLoader::_Run()
{
	while (!_stop)
	{
		if (_restart)
		{
			_restart = false;
			_cell.Release();
		}
			
		if (!_queue->Next(_cell, _offset, _stride, _runLength))
			break;

		if (_cell)
		{
			if (!_cell->IsGenerated())
			{
				_cell->Lock();
				_region->GetWorld().Generator().GenerateCell(*_cell);
				_region->UpdateCell(&*_cell);
				_cell->Unlock();
			}
			else if (_cell->NeedsMeshUpdate())
			{
				_cell->Lock();
				_region->UpdateCell(&*_cell);
				_cell->Unlock();
			}
		}
	}

	_stop = true;
}

void CellLoader::Start()
{
	if (!_stop)
		_restart = true;
	else
	{
		if (_thread.joinable())
			_thread.join();

		_stop = false;
		_restart = false;
		_thread = std::thread(&CellLoader::_Run, this);
	}
}

void CellLoader::Stop()
{
	_stop = true;
	_restart = false;

	if (_thread.joinable())
		_thread.join();

	_cell.Release();
}
