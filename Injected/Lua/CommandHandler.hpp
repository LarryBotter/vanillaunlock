#pragma once
#include "Lua.hpp"
#include "..\WowStuff\WowFuncs.hpp"
#include "..\OM.hpp"
#include "..\Util\Helpers.hpp"

inline auto UnitIdOrGuidToAddr(const char* pstr)
{
	/*if (*pstr == '0' && (pstr[1] == 'X' || pstr[1] == 'x'))
		return GetObjectByGuidTypeCheck(StringToGuid(pstr));
	else*/
		return GetPtrFromUnitId(pstr);
}

inline auto GetUnitFromLuaState(int state)
{
	if (!GetLocalPlayer())
		return 0;

	if (FrameScriptIsString(state, 1))
	{
		char* unit = FrameScriptToLstring(state, 1);
		return UnitIdOrGuidToAddr(unit);
	}
	else if (FrameScriptIsNumber(state, 1))
	{
		auto guid = (uint64)FrameScriptToNumber(state, 1);
		return GetObjectByGuidTypeCheck(guid);
	}
	return 0;
}

inline auto LuaUnitInLos(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
	{
		if (Object(addr).InLos())
			FrameScriptPushNumber(state, 1);
		else
			FrameScriptPushNumber(state, 0);
	}
	return 1;
}

inline auto LuaUnitGetDistance(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		FrameScriptPushNumber(state, (double)(Object(addr).Distance()));
	else
		FrameScriptPushNumber(state, std::numeric_limits<double>::infinity());
	return 1;
}

inline auto LuaUnitRotation(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		FrameScriptPushNumber(state, (double)(Object(addr).Rotation()));
	else
		FrameScriptPushNumber(state, 0);
	return 1;
}

inline auto LuaSetRotation(int state)
{
	if (FrameScriptIsNumber(state, 1))
	{
		float angle = (float)FrameScriptToNumber(state, 1);

		int lpa = GetLocalPlayer();
		if (lpa)
		{
			((char*(__thiscall*)(int, float))0x007C6F30)(lpa + 0x9A8, angle);
			((int*(__thiscall*)(int, int, int, float, int))0x00600A30)(lpa, PerformanceCount(), MSG_MOVE_SET_FACING, 0, 0);
		}
	}
	return 1;
}

inline auto LuaFaceRanged(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		Object(addr).FaceRanged();
	return 1;
}

inline auto LuaFaceMelee(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		Object(addr).FaceMelee();
	return 1;
}

//returns a list of hex string of all visible units GUIDs
inline auto LuaGetUnits(int state)
{
	OM_Pulse();

	for (auto unit : units)
	{
		FrameScriptPushString(state, ("0X" + tohex(unit->Guid())).c_str());
	}
	return units.size();
}

inline auto LuaUnitGetCoords(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
	{
		FrameScriptPushNumber(state, (double)(Object(addr).Coords().x));
		FrameScriptPushNumber(state, (double)(Object(addr).Coords().y));
		FrameScriptPushNumber(state, (double)(Object(addr).Coords().z));
	}
	else
	{
		FrameScriptPushNumber(state, 0);
		FrameScriptPushNumber(state, 0);
		FrameScriptPushNumber(state, 0);
	}
	return 3;
}

// by hex string GUID or by UniId 
inline auto LuaTargetByGuid(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		Object(addr).Select();
	return 1;
}

// by hex string GUID or by UniId 
inline auto LuaGetNameByGuid(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		FrameScriptPushString(state, Object(addr).Name());
	return 1;
}

// by hex string GUID or by UniId 
inline auto LuaIsFacingMelee(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		if (Object(addr).IsFacingMelee())
			FrameScriptPushNumber(state, 1);
		else
			FrameScriptPushNumber(state, 0);
	return 1;
}

// by hex string GUID or by UniId 
inline auto LuaIsFacingRanged(int state)
{
	auto addr = GetUnitFromLuaState(state);
	if (addr)
		if (Object(addr).IsFacingRanged())
			FrameScriptPushNumber(state, 1);
		else
			FrameScriptPushNumber(state, 0);
	return 1;
}

//// by hex string GUID or by UniId 
//inline auto LuaConstantFace(int state)
//{
//	auto addr = GetUnitFromLuaState(state);
//	if (addr)
//		Object(addr).CTMConstFace();
//	return 1;
//}