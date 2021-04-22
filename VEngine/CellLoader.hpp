#pragma once
#include "CellQueue.hpp"
#include "Region.hpp"

class CellLoader
{
	Region* _region;
	CellQueue* _queue;
	PositionedCellHandle _cell;

	std::thread _thread;
	std::atomic_bool _stop;
	std::atomic_bool _restart;
	
	std::atomic_int _runLength;
	std::atomic_int _stride;
	std::atomic_int _offset;

	void _Run();

public:
	CellLoader() : _region(nullptr), _queue(nullptr), _stop(true), _restart(false), _runLength(1), _stride(1), _offset(0) {}
	~CellLoader() { Stop(); }

	void SetQueue(CellQueue* queue, Region* region) { _queue = queue; _region = region; _restart = true; }
	void SetRunLength(int runLength) { _runLength = runLength; _restart = true; }
	void SetStride(int stride) { _stride = stride; _restart = true; }
	void SetOffset(int offset) { _offset = offset; _restart = true; }

	//Thread will restart if start is called on an active loader
	//Region must be set before starting
	void Start();
	void Stop();
};
