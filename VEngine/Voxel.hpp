#pragma once
#include <ELGraphics/Material.hpp>
#include <ELMaths/Vector3.hpp>

class RenderEntry;

class Voxel
{
private:
	Material* _material;

public:
	enum class Side
	{
		TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK
	};

	Voxel() {}

	void Initialise(const Context&);
	void RenderSide(const Vector3&, Side) const;
};
