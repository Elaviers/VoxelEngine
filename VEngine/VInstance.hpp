#pragma once
#include "VoxelRegistry.hpp"
#include "World.hpp"
#include <ELAudio/AudioManager.hpp>
#include <ELCore/Context.hpp>
#include <ELGraphics/AnimationManager.hpp>
#include <ELGraphics/DebugManager.hpp>
#include <ELGraphics/FontManager.hpp>
#include <ELGraphics/MaterialManager.hpp>
#include <ELGraphics/MeshManager.hpp>
#include <ELGraphics/TextureManager.hpp>
#include <ELSys/InputManager.hpp>
#include <ELSys/Window.hpp>
#include <ELUI/Container.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

class VInstance
{
	Context _context;

	AudioManager _audioManager;
	AnimationManager _animationManager;
	DebugManager _debugManager;
	FontManager _fontManager;
	MaterialManager _materialManager;
	MeshManager _meshManager;
	TextureManager _textureManager;
	InputManager _inputManager;
	VoxelRegistry _voxels;
	FT_Library _ft;

	World _world;

public:
	VInstance();
	~VInstance();

	AudioManager& Audio()						{ return _audioManager; }
	AnimationManager& Animations()				{ return _animationManager; }
	DebugManager& Debug()						{ return _debugManager; }
	FontManager& Fonts()						{ return _fontManager; }
	MaterialManager& Materials()				{ return _materialManager; }
	MeshManager& Meshes()						{ return _meshManager; }
	TextureManager& Textures()					{ return _textureManager; }
	InputManager& Input()						{ return _inputManager; }
	VoxelRegistry& Voxels()						{ return _voxels; }
	World& GetWorld()							{ return _world; }

	const Context& GetContext() const			{ return _context; }
	const AudioManager& Audio() const			{ return _audioManager; }
	const AnimationManager& Animations() const	{ return _animationManager; }
	const DebugManager& Debug() const			{ return _debugManager; }
	const FontManager& Fonts() const			{ return _fontManager; }
	const MaterialManager& Materials() const	{ return _materialManager; }
	const MeshManager& Meshes() const			{ return _meshManager; }
	const TextureManager& Textures() const		{ return _textureManager; }
	const InputManager& Input() const			{ return _inputManager; }
	const VoxelRegistry& Voxels() const			{ return _voxels; }
	const World& GetWorld() const				{ return _world; }

	void Initialise();

	void HandleEvent(const WindowEvent&);
};
