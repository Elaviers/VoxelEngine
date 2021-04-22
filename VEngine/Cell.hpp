#pragma once
#include "VoxelRegistry.hpp"
#include <ELCore/Buffer.hpp>
#include <ELMaths/Vector3.hpp>
#include <ELSys/GL.hpp>
#include <ELSys/GLMeshRenderer.hpp>
#include <ELSys/GLProgram.hpp>
#include <ELSys/Vertex.hpp>
#include <atomic>
#include <mutex>

class Cell
{
public:
	static const int SIZE = 32;
	static const int SIZE2 = SIZE * SIZE;
	static const int SIZE3 = SIZE2 * SIZE;

private:
	const Vector3T<int32> _coords;

	byte _data[SIZE3];
	
	std::atomic_bool _bDirty;
	std::atomic_bool _bGenerated;
	bool _bVisible;
	
	GLMeshRenderer _mesh;

	std::mutex _pendingLock;
	std::atomic_bool _bPending;
	Buffer<Vertex17F> _pendingVerts;
	Buffer<uint32> _pendingElements;

	std::mutex _lock;

public:
	Cell(const Vector3T<int32>& coords) : _coords(coords), _data(), _bDirty(true) { }
	~Cell() { _mesh.Delete(); }

	void SetVisible(bool visible) { _bVisible = visible; }
	bool IsVisible() const { return _bVisible; }

	const Vector3T<int32>& GetCoords() const { return _coords; }

	byte Get(const Vector3T<byte>& pos) const;
	void Set(const Vector3T<byte>& pos, byte id);
	void LazySet(const Vector3T<byte>& pos, byte id);

	void Lock() { _lock.lock(); }
	bool TryLock() { return _lock.try_lock(); }
	void Unlock() { _lock.unlock(); }

	bool IsGenerated() const { return _bGenerated; }
	bool NeedsMeshUpdate() const { return _bDirty; }
	void MarkForMeshUpdate() { _bDirty = true; }
	void MarkGenerated() { _bGenerated = true; }

	//front, back, above, below, right, left
	void UpdateMesh(const Cell* neighbours[6]);
	void Draw();
};
