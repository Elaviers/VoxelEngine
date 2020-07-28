#pragma once
#include "Cell.hpp"
#include "WorldGenerator.hpp"
#include <ELCore/Hashmap.hpp>
#include <ELCore/List.hpp>
#include <ELCore/Pool.hpp>
#include <ELMaths/Frustum.hpp>
#include <thread>
#include <mutex>

constexpr const int VCELLSIZE = 32;
constexpr const int CellSimRadius = 8;
constexpr const int CellSimDiameter = (CellSimRadius * 2 + 1);

constexpr const int THREAD_COUNT = 8;

class WorldCell : public Cell<VCELLSIZE>
{
public:
	std::atomic_bool bVisible = false;
	std::atomic_bool bGenerating = false;
	std::atomic_bool bUpdating = false;

	std::atomic_bool bLoaded = false;

	WorldCell() {}
	~WorldCell() {}
};

class World
{
private:
	constexpr static int _DIAMSQ = CellSimDiameter * CellSimDiameter;
	constexpr static int _DIAMCUBED = CellSimDiameter * CellSimDiameter * CellSimDiameter;

	typedef MultiPool<byte, sizeof(WorldCell) * 64> _CellPoolType;
	_CellPoolType _cellPool;

	//Todo: switch to 3d tree
	Hashmap<Vector3T<int32>, WorldCell> _cells;
	WorldCell* _visibleCells[_DIAMCUBED];

	WorldGenerator _generator;

	std::mutex _cellMutex;

	class Thread
	{
		World* _world;

		std::thread _thread;

		std::atomic_bool _active;
		std::atomic_bool _restart;
		std::atomic_bool _stopped;

		std::atomic_uint _start;
		std::atomic_uint _step;

		void _main();

	public:
		Thread() : _thread(&Thread::_main, this), _active(true), _restart(false), _stopped(true), _start(0), _step(1) {}
		~Thread()
		{
			_active = false;

			while (!_thread.joinable());

			_thread.join();
		}

		void SetWorld(World* world) { _world = world; }
		void SetStart(unsigned int start) { _start = start; }
		void SetStep(unsigned int step) { _step = step; }

		void Stop() { _stopped = true; }
		void Start() { _stopped = false; }
		void Restart() { _restart = true; _stopped = false; }
	};

	Thread _threads[THREAD_COUNT];

public:
	World();
	~World() {}

	const WorldCell* GetCellIfLoaded(const Vector3T<int32>& pos) const;
	WorldCell* GetCellIfLoaded(const Vector3T<int32>& pos) { return const_cast<WorldCell*>(GetCellIfLoaded(pos)); }
	const VoxelData* GetIfLoaded(const VoxelPos& pos) const;
	WorldCell& GetCell(const Vector3T<int32>& coords, bool generate = false);
	const VoxelData& Get(const VoxelPos& pos);
	void Set(const VoxelPos& pos, byte id);

	void Update(const Frustum& cameraFrustum, float deltaSeconds);
	void Draw() const;
};
