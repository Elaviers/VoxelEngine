#include "Player.hpp"
#include <ELCore/Context.hpp>
#include <ELSys/InputManager.hpp>

void Player::Place()
{
	_world->Set(_transform.GetPosition(), 1);
}

void Player::Break()
{
	_world->Set(_transform.GetPosition(), 0);
}

void Player::Setup(const Context& context)
{
	InputManager* inputManager = context.GetPtr<InputManager>();
	if (inputManager)
	{
		inputManager->BindAxis(EInputAxis::MOUSE_X, &_lookInput.x);
		inputManager->BindAxis(EInputAxis::MOUSE_Y, &_lookInput.y);
		inputManager->BindKeyAxis(EKeycode::D, &_movementInput.x, 1.f);
		inputManager->BindKeyAxis(EKeycode::A, &_movementInput.x, -1.f);
		inputManager->BindKeyAxis(EKeycode::SPACE, &_movementInput.y, 1.f);
		inputManager->BindKeyAxis(EKeycode::LSHIFT, &_movementInput.y, -1.f);
		inputManager->BindKeyAxis(EKeycode::W, &_movementInput.z, 1.f);
		inputManager->BindKeyAxis(EKeycode::S, &_movementInput.z, -1.f);

		inputManager->BindKeyDown(EKeycode::MOUSE_RIGHT, Callback(this, &Player::Place));
		inputManager->BindKeyDown(EKeycode::MOUSE_LEFT, Callback(this, &Player::Break));
	}
	else Debug::Error("No input manager in player setup context! Oh dear!");
}

void Player::Update(float deltaSeconds)
{
	constexpr const float sens = .1f;
	constexpr const float maxSpeed = 30.f;
	float a = 50.f * deltaSeconds;
	float d = 30.f * deltaSeconds;

	if (_movementInput.LengthSquared() < 0.1f)
	{
		//hahahahahaahahaaaaaaaaaaaaaaaaa
		_vel *= Maths::Clamp(1.f - deltaSeconds, 0.01f, 0.99f);
	}

	_vel += _transform.GetRightVector() * _movementInput.x * a +
		_transform.GetUpVector() * _movementInput.y * a +
		_transform.GetForwardVector() * _movementInput.z * a;

	if (_vel.LengthSquared() > maxSpeed * maxSpeed)
		_vel = _vel.Normalised() * maxSpeed;

	_transform.Move(_vel * deltaSeconds);
	_transform.AddRotation(Vector3(-_lookInput.y * sens, _lookInput.x * sens, 0.f));
}

#include <ELSys/GLProgram.hpp>

void Player::ApplyCameraToCurrentProgram() const
{
	GLProgram::Current().SetMatrix4(DefaultUniformVars::mat4View, _transform.GetInverseTransformationMatrix());
	GLProgram::Current().SetMatrix4(DefaultUniformVars::mat4Projection, _projection.GetMatrix());
}
