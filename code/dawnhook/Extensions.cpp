
/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#include <ScriptSystem.h>
#include <Menu.h>

#include <Hooking.h>
#include <Nomad/nomad_base_function.h>

#define EXPOSE extern "C" __declspec (dllexport)

static void(*CScriptCallBackSystem_RegisterProperties)();

static void(*lua_pushclosure_orig)(lua_State, lua_CFunction, int32_t);
static void(*LuaRegisterFunction)(lua_State , __int64, const char *);
static void(*Agents_RegisterProperties)();

EXPOSE bool IsMenuActive()
{
	return g_MenuActive;
}

EXPOSE inline void BindLuaFunction(lua_State state, const char *name, lua_CFunction handler, bool ns = false)
{
	lua_pushclosure_orig(state, handler, 0);
	LuaRegisterFunction(state, /*0xFFFFFFFE*/0xFFFFD8EE, name);
}

static void RegisterExtensions()
{
	// first we register the org stuff
	CScriptCallBackSystem_RegisterProperties();

	std::puts("Loading extensions");

	// this is a good time point to load all scripts
	WIN32_FIND_DATAW fd;

	HANDLE handle = FindFirstFileW(L"*.asi", &fd);

	if (handle != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileW(handle, &fd))
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{

			}
		}

		FindClose(handle);
	}
}

static void RegisterExtensionsNew()
{
	Agents_RegisterProperties();

	std::puts("Registering custom lua extensions");

#if 0
	auto state = Lua::GetCurrentLuaState();
	BindLuaFunction(state, "PrintDawnHookVersion", [](lua_State)
		{
			std::printf("hello?");
		});
#endif

	/*Lua::RegisterFunction("oke", [](lua_State*)
	{
			std::printf("hello?");
	});*/
}

static nomad::base_function init([]()
{
	// we reuse the bad pattern
	char *loc = nio::pattern("31 D2 8D 4A 38 E8 ? ? ? ? 48 8D 0D ? ? ? ? 45 31 C0").first(-0x13D);
	loc = nio::get_call(loc);

	// we need these two
	CScriptCallBackSystem_RegisterProperties = 
		(decltype(CScriptCallBackSystem_RegisterProperties))nio::get_call(loc);

	char *loc2 = nio::get_call(loc) + 115;
	nio::set_dcall(&lua_pushclosure_orig, loc2);
	nio::set_dcall(&LuaRegisterFunction, loc2 + 20);

	// mount our extensions after CScriptCallbackSystem
	nio::put_ljump_inline(loc, RegisterExtensions);

	loc = nio::pattern("40 53 48 83 EC 20 31 C9 E8").first(8);
	nio::set_call(&Agents_RegisterProperties, loc);
	nio::put_call(loc, RegisterExtensionsNew);
});
