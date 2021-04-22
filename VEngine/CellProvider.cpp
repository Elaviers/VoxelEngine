#include "CellProvider.hpp"

CellHandle::CellHandle(_PCell* pCell) : _cell(pCell)
{
	if (_cell)
		++_cell->users;
}

CellHandle::CellHandle(const CellHandle& other) : _cell(other._cell)
{
	if (_cell)
		++_cell->users;
}

CellHandle::CellHandle(CellHandle&& other) : _cell(other._cell)
{
	other._cell = nullptr;
}

CellHandle::~CellHandle()
{
	Release();
}

void CellHandle::Release()
{
	if (_cell)
	{
		--_cell->users;
		if (_cell->users <= 0)
		{
			_cell->provider._NullLocation(_cell->cell->GetCoords());
			delete _cell->cell;
			delete _cell;
		}	

		_cell = nullptr;
	}
}

CellHandle& CellHandle::operator=(const CellHandle& other)
{
	Release();

	_cell = other._cell;
	if (_cell)
		++_cell->users;

	return *this;
}

CellHandle& CellHandle::operator=(CellHandle&& other) noexcept
{
	Release();
	_cell = other._cell;
	other._cell = nullptr;
	return *this;
}

void CellProvider::_NullLocation(const Vector3T<int32>& location)
{
	_lock.lock();

	CellHandle::_PCell** pCell = _cells.Get(location.GetData());
	if (pCell)
		(*pCell) = nullptr;
	
	_lock.unlock();
}

CellHandle CellProvider::GetCell(const Vector3T<int32>& location)
{
	_lock.lock();
	CellHandle::_PCell*& cell = _cells[location.GetData()];

	if (!cell)
		cell = new CellHandle::_PCell(*this);

	if (!cell->cell)
		cell->cell = new Cell(location);

	_lock.unlock();
	return CellHandle(cell);
}
