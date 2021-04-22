#include "Voxel.hpp"
#include <ELCore/Context.hpp>
#include <ELGraphics/MaterialManager.hpp>
#include <ELSys/GLProgram.hpp>

float s = 1.f;
Vector3 scale = Vector3(s, s, s);
Matrix4 baseBack = Matrix4::Transformation(Vector3(0.f, 0.f, -0.5f * s), Vector3(), scale);
Matrix4 baseFront = Matrix4::Transformation(Vector3(0.f, 0.f, 0.5f * s), Vector3(0.f, 180.f, 0.f), scale);
Matrix4 baseBottom = Matrix4::Transformation(Vector3(0.f, -0.5f * s, 0.f), Vector3(90.f, 0.f, 0.f), scale);
Matrix4 baseTop = Matrix4::Transformation(Vector3(0.f, 0.5f * s, 0.f), Vector3(-90.f, 0.f, 0.f), scale);
Matrix4 baseLeft = Matrix4::Transformation(Vector3(-0.5f * s, 0.f, 0.f), Vector3(0.f, 90.f, 0.f), scale);
Matrix4 baseRight = Matrix4::Transformation(Vector3(0.5f * s, 0.f, 0.f), Vector3(0.f, -90.f, 0.f), scale);

void Voxel::Initialise(const Context& ctx)
{
	TextureManager* t = ctx.GetPtr<TextureManager>();
	MaterialManager* materialManager = ctx.GetPtr<MaterialManager>();
	_material = materialManager->Get("block", ctx).Ptr();
}

void Voxel::RenderSide(const Vector3& pos, Side side) const
{
	Matrix4 transform;
	switch (side)
	{
	case Side::FRONT:
		transform = baseFront;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	case Side::BACK:
		transform = baseBack;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	case Side::TOP:
		transform = baseTop;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	case Side::BOTTOM:
		transform = baseBottom;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	case Side::RIGHT:
		transform = baseRight;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	case Side::LEFT:
		transform = baseLeft;
		transform[3][0] += pos.x;
		transform[3][1] += pos.y;
		transform[3][2] += pos.z;
		break;
	}

	GLProgram::Current().SetMatrix4(DefaultUniformVars::mat4Model, transform);
}
