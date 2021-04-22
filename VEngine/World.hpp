#pragma once
#include "CellProvider.hpp"
#include "Tree3D.hpp"
#include "WorldGenerator.hpp"
#include <ELCore/List.hpp>
#include <ELCore/Pool.hpp>
#include <ELMaths/Frustum.hpp>
#include <thread>
#include <mutex>

class World
{
private:
	CellProvider _cellProvider;
	WorldGenerator _generator;

public:
	World();
	~World() {}

	CellProvider& CellProvider() { return _cellProvider; }
	WorldGenerator& Generator() { return _generator; }

	byte Get(const int32 pos[3]);
	void Set(const int32 pos[3], byte id);

	void Update(const Frustum& cameraFrustum, float deltaSeconds);

	//Does nothing right now.
	//Use regions to draw cells
	void Draw() const;
};
