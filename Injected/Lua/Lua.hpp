#pragma once
#include <string>
#include <time.h>
#include <algorithm>
#include "WowStuff\WowFuncs.hpp"

inline void FramescriptExecute(const char* text)
{
	((void(__fastcall*)(const char*, const char*))0x00704CD0)(text, text);
}

inline auto FramescriptRegister(const char* command, const int func_pointer)
{
	return ((bool(__fastcall*)(const char*, int))0x00704120)(command, func_pointer);
}

inline auto FramescriptUnregister(const char* command)
{
	return ((bool(__fastcall*)(const char*))0x00704160)(command);
}

inline auto FrameScriptIsString(int state, int n_param)
{
	return ((bool(__fastcall*)(int, int))0x006F3510)(state, n_param);
}

inline auto FrameScriptIsNumber(int state, int n_param)
{
	return ((bool(__fastcall*)(int, int))0x006F34D0)(state, n_param);
}

inline auto FrameScriptToLstring(int state, int n_param)
{
	return ((char*(__fastcall*)(int, int))0x006F3690)(state, n_param);
}

inline auto FrameScriptToNumber(int state, int n_param)
{
	return ((double(__fastcall*)(int, int))0x006F3620)(state, n_param);
}

inline auto FrameScriptPushNumber(int state, double number)
{
	return ((int(__fastcall*)(int, int, double))0x006F3810)(state, 0, number);	//is a thiscall
}

inline auto FrameScriptPushString(int state, const char* text)
{
	return ((int(__fastcall*)(int, const char*))0x006F3890)(state, text);
}

inline auto FrameScriptPushNil(int state)
{
	return ((int(__fastcall*)(int))0x006F37F0)(state);
}

//inline auto FrameScriptDisplayError(int state, const char* err, ...)
//{
//	return ((int(__cdecl*)(int, const char*, ...))0x0084F280)(state, err);
//}
//
//inline auto FrameScriptCreateTable(int state, int column, int row)
//{
//	return ((int(__cdecl*)(int, int, int))0x0084E6E0)(state, column, row);
//}
//
inline auto GetPtrFromUnitId(const char* unit_id)
{
	return ((int(__fastcall*)(const char*))0x00515940)(unit_id);	// is a thiscall
}
//
//inline auto StringToGuid(const char* guid_string)
//{
//	return ((uint64(__cdecl*)(const char*))0x0074D120)(guid_string);
//}


inline auto chat(std::string message)
{	
	if(!GetLocalPlayer())
		return;

	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, 80, "[%H:%M:%S] ", &timeinfo);	
	
	std::string taggedMessage = buffer + message;
	std::replace(taggedMessage.begin(), taggedMessage.end(), '\'', '_');
	std::string to_print = "DEFAULT_CHAT_FRAME:AddMessage('" + taggedMessage + "')";
	FramescriptExecute(to_print.c_str());
}