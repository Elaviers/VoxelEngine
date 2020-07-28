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

struct VoxelData
{
	byte id;
};

template <int SIZE>
class Cell
{
private:
	static const int _VSIZE = SIZE + 1;
	static const int _SIZESQ = SIZE * SIZE;
	static const int _SIZECUBED = SIZE * SIZE * SIZE;

	struct RenderData
	{
		int32 index;

		union
		{
			bool bRender;
			struct _sides
			{
				bool bAbove : 1;
				bool bBelow : 1;
				bool bFront : 1;
				bool bBack : 1;
				bool bRight : 1;
				bool bLeft : 1;
			} sides;
		};
	};

	Vector3T<int32> _coords;

	VoxelData _data[_SIZECUBED];
	
	bool _bDirty;
	
	GLMeshRenderer _mesh;
	Matrix4 _renderTransform;

	std::mutex _pendingLock;
	std::atomic_bool _bPending;
	Buffer<Vertex17F> _pendingVerts;
	Buffer<uint32> _pendingElements;

	static inline int _ConvertXYZ(byte x, byte y, byte z) { return x * (_SIZESQ) + y * SIZE + z; }

public:
	Cell() : _data(), _bDirty(true) { }
	~Cell() { _mesh.Delete(); }

	void SetCoords(const Vector3T<int32>& coords) { _coords = coords; }
	const Vector3T<int32>& GetCoords() const { return _coords; }

	const VoxelData& Get(const Vector3T<byte>& pos) const
	{
		return _data[_ConvertXYZ(pos.x, pos.y, pos.z)];
	}

	void Set(const Vector3T<byte>& pos, byte id)
	{
		_data[_ConvertXYZ(pos.x, pos.y, pos.z)].id = id;
		_bDirty = true;
	}

	bool NeedsUpdate() const { return _bDirty; }
	void MarkForUpdate() { _bDirty = true; }

	//front, back, above, below, right, left
	void Update(const Cell* surrounding[6])
	{
		if (_bDirty)
		{
			_bDirty = false;
			uint32 faceCount = 0;
			uint32 entryCount = 0;

			thread_local static RenderData renderdata[_SIZECUBED];

			for (uint16 i = 0; i < _SIZECUBED; ++i)
				if (_data[i].id)
					renderdata[entryCount++].index = i;

			for (int i = 0; i < entryCount; ++i)
			{
				RenderData& rd = renderdata[i];
				uint16 vi = rd.index;
				byte z = vi % SIZE;
				byte y = (vi / SIZE) % SIZE;
				byte x = vi / _SIZESQ;

				rd.sides.bFront = z < SIZE - 1 ? _data[vi + 1].id == 0 : (surrounding[0] ? surrounding[0]->_data[_ConvertXYZ(x, y, 0)].id == 0 : false);
				rd.sides.bBack = z > 0 ? _data[vi - 1].id == 0 : (surrounding[1] ? surrounding[1]->_data[_ConvertXYZ(x, y, SIZE - 1)].id == 0 : false);
				rd.sides.bAbove = y < SIZE - 1 ? _data[vi + SIZE].id == 0 : (surrounding[2] ? surrounding[2]->_data[_ConvertXYZ(x, 0, z)].id == 0 : false);
				rd.sides.bBelow = y > 0 ? _data[vi - SIZE].id == 0 : (surrounding[3] ? surrounding[3]->_data[_ConvertXYZ(x, SIZE - 1, z)].id == 0 : false);
				rd.sides.bRight = x < SIZE - 1 ? _data[vi + _SIZESQ].id == 0 : (surrounding[4] ? surrounding[4]->_data[_ConvertXYZ(0, y, z)].id == 0 : false);
				rd.sides.bLeft = x > 0 ? _data[vi - _SIZESQ].id == 0 : (surrounding[5] ? surrounding[5]->_data[_ConvertXYZ(SIZE - 1, y, z)].id == 0 : false);

				if (rd.sides.bFront) ++faceCount;
				if (rd.sides.bBack) ++faceCount;
				if (rd.sides.bAbove) ++faceCount;
				if (rd.sides.bBelow) ++faceCount;
				if (rd.sides.bRight) ++faceCount;
				if (rd.sides.bLeft) ++faceCount;
			}

			_pendingLock.lock();
			if (faceCount)
			{
				_pendingVerts.SetSize(faceCount * 4);
				_pendingElements.SetSize(faceCount * 6);

				int vi = 0;
				int ei = 0;

				for (int i = 0; i < entryCount; ++i)
				{
					const RenderData& rd = renderdata[i];
					if (rd.bRender)
					{
						uint16 vid = renderdata[i].index;
						if (_data[vid].id)
						{
							Vector3 v(vid / _SIZESQ, (vid / SIZE) % SIZE, vid % SIZE);

							Vector3 x = v + Vector3(1.f, 0.f, 0.f);
							Vector3 y = v + Vector3(0.f, 1.f, 0.f);
							Vector3 z = v + Vector3(0.f, 0.f, 1.f);
							Vector3 xy = v + Vector3(1.f, 1.f, 0.f);
							Vector3 yz = v + Vector3(0.f, 1.f, 1.f);
							Vector3 xz = v + Vector3(1.f, 0.f, 1.f);
							Vector3 xyz = v + Vector3(1.f, 1.f, 1.f);

							if (rd.sides.bFront) _AddFace(vi, ei, z, yz, xz, xyz, Vector3(0.f, 0.f, 1.f));
							if (rd.sides.bBack) _AddFace(vi, ei, v, x, y, xy, Vector3(0.f, 0.f, -1.f));
							if (rd.sides.bAbove) _AddFace(vi, ei, y, xy, yz, xyz, Vector3(0.f, 1.f, 0.f));
							if (rd.sides.bBelow) _AddFace(vi, ei, v, z, x, xz, Vector3(0.f, -1.f, 0.f));
							if (rd.sides.bRight) _AddFace(vi, ei, x, xz, xy, xyz, Vector3(1.f, 0.f, 0.f));
							if (rd.sides.bLeft) _AddFace(vi, ei, v, y, z, yz, Vector3(-1.f, 0.f, 0.f));
						}
					}
				}
			}

			_renderTransform = Matrix4::Translation(_coords * SIZE);
			_bPending = true;
			_pendingLock.unlock();
		}
	}

	static inline Vertex17F& _DefaultVertex(Vertex17F& v)
	{
		v.colour = Vector3(1.f, 1.f, 1.f);
		return v;
	}

	inline void _AddFace(int& vi, int& ei, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3& pos4, const Vector3& normal)
	{
		_pendingElements[ei++] = vi;
		Vertex17F& v1 = _DefaultVertex(_pendingVerts[vi++]);
		v1.pos = pos1;
		v1.uv = Vector2(0.f, 0.f);
		v1.normal = normal;

		_pendingElements[ei++] = vi;
		Vertex17F& v2 = _DefaultVertex(_pendingVerts[vi++]);
		v2.pos = pos2;
		v2.uv = Vector2(0.f, 1.f);
		v2.normal = normal;

		_pendingElements[ei++] = vi;
		Vertex17F& v3 = _DefaultVertex(_pendingVerts[vi++]);
		v3.pos = pos3;
		v3.uv = Vector2(1.f, 0.f);
		v3.normal = normal;

		_pendingElements[ei++] = vi;
		Vertex17F& v4 = _DefaultVertex(_pendingVerts[vi++]);
		v4.pos = pos4;
		v4.uv = Vector2(1.f, 1.f);
		v4.normal = normal;

		_pendingElements[ei++] = vi - 2;
		_pendingElements[ei++] = vi - 3;

		Vertex17F::CalculateTangents(v1, v2, v3);
		Vertex17F::CalculateTangents(v4, v3, v2);
	}

	void Draw()
	{
		if (_bPending && _pendingLock.try_lock())
		{
			_mesh.Delete();

			if (_pendingVerts.GetSize())
			{
				_mesh.Create(_pendingVerts.Data(), _pendingVerts.GetSize(), _pendingElements.Data(), _pendingElements.GetSize());
				_pendingVerts.Clear();
				_pendingElements.Clear();
			}
				
			_bPending = false;
			_pendingLock.unlock();
		}
		
		if (_mesh.IsValid())
		{
			GLProgram::Current().SetMatrix4(DefaultUniformVars::mat4Model, _renderTransform);
			_mesh.Render();
		}
	}
};
