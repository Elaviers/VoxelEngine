#pragma once
#include "Player.hpp"
#include <VEngine/CellLoader.hpp>
#include <VEngine/VInstance.hpp>
#include <ELSys/GLContext.hpp>
#include <ELSys/GLProgram.hpp>
#include <ELSys/Timer.hpp>
#include <ELSys/Window.hpp>

class Game
{
private:
	GLContext _glContext;
	Window _window;
	VInstance _inst;

	Player _player;
	Region _region;

	static const int THREAD_COUNT = 8;
	CellLoader _regionLoaders[THREAD_COUNT];

	CellQueue _regionWorkQueue;

	struct shaders_T
	{
		GLProgram lit;
	} _shaders;

	void _InitialiseGL();
	void _Initialise();

public:
	Game() : _player(_inst.GetWorld()), _region(_inst.GetWorld()) {}
	~Game() {}

	int Run();
};

