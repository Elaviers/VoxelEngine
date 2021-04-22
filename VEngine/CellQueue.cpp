#include "CellQueue.hpp"

CellQueue::CellQueue() : _begin(0), _end(0)
{
}

void CellQueue::Reset()
{
	_lock.lock();
	for (CellHandle& cell : _cells)
		cell.Release();

	_begin = 0;
	_end = 0;
	_lock.unlock();
}

void CellQueue::Add(const CellHandle& cell)
{
	_lock.lock();

	if (_cells.GetSize() <= 0)
	{
		_lock.unlock();
		return;
	}

	CellHandle& slot = _cells[_end];
	if (!slot)
	{
		slot = cell;
		_end = (_end + 1) % _cells.GetSize();
		_lock.unlock();
	}
	else
	{
		_lock.unlock();
		Debug::PrintLine("CellQueue::Add: Cannot add to a full queue!", "[ERROR]");
	}
}

CellHandle CellQueue::Next()
{
	CellHandle pos;
	_lock.lock();
	while (_begin != _end)
	{
		pos = _cells[_begin];
		_begin = (_begin + 1) % _cells.GetSize();
		if (pos) break;
	}
	_lock.unlock();
	
	return pos;
}

bool CellQueue::Next(PositionedCellHandle& pHandle, int offset, int stride, int runlength)
{
	_lock.lock();
	if (_cells.GetSize() <= offset)
	{
		_lock.unlock();
		pHandle.Release();
		return false;
	}

	bool gotoStart = !pHandle;
	bool wasBeforeEnd = gotoStart;
	int dist;

	do
	{
		if (gotoStart)
		{
			dist = (_begin - offset) % stride;
			pHandle._index = dist < runlength ? (_begin + dist) : (_begin + (stride - dist));
			gotoStart = false;
		}
		else
		{
			wasBeforeEnd = pHandle._index < _end;
			dist = (pHandle._index - offset) % stride;
			if (dist < runlength - 1)
				++pHandle._index;
			else
				pHandle._index += stride - dist;
		}

		if (pHandle._index >= _cells.GetSize())
		{
			pHandle._index = offset;

			if (pHandle._index >= _end)
			{
				_lock.unlock();
				pHandle.Release();
				return false;
			}
		}

		if (wasBeforeEnd && pHandle._index >= _end)
		{
			_lock.unlock();
			pHandle.Release();
			return false;
		}
	} while (!_cells[pHandle._index]);

	while (!_cells[_begin] && _begin != _end)
		_begin = (_begin + 1) % _cells.GetSize();

	pHandle._handle = std::move(_cells[pHandle._index]);
	_lock.unlock();
	return true;
}
