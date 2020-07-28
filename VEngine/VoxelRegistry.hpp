#pragma once
#include "Voxel.hpp"
#include <ELCore/Pair.hpp>

class VoxelRegistry
{
private:
	Voxel* _voxels[256];

public:
	VoxelRegistry() : _voxels() { _voxels[1] = new Voxel(); }
	~VoxelRegistry();

	void TempInit(const Context& ctx) { _voxels[1]->Initialise(ctx); }

	template <typename T, typename ...Args>
	void Register(byte id, Args... args)
	{
		Voxel*& v = _voxels[id];
		delete v;

		v = new T(args...);
	}

	Voxel* Get(byte id) { return _voxels[id]; }
	const Voxel* Get(byte id) const { return _voxels[id]; }
};
