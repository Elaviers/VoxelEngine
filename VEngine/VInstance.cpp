#include "VInstance.hpp"
#include <ELSys/Utilities.hpp>

VInstance::VInstance() : _ft(0)
{
	_context.Set(&_audioManager);
	_context.Set(&_animationManager);
	_context.Set(&_debugManager);
	_context.Set(&_fontManager);
	_context.Set(&_materialManager);
	_context.Set(&_textureManager);
	_context.Set(&_inputManager);
}

VInstance::~VInstance()
{
	FT_Done_FreeType(_ft);
}

void VInstance::Initialise()
{
	FT_Error error = FT_Init_FreeType(&_ft);
	if (error) Debug::Error("Freetype init error");

	_animationManager.Initialise();
	_animationManager.SetRootPath("Data/Animations/");
	_audioManager.Initialise();
	_audioManager.SetRootPath("Data/Audio/");
	_meshManager.Initialise();
	_meshManager.SetRootPath("Data/Meshes/");
	_textureManager.Initialise();
	_textureManager.SetRootPath("Data/Textures/");
	_textureManager.SetMaxAnisotropy(16);
	_materialManager.Initialise(_textureManager);
	_materialManager.SetRootPath("Data/Materials/");
	_fontManager.Initialise();
	_fontManager.SetRootPath("Data/Fonts/");
	_fontManager.AddPath(Utilities::GetSystemFontDir());

	_voxels.TempInit(_context);
}

void VInstance::HandleEvent(const WindowEvent& e)
{
	switch (e.type)
	{
	case WindowEvent::KEYDOWN:
		if (e.data.keyDown.isRepeat)
			return;

		_inputManager.KeyDown(e.data.keyDown.key);
		break;

	case WindowEvent::KEYUP:
		_inputManager.KeyUp(e.data.keyUp.key);
		break;

	case WindowEvent::RAWINPUT:
		_inputManager.AddMouseInput(e.data.rawInput.mouse.lastX, e.data.rawInput.mouse.lastY);
		break;

	case WindowEvent::LEFTMOUSEDOWN:
		_inputManager.KeyDown(EKeycode::MOUSE_LEFT);
		break;

	case WindowEvent::LEFTMOUSEUP:
		_inputManager.KeyUp(EKeycode::MOUSE_LEFT);
		break;

	case WindowEvent::RIGHTMOUSEDOWN:
		_inputManager.KeyDown(EKeycode::MOUSE_RIGHT);
		break;

	case WindowEvent::RIGHTMOUSEUP:
		_inputManager.KeyUp(EKeycode::MOUSE_RIGHT);
		break;
	}
}
