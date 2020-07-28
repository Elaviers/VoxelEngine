#include "VoxelRegistry.hpp"

VoxelRegistry::~VoxelRegistry()
{
	for (Voxel* v : _voxels)
		delete v;
}
