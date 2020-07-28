#pragma once
#include <ELMaths/Projection.hpp>
#include <ELMaths/Transform.hpp>
#include <ELMaths/Vector2.hpp>
#include <VEngine/World.hpp>

class Player
{
private:
	Transform _transform;

	Vector2 _lookInput;
	Vector3 _movementInput;

	Vector3 _vel;

	Projection _projection;

	World* _world;

public:
	Player(World& world) : _world(&world) 
	{ 
		//_transform.SetPosition(Vector3(254, -113, 75)); 
		//_transform.SetRotationEuler(Vector3(8, 213, 0)); 
	}
	~Player() {}

	const Transform& GetTransform() const { return _transform; }
	const Projection& GetProjection() const { return _projection; }

	void UpdateProjection(const Vector2T<uint16>& dimensions) { _projection.SetDimensions(dimensions); }

	void Place();
	void Break();

	void Setup(const Context& context);

	void Update(float deltaSeconds);

	void ApplyCameraToCurrentProgram() const;
};
