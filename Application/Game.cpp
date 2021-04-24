#include "Game.hpp"
#include <ELGraphics/DebugFrustum.hpp>
#include <ELGraphics/RenderQueue.hpp>
#include <VEngine/imgui/imgui.h>
#include <VEngine/imgui/imgui_internal.h>
#include <VEngine/imgui/imgui_impl_opengl3.h>
#include <VEngine/imgui/imgui_impl_win32.h>

void Game::_InitialiseGL()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	wglSwapIntervalEXT(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_MULTISAMPLE);

	_shaders.lit.Load("Data/Shaders/Shader.vert", "Data/Shaders/Unlit.frag");
}

void Game::_Initialise()
{
	{
		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.hInstance = ::GetModuleHandle(NULL);
		windowClass.lpszClassName = "VWindow";
		windowClass.lpfnWndProc = _WindowProc;
		windowClass.hIcon = windowClass.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
		::RegisterClassEx(&windowClass);
	}

	_glContext.CreateDummyAndUse();
	GL::LoadDummyExtensions();

	_window.Create("VWindow", "VEngine");
	_glContext.Delete();

	_glContext.Create(_window);
	_glContext.Use(_window);
	GL::LoadExtensions(_window);
	_InitialiseGL();

	_inst.Initialise();

	_player.Setup(_inst.GetContext());

	const int runLength = 8;
	const int stride = runLength * THREAD_COUNT;
	for (int i = 0; i < THREAD_COUNT; ++i)
	{
		_regionLoaders[i].SetQueue(&_regionWorkQueue, &_region);
		
		_regionLoaders[i].SetRunLength(runLength);
		_regionLoaders[i].SetStride(stride);
		_regionLoaders[i].SetOffset(i * runLength);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui::GetIO().IniFilename = NULL;

	ImGui_ImplWin32_Init(_window.GetHWND());
	ImGui_ImplOpenGL3_Init("#version 410");
}

int Game::Run()
{
	_Initialise();
	_window.Show();

	const Texture* tex = _inst.Textures().Get("block", _inst.GetContext()).Ptr();

	bool running = true;

	float deltaSeconds = 1.f / 60.f;
	Timer timer;
	RenderQueue q;

	_region.SetWorkQueue(&_regionWorkQueue);
	_region.SetSize(Vector3T(8));
	_region.LoadCells(Vector3T<int32>());

	for (int i = 0; i < THREAD_COUNT; ++i)
		_regionLoaders[i].Start();

	WindowEvent e;
	Frustum cameraFrustum;
	while (running)
	{
		if (deltaSeconds > 0.3f)
			deltaSeconds = 0.01f;

		timer.Start();

		_inst.Input().ClearMouseInput();

		while (_window.PollEvent(e))
		{
			switch (e.type)
			{
			case WindowEvent::CLOSED:
				running = false;
				break;

			case WindowEvent::RESIZE:
				_player.UpdateProjection(Vector2T<uint16>(e.data.resize.w, e.data.resize.h));
				glViewport(0, 0, e.data.resize.w, e.data.resize.h);
				break;

			case WindowEvent::LEFTMOUSEDOWN:
				_looking = !_looking;
				break;

			case WindowEvent::FOCUS_LOST:
				_looking = false;
				break;
			}

			_inst.HandleEvent(e);
		}

		if (_looking)
		{
			RECT wr;
			::GetWindowRect(_window.GetHWND(), &wr);
			::SetCursorPos((wr.left + wr.right) / 2, (wr.top + wr.bottom) / 2);
			
		}

		_player.Update(deltaSeconds, _looking);
		_player.GetProjection().ToFrustum(_player.GetTransform(), cameraFrustum);
		_window.SetTitle(CSTR("VEngine (", _player.GetTransform().GetPosition(), ") (", _player.GetTransform().GetRotation().GetEuler(), ")"));

		Vector3T<int32> playerCell = (_player.GetTransform().GetPosition() / Cell::SIZE).Floor();
		if (_region.GetCentreCellCoords() != playerCell)
		{
			_region.LoadCells(playerCell);

			for (int i = 0; i < THREAD_COUNT; ++i)
				_regionLoaders[i].Start();
		}

		_inst.Debug().Update(deltaSeconds);
		_region.UpdateRenderableCells(cameraFrustum);

		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("VEngine");

			ImGui::End();
		}

		q.Clear();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_shaders.lit.Use();
		_shaders.lit.SetInt(DefaultUniformVars::intTextureDiffuse, 0);
		_shaders.lit.SetInt(DefaultUniformVars::intTextureNormal, 1);
		_shaders.lit.SetInt(DefaultUniformVars::intTextureSpecular, 2);
		_shaders.lit.SetInt(DefaultUniformVars::intTextureReflection, 3);
		_shaders.lit.SetVec2(DefaultUniformVars::vec2UVScale, Vector2(1.f, 1.f));
		_shaders.lit.SetVec4(DefaultUniformVars::vec4Colour, Colour::White);
		_player.ApplyCameraToCurrentProgram();

		tex->Bind(0);
		_region.Draw();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		q.Render(ERenderChannels::SURFACE | ERenderChannels::UNLIT, &_inst.Meshes(), &_inst.Textures(), 0);
		_window.SwapBuffers();

		deltaSeconds = timer.SecondsSinceStart();
	}

	for (int i = 0; i < THREAD_COUNT; ++i)
		_regionLoaders[i].Stop();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();

	return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Game::_WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui::GetCurrentContext())
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return TRUE;

		ImGuiIO& io = ImGui::GetIO();

		if (io.WantCaptureMouse && WindowFunctions_Win32::IsMouseInput(msg))
			return 0;
		if (io.WantCaptureKeyboard && WindowFunctions_Win32::IsKeyInput(msg))
			return 0;
	}

	return WindowFunctions_Win32::WindowProc(hwnd, msg, wparam, lparam);
}
