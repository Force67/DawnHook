#pragma once

/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#if 0
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#endif

using lua_State = __int64;
using lua_CFunction = void(*)(lua_State*);

namespace Lua
{
    lua_State *GetCurrentLuaState();

    void RegisterFunction(const char *name, lua_CFunction fn);
    bool RunFile(const wchar_t *path);

 //   bool TriggerEvent(const char *name, char *data, size_t data_size);
}

class CScriptSystem
{
public:

    bool ExecuteString(const char*, bool);
    bool ExecuteFile(const char*, size_t, const char*, bool);
    static void RaiseError(const char *, ...);

    static CScriptSystem *GetInstance();
};