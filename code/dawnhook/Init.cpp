
/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#include <Windows.h>
#include <cstdio>
#include <Nomad/nomad_base_function.h>
#include <Utility/PathUtils.h>
#include <Menu.h>
#include <Hooking.h>
#include <MinHook.h>

bool IsGameGenuine()
{
	char *code = (char*)LoadLibraryW(L"uplay_r1_loader64.dll") + 0x3D727;

	DWORD oldProtect;
	VirtualProtect(code, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	bool result = *(BYTE*)code == 0x83;
	VirtualProtect(code, 1, oldProtect, nullptr);

	return result;
}

bool CheckViability()
{
	// TODO: query module directory
	if (!GetModuleHandle(L"dinput8.dll"))
	{
		MessageBoxW(nullptr, L"DawnHook does not support beeing injected.", L"DawnHook", MB_ICONWARNING);
		return false;
	}

	return true;
}

bool InitMod()
{
	if (!CheckViability())
		return false;

	auto base = GetModuleHandle(L"FC_m64.dll");

	// Console
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	// Console Window
	auto consolew = GetConsoleWindow();
	RECT rect;
	GetWindowRect(consolew, &rect);

	SetConsoleTitleW(L"DawnHook Debug");

	freopen("CON", "w", stdout);
	freopen("CONIN$", "r", stdin);

	puts("Welcome to DawnHook");

	if (!IsGameGenuine())
		puts("Warning: I can't support issues with cracked games");

	auto check = [](const std::wstring &path)
	{
		auto abs = Utility::MakeAbsolutePathW(path);

		if (GetFileAttributesW(abs.c_str()) == INVALID_FILE_ATTRIBUTES)
			CreateDirectoryW(abs.c_str(), nullptr);
	};

	check(L"dawnhook");
	check(L"dawnhook\\scripts");
	check(L"dawnhook\\plugins");
	check(L"dawnhook\\mods");

	MH_Initialize();
	nio::set_base((uintptr_t)base);

	nomad::base_function::run_all();

	g_Menu = new Menu;

	return true;
}

//CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)CH_Thread, hinstDLL, NULL, nullptr);
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
		return InitMod();

	if (fdwReason == DLL_PROCESS_DETACH)
	{
		if (g_Menu)
			delete g_Menu;
	}

    return true;
}