#pragma once
#include "CellProvider.hpp"

class PositionedCellHandle
{
private:
	friend class CellQueue;

	size_t _index;
	CellHandle _handle;

	PositionedCellHandle(size_t index, const CellHandle& handle) : _index(0), _handle(handle) {}

public:
	PositionedCellHandle() : _index(0) {}
	void Release() { _handle.Release(); }
	Cell& operator*() { return *_handle; }
	Cell* operator->() { return _handle.operator->(); }
	operator bool() const { return _handle; }
};

class CellQueue
{
	Buffer<CellHandle> _cells;
	std::atomic_int _begin;
	std::atomic_int _end;
	std::mutex _lock;

public:
	CellQueue();

	size_t GetSize() const { _cells.GetSize(); }
	void SetSize(size_t size) { _cells.SetSize(size); }

	void Reset();
	void Add(const CellHandle& cell);
	CellHandle Next();
	bool Next(PositionedCellHandle& pHandle, int offset, int stride, int runlength);
};
