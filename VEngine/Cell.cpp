#include "Cell.hpp"

static inline int _ConvertXYZ(byte x, byte y, byte z) { return (int)x * Cell::SIZE2 + (int)y * Cell::SIZE + z; }

byte Cell::Get(const Vector3T<byte>& pos) const
{
	return _data[_ConvertXYZ(pos.x, pos.y, pos.z)];
}

void Cell::Set(const Vector3T<byte>& pos, byte id)
{
	_data[_ConvertXYZ(pos.x, pos.y, pos.z)] = id;
	_bDirty = true;
}

void Cell::LazySet(const Vector3T<byte>& pos, byte id)
{
	_data[_ConvertXYZ(pos.x, pos.y, pos.z)] = id;
}

static inline Vertex17F& _DefaultVertex(Vertex17F& v)
{
	v.colour = Vector3(1.f, 1.f, 1.f);
	return v;
}

inline void _AddFace(Buffer<Vertex17F>& verts, Buffer<uint32>& elements, int& vi, int& ei, const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3& pos4, const Vector3& normal)
{
	elements[ei++] = vi;
	Vertex17F& v1 = _DefaultVertex(verts[vi++]);
	v1.pos = pos1;
	v1.uv = Vector2(0.f, 0.f);
	v1.normal = normal;

	elements[ei++] = vi;
	Vertex17F& v2 = _DefaultVertex(verts[vi++]);
	v2.pos = pos2;
	v2.uv = Vector2(0.f, 1.f);
	v2.normal = normal;

	elements[ei++] = vi;
	Vertex17F& v3 = _DefaultVertex(verts[vi++]);
	v3.pos = pos3;
	v3.uv = Vector2(1.f, 0.f);
	v3.normal = normal;

	elements[ei++] = vi;
	Vertex17F& v4 = _DefaultVertex(verts[vi++]);
	v4.pos = pos4;
	v4.uv = Vector2(1.f, 1.f);
	v4.normal = normal;

	elements[ei++] = vi - 2;
	elements[ei++] = vi - 3;

	Vertex17F::CalculateTangents(v1, v2, v3);
	Vertex17F::CalculateTangents(v4, v3, v2);
}

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

//front, back, above, below, right, left
void Cell::UpdateMesh(const Cell* surrounding[6])
{
	if (_bDirty)
	{
		_bDirty = false;
		uint32 faceCount = 0;
		uint32 entryCount = 0;

		thread_local static RenderData renderdata[SIZE3];

		for (uint16 i = 0; i < SIZE3; ++i)
			if (_data[i])
			{
				RenderData& rd = renderdata[entryCount++];
				rd.index = i;
				byte z = i % SIZE;
				byte y = (i / SIZE) % SIZE;
				byte x = i / SIZE2;

				rd.sides.bFront = z < SIZE - 1 ? _data[i + 1] == 0 : (surrounding[0] ? surrounding[0]->_data[_ConvertXYZ(x, y, 0)] == 0 : false);
				rd.sides.bBack = z > 0 ? _data[i - 1] == 0 : (surrounding[1] ? surrounding[1]->_data[_ConvertXYZ(x, y, SIZE - 1)] == 0 : false);
				rd.sides.bAbove = y < SIZE - 1 ? _data[i + SIZE] == 0 : (surrounding[2] ? surrounding[2]->_data[_ConvertXYZ(x, 0, z)] == 0 : false);
				rd.sides.bBelow = y > 0 ? _data[i - SIZE] == 0 : (surrounding[3] ? surrounding[3]->_data[_ConvertXYZ(x, SIZE - 1, z)] == 0 : false);
				rd.sides.bRight = x < SIZE - 1 ? _data[i + SIZE2] == 0 : (surrounding[4] ? surrounding[4]->_data[_ConvertXYZ(0, y, z)] == 0 : false);
				rd.sides.bLeft = x > 0 ? _data[i - SIZE2] == 0 : (surrounding[5] ? surrounding[5]->_data[_ConvertXYZ(SIZE - 1, y, z)] == 0 : false);

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
			_pendingVerts.SetSize((size_t)faceCount * 4);
			_pendingElements.SetSize((size_t)faceCount * 6);

			int vi = 0;
			int ei = 0;

			for (int i = 0; i < entryCount; ++i)
			{
				const RenderData& rd = renderdata[i];
				if (rd.bRender)
				{
					uint16 vid = renderdata[i].index;
					if (_data[vid])
					{
						Vector3 v(vid / SIZE2, (vid / SIZE) % SIZE, vid % SIZE);

						Vector3 x = v + Vector3(1.f, 0.f, 0.f);
						Vector3 y = v + Vector3(0.f, 1.f, 0.f);
						Vector3 z = v + Vector3(0.f, 0.f, 1.f);
						Vector3 xy = v + Vector3(1.f, 1.f, 0.f);
						Vector3 yz = v + Vector3(0.f, 1.f, 1.f);
						Vector3 xz = v + Vector3(1.f, 0.f, 1.f);
						Vector3 xyz = v + Vector3(1.f, 1.f, 1.f);

						if (rd.sides.bFront) _AddFace(_pendingVerts, _pendingElements, vi, ei, z, yz, xz, xyz, Vector3(0.f, 0.f, 1.f));
						if (rd.sides.bBack) _AddFace(_pendingVerts, _pendingElements, vi, ei, v, x, y, xy, Vector3(0.f, 0.f, -1.f));
						if (rd.sides.bAbove) _AddFace(_pendingVerts, _pendingElements, vi, ei, y, xy, yz, xyz, Vector3(0.f, 1.f, 0.f));
						if (rd.sides.bBelow) _AddFace(_pendingVerts, _pendingElements, vi, ei, v, z, x, xz, Vector3(0.f, -1.f, 0.f));
						if (rd.sides.bRight) _AddFace(_pendingVerts, _pendingElements, vi, ei, x, xz, xy, xyz, Vector3(1.f, 0.f, 0.f));
						if (rd.sides.bLeft) _AddFace(_pendingVerts, _pendingElements, vi, ei, v, y, z, yz, Vector3(-1.f, 0.f, 0.f));
					}
				}
			}
		}

		_bPending = true;
		_pendingLock.unlock();
	}
}

void Cell::Draw()
{
	if (_bVisible)
	{
		if (_bPending && _pendingLock.try_lock())
		{
			_mesh.Delete();

			if (_pendingVerts.GetSize())
			{
				_mesh.Create(_pendingVerts.Elements(), _pendingVerts.GetSize(), _pendingElements.Elements(), _pendingElements.GetSize());
				_pendingVerts.Clear();
				_pendingElements.Clear();
			}

			_bPending = false;
			_pendingLock.unlock();
		}

		if (_mesh.IsValid())
		{
			Matrix4 renderTransform = Matrix4::Identity();
			renderTransform[3][0] = _coords.x * SIZE;
			renderTransform[3][1] = _coords.y * SIZE;
			renderTransform[3][2] = _coords.z * SIZE;
			GLProgram::Current().SetMatrix4(DefaultUniformVars::mat4Model, renderTransform);
			_mesh.Render();
		}
	}
}
