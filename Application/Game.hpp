#pragma once
#include "Player.hpp"
#include <VEngine/CellLoader.hpp>
#include <VEngine/VInstance.hpp>
#include <ELSys/GLContext.hpp>
#include <ELSys/GLProgram.hpp>
#include <ELSys/Timer.hpp>
#include <ELSys/Window_Win32.hpp>

class Game
{
private:
	GLContext _glContext;
	Window_Win32 _window;
	VInstance _inst;

	Player _player;
	Region _region;

	bool _looking;

	static const int THREAD_COUNT = 8;
	CellLoader _regionLoaders[THREAD_COUNT];

	CellQueue _regionWorkQueue;

	struct shaders_T
	{
		GLProgram lit;
	} _shaders;

	void _InitialiseGL();
	void _Initialise();

	static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	Game() : _player(_inst.GetWorld()), _region(_inst.GetWorld()) {}
	~Game() {}

	int Run();

};

