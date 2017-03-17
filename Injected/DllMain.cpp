#pragma once
#ifndef WIN32
#define WIN32
#endif
#include <WinSock2.h>
#include <string>
#include <cstdio>

#include "Util\KeyboardHook.hpp"
#include "WowStuff\WowConsole.hpp"
#include "Lua\CommandHandler.hpp"
#include "EndsceneHandler.hpp"
#include "WowStuff\WowClientDB.hpp"
#include "CommonDetours\ReadChatDetour.hpp"
#include "CommonDetours\NetClientProcessDetour.hpp"
#include "Memory\AntiWarden.hpp"

HINSTANCE instanceDLL = nullptr;
HWND wowWindow = nullptr;

DWORD MainThreadControl(LPVOID /* param */);
LRESULT WINAPI KeyboardHook(int/* nCode*/, WPARAM/* wParam*/, LPARAM/* lParam*/);

// entry point of the DLL
BOOL APIENTRY DllMain(HINSTANCE instDLL, DWORD reason, LPVOID /* reserved */)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		instanceDLL = instDLL;

		// disables thread notifications (DLL_THREAD_ATTACH, DLL_THREAD_DETACH)
		DisableThreadLibraryCalls(instDLL);	

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&MainThreadControl, NULL, 0, NULL);		
	}	
	else if (reason == DLL_PROCESS_DETACH)
	{	
	}
	return TRUE;
}

auto RegisterCommands()
{
	FramescriptRegister("UnitInLos", (int)LuaUnitInLos);
	FramescriptRegister("UnitGetDistance", (int)LuaUnitGetDistance);
	FramescriptRegister("UnitRotation", (int)LuaUnitRotation);
	FramescriptRegister("SetRotation", (int)LuaSetRotation);
	FramescriptRegister("IsFacingMelee", (int)LuaIsFacingMelee);
	FramescriptRegister("IsFacingRanged", (int)LuaIsFacingRanged);
	FramescriptRegister("UnitFaceRanged", (int)LuaFaceRanged);
	FramescriptRegister("UnitFaceMelee", (int)LuaFaceMelee);
	FramescriptRegister("GetUnits", (int)LuaGetUnits);
	FramescriptRegister("UnitGetCoords", (int)LuaUnitGetCoords);
	/*FramescriptRegister("TargetByGuid", (int)LuaTargetByGuid);
	FramescriptRegister("GetNameByGuid", (int)LuaGetNameByGuid);	
	FramescriptRegister("ConstantFace", (int)LuaConstantFace);*/
}

auto UnregisterCommands()
{
	FramescriptUnregister("UnitInLos");
	FramescriptUnregister("UnitGetDistance");
	FramescriptUnregister("UnitRotation");
	FramescriptUnregister("SetRotation");
	FramescriptUnregister("IsFacingMelee");
	FramescriptUnregister("IsFacingRanged");
	FramescriptUnregister("UnitFaceRanged");
	FramescriptUnregister("UnitFaceMelee");
	FramescriptUnregister("GetUnits");
	FramescriptUnregister("UnitGetCoords");
	/*FramescriptUnregister("TargetByGuid");
	FramescriptUnregister("GetNameByGuid");	
	FramescriptUnregister("ConstantFace");*/
}

auto luaCommandsRegistered{ false };
auto LoadScriptFunctionsDetour()
{
	RegisterCommands();
	luaCommandsRegistered = true;

	//---------------- return to the original function ----------------
	auto det = g_memops["LoadScriptFunctions"];
	det->Restore();
	int res = ((int(__cdecl*)())det->target)();
	det->Apply();
	return res;
}

int __cdecl CheckPermissionsDetour(int p)
{

	//---------------- return to the original function ----------------
	auto det = g_memops["CGGameUICheckPermissions"];
	det->Restore();
	int res = ((int(__fastcall*)(int))det->target)(1);	// is a thiscall
	det->Apply();
	return res;
}

int endScenePointer{};
int resetPointer{};
void GetDevicePointers();

void WINAPI MessageLoop(LPVOID lpParm)
{
	hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, instanceDLL, 0);
	MSG message;
	GetMessage(&message, NULL, 0, 0);	
}


DWORD MainThreadControl(LPVOID lpParm)
{	
	Sleep(200);
	g_memops["WardenDataHandler"] = new Detour(0x006CA5C0, (int)SMSG_WARDEN_DATA_HandlerDetour);
	

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&MessageLoop, NULL, 0, NULL);

	GetDevicePointers();

	// Fix InvalidPtrCheck for callbacks outside of .text section
	*(int*)0x884800 = 1;
	*(int*)0x884C00 = 0x7FFFFFFF;

	g_memops["EndScene"] = new Detour(endScenePointer, (int)EndSceneDetour);
	g_memops["LoadScriptFunctions"] = new Detour(0x00490250, (int)LoadScriptFunctionsDetour);
	g_memops["CGGameUICheckPermissions"] = new Detour(0x00494A50, (int)CheckPermissionsDetour);
	
	/*g_memops["NetClientProcess"] = new Detour(0x00631FE0, (int)NetClientProcessDetour);
	g_memops["ReadChat"] = new Detour(0x00966580, (int)ReadChatDetour);	*/

	if (!luaCommandsRegistered && *(int*)0x00CEEF74)
		RegisterCommands();

	//-----------------------------------  loop here before exit ----------------------------------
	//wait for the numpad 7 to be pressed...
	while (!should_exit) { }

	// exit
	UnregisterCommands();

	if (hKeyHook)	
		UnhookWindowsHookEx(hKeyHook);

	//tell the enscene hook to finish
	shouldRemoveEndSceneInjection = true;

	//wait the endscene hook to finish...
	while (!endSceneUnhooked) { }

	// unhooks all detours we created
	for (auto& det : g_memops)
	{
		delete det.second;
	}
	g_memops.clear();
	// shutdowns the bot
	// note: after that DLL's entry point will be called with reason DLL_PROCESS_DETACH
	FreeLibraryAndExitThread((HMODULE)instanceDLL, 0);
	return 0;
}

void GetDevicePointers()
{
	// Device
	int ptr;
	{
		ptr = *(int*)0x00C0ED38;
	}while (ptr == 0);

	{
		ptr = *(int*)(ptr + 0x38A8);
	}while (ptr == 0);

	// Scene	
	{
		ptr = *(int*)ptr;
	}while (ptr == 0);

	{
		endScenePointer = *(int*)(ptr + 0xA8);   //42 * 4	
	}while (endScenePointer == 0);

	{
		resetPointer = *(int*)(ptr + 0x40);   //16 * 4	
	}while (resetPointer == 0);
}








































//hKeyHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardHook, instanceDLL, 0);
//if (hKeyHook)
//	UnhookWindowsHookEx(hKeyHook);


//int countt;
//int __stdcall EndSceneDetour(int device)
//{
//	if (countt++ % 10 == 0)
//	{
//		//MessageBox(HWND_DESKTOP, "Text here", "Text here", MB_OK);
//		printf("count = %d\n", countt);
//		if (countt < 30)
//		{
//			((void(__cdecl*)(const char*, const char*, void*))0x00819210)("print('aaaaaaaaaaa')", "LetsDance", nullptr);
//		}
//	}
//	auto det = g_memops["EndScene"];
//	det->Restore();
//	int res = ((int(__stdcall*)(int))det->GetOrig())(device);
//	det->Apply();
//	return res;
//}