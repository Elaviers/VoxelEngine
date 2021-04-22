#pragma once
#include "Cell.hpp"
#include "Tree3D.hpp"

class CellHandle
{
private:
	friend class CellProvider;

	struct _PCell
	{
		std::atomic_int users;
		Cell* cell;
		CellProvider& provider;

		_PCell(CellProvider& provider) : provider(provider), users(0), cell(nullptr) {}
	};

	_PCell* _cell;

	CellHandle(_PCell* pCell);

public:
	CellHandle() : _cell(nullptr) {}
	CellHandle(const CellHandle& other);
	CellHandle(CellHandle&& other);
	~CellHandle();

	void Release();

	CellHandle& operator=(const CellHandle& other);
	CellHandle& operator=(CellHandle&& other) noexcept;

	operator bool() const { return _cell != nullptr; }
	Cell& operator*() const { return *_cell->cell; }
	Cell* operator->() const { return _cell->cell; }
};

class CellProvider
{
private:
	Tree3D<CellHandle::_PCell*, int32> _cells;

	std::mutex _lock;

	friend class CellHandle;
	void _NullLocation(const Vector3T<int32>& location);

public:
	CellHandle GetCell(const Vector3T<int32>& location);

};
