#pragma once
#include <map>
#include <string>
#include <d3d9.h>
#include "..\Microsoft DirectX SDK2010\Include\d3dx9core.h"
#include "Memory\Detour.hpp"
#include "Memory\Patch.hpp"
#include "Util\Timer.hpp"
#include "Util\Helpers.hpp"
#include "Lua\Lua.hpp"
#include "WowStuff\WowFuncs.hpp"
#include "Objects\Spell.hpp"

bool shouldRemoveEndSceneInjection{};
bool endSceneUnhooked{};
std::map<std::string, MemoryOperation*> g_memops{};
bool should_exit{};

int frameCount = 0;
timer afk_timer;

//---------------- END SCENE DETOUR ------------------
int __stdcall EndSceneDetour(int device)
{
	if (GetLocalPlayer())
	{
		if (frameCount == 0)
			chat("hi");

		if (afk_timer.elapsedTime() > 150)
		{
			afk_timer.start();
			*(int*)0x00CF0BC8 = PerformanceCount();
		}

		frameCount++;
	}

	//-------- return to the original function (and remove injection if needed) --------
	auto det = g_memops["EndScene"];
	det->Restore();
	int res = ((int(__stdcall*)(int))det->target)(device);
	if (shouldRemoveEndSceneInjection)
	{
		chat("bye");
		auto it = g_memops.find("EndScene");
		delete it->second;
		g_memops.erase(it);
		
		endSceneUnhooked = true;
	}
	else
	{
		det->Apply();
	}
	return res;
}