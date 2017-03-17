#pragma once
#include "..\WowStuff\WowFuncs.hpp"

class Object
{
public:
	int addr;

	Object() :addr{} {}

	Object(int arg_addr) :addr{ arg_addr } {}

	virtual ~Object() {}

	int Descriptors() const { return addr ? *(int*)(addr + 8) : 0; }

	template<typename DSC>
	DSC GetDescriptor(const int offset) const
	{
		int desc = Descriptors();
		return desc ? *(DSC*)(desc + offset * 4) : 0;
	}

	void* GetPointerDescriptor(const int offset) const
	{
		return (void*)(Descriptors() + offset * 4);
	}

	int GetVirtualFuncAddr(const int offset) const { return addr ? *(int*)(*(int*)addr + 4 * offset) : 0; }

	uint64 Guid() const { return GetDescriptor<uint64>(OBJECT_FIELD_GUID); }

	char* Name() const { return  addr ? ((char*(__thiscall*)(int))GetVirtualFuncAddr(28))(addr) : ""; }

	C3Vector Coords() const
	{
		C3Vector res;
		if (addr)
			((void(__thiscall*)(int, C3Vector*))GetVirtualFuncAddr(5))(addr, &res);

		return res;
	}

	bool HasZeroCoords() const { return Coords().Invalid(); }

	bool Select()
	{
		if (!addr)
			return false;

		uint64 guid = Guid();
		return guid? ((void(__cdecl*)(uint64))0x00493540)(guid), true : false;
	}

	void Interact()
	{
		if (addr)
			((void(__thiscall*)(int))GetVirtualFuncAddr(24))(addr);
	}

	float Rotation() const
	{
		return addr ? ((float(__thiscall*)(int))GetVirtualFuncAddr(6))(addr) : 0;
	}

	float DistanceTo(const C3Vector& to) const { return Coords().DistanceTo(to); }

	float DistanceTo(const Object* other) const
	{
		if (!other)
			return std::numeric_limits<float>::infinity();

		//TODO consider the bounding raduis please

		return other->Coords().DistanceTo(Coords());
	}

	float Distance() const
	{		
		return DistanceTo(&Object(GetLocalPlayer()));
	}	

	bool InMeleeTo(Object* other) const
	{
		return DistanceTo(other) < MeleeRange;
	}

	bool InMelee() const
	{
		return Distance() < MeleeRange;
	}

	bool Traceline(C3Vector& start, C3Vector& end, C3Vector& result, int flags) const
	{
		float dist = 1.0f;
		return ((bool(__fastcall*)(C3Vector&, C3Vector&, C3Vector&, float&, int))0x0069BFF0)(start, end, result, dist, flags);
	}

	bool InLosTo(const C3Vector& other) const
	{
		if (!addr)
			return false;
		if (other.Invalid())
			return false;
		C3Vector start = Coords();
		if (start.Invalid())
			return false;
		C3Vector result;
		start.z += 1.3f;
		C3Vector end = { other.x, other.y, other.z + 1.3f };
		return Traceline(start, end, result, 0x120171) == 0;		//0x120171  0x100121  
	}

	bool InLos() const
	{
		Object lp = Object(GetLocalPlayer());
		return InLosTo(lp.addr ? lp.Coords() : C3Vector());
	}

	int Type() const { return GetDescriptor<int>(OBJECT_FIELD_TYPE); }
	bool IsUnit() const { return (Type() & TYPEMASK_UNIT) != 0; }
	bool IsPlayer() const { return (Type() & TYPEMASK_PLAYER) != 0; }
	bool IsItem() const { return (Type() & TYPEMASK_ITEM) != 0; }

	bool HasFlag(const int idx, const int flag) const
	{
		int flags = GetDescriptor<int>(idx);
		return (flags & flag) != 0;
	}

#pragma region facing

	float NormalizeRadian(float radian)
	{
		while (radian < 0)
		{
			radian = radian + 2 * PI;
		}
		while (radian >= 2 * PI)
		{
			radian = radian - 2 * PI;
		}
		return radian;
	}

	float CalculateNeededFacing(C3Vector start, C3Vector end)
	{
		return atan2((end.y - start.y), (end.x - start.x));
	}

	float GetSideFaceRotation(float halfShootCone)
	{
		Object lp = Object(GetLocalPlayer());
		float angle = NormalizeRadian(CalculateNeededFacing(lp.Coords(), Coords()));
		float faceTo0 = NormalizeRadian(angle - lp.Rotation());
		float faceDiff = faceTo0;
		bool objectOnRightSide = false;
		float sideFaceDiff = 0;

		if (faceTo0 >= PI)
		{
			faceDiff = 2 * PI - faceTo0;
			objectOnRightSide = true;
		}

		if (faceDiff > halfShootCone)
		{
			sideFaceDiff = faceDiff - halfShootCone;

			if (!objectOnRightSide)
			{
				return sideFaceDiff + lp.Rotation();
			}
			else
			{
				return lp.Rotation() - sideFaceDiff;
			}
		}
		else
		{
			return lp.Rotation();
		}
	}

	float GetSideFaceAngle()
	{
		Object lp = Object(GetLocalPlayer());
		float angle = CalculateNeededFacing(lp.Coords(), Coords());
		float faceTo0 = NormalizeRadian(angle - lp.Rotation());

		if (faceTo0 > PI)
		{
			faceTo0 = -(2 * PI - faceTo0);
		}
		return faceTo0;
	}

	bool IsFacing(float angle)
	{
		Object lp = Object(GetLocalPlayer());
		return lp.Rotation() == GetSideFaceRotation(angle);
	}

	bool IsFacingMelee() { return IsFacing(halfMeleeShootCone); }

	bool IsFacingRanged() { return IsFacing(halfRangedShootCone); }

	void SetFacing(float angle)
	{
		if (angle < 0.0f)
			angle += PI * 2;
		if (angle > PI * 2)
			angle -= PI * 2;

		int lpa = GetLocalPlayer();
		if (!lpa)
			return;

		((char*(__thiscall*)(int, float))0x007C6F30)(lpa + 0x9A8, angle);
		((int*(__thiscall*)(int, int, int, float, int))0x00600A30)(lpa, PerformanceCount(), MSG_MOVE_SET_FACING, 0, 0);
	}

	bool Face(float angle)
	{
		if (!IsFacing(angle))
		{
			float sfr = GetSideFaceRotation(angle);
			sfr = sfr < PI ? sfr - angle / 100 : sfr + angle / 100;

			SetFacing(sfr);
			return true;
		}
		return false;
	}

	bool FaceMelee() { return Face(halfMeleeShootCone); }

	bool FaceRanged() { return Face(halfRangedShootCone); }

#pragma endregion

	//TODO: get this work

	//std::string ToString() const
	//{		
	//	return std::string("[") + /*std::string(Name()) +*/ ", Distance = " + /*std::to_string(Distance()) +*/ ", Type = " + /*std::to_string(Type()) +*/ "]";
	//}
};

extern std::map<uint64, Object*> objects;