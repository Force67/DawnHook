
/*
*  This source file is part of the Far Cry 5 ScriptHook by Force67
*  More information regarding licensing can be found in LICENSE.md
*/

#include <Nomad/nomad_base_function.h>
#include <Utility/PathUtils.h>
#include <Hooking.h>
#include <MinHook.h>

#include <ScriptSystem.h>

#include <udis86.h>

static FILE *g_file;
static char *g_lastsub;
static char *g_lastprefix = "";

static void(*lua_pushclosure_orig)(__int64*, lua_CFunction, int32_t);
static void(*LuaRegisterFunction)(lua_State *, __int64, const char *);

static void lua_pushclosure_hook(__int64 * a1, lua_CFunction a2, int32_t a3)
{
	g_lastsub = (char*)a2;

    lua_pushclosure_orig(a1, a2, a3);
}

static void LuaRegisterFunction_Hook(lua_State * state, __int64 index, const char *name)
{
	/*if (!_strcmpi(name, "GetInstance"))
	{
		
	}*/

	std::printf("registering %s\n", name);
    fprintf(g_file, "MakeName	(0X%p, \"SCR_%s\");\n", g_lastsub, name);
	fflush(g_file);

#if 0
    fprintf(g_file, "{ \"name\":\"%s\", \"addr\":\"0x%p\" },\n", a3, latestsub);

    fflush(g_file);
#endif

    LuaRegisterFunction(state, index, name);
}

void __fastcall sub_182E26300(const char *ptr)
{
    fprintf(g_file, "\n \"%s\": [", ptr);
    fflush(g_file);

   // lua_regnamespace(ptr);
}

static void AnalyzeHirachy(char *code)
{
	auto get_string = [](ud_t *ud, char **name_out)
	{
		auto operand = ud_insn_opr(ud, 1);

		// if the operand is immediate
		if (operand->type == UD_OP_MEM)
		{
			// and relative to the instruction...
			if (operand->base == UD_R_RIP)
			{
				// get the string reference
				*name_out = reinterpret_cast<char*>(ud_insn_len(ud) + ud_insn_off(ud) + operand->lval.sdword);
			}
		}
	};

	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(code));
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(code), INT32_MAX);

	std::vector<std::string> sub_classes;
	char *class_name = nullptr;

	while (true)
	{
		ud_disassemble(&ud);

		// are we done ?
		if (ud_insn_mnemonic(&ud) == UD_Iret)
			break;

		// is it a string reference?
		if (ud_insn_mnemonic(&ud) == UD_Ilea)
		{
			if (!class_name)
			{
				get_string(&ud, &class_name);

				std::printf("parent class %s\n", class_name);
			}
			else
			{
				char *temp = "unknown";
				get_string(&ud, &temp);

				sub_classes.push_back(temp);
			}
		}

		// follow the base class calls
		if (ud_insn_mnemonic(&ud) == UD_Icall)
		{
			intptr_t targetPtr = static_cast<intptr_t>(ud_insn_len(&ud) + ud_insn_off(&ud) + ud_insn_opr(&ud, 0)->lval.sdword);

			// we dont care about this other call
			if (targetPtr != 0x18015AED0)
			{
				std::printf("following hirachy!\n");

				// follow the call
				ud_set_pc(&ud, targetPtr);
			}
		}
	}

	std::printf("Found class %s\n", class_name);
}

__int64(*o)(__int64, const char*, __int64);
__int64 __fastcall sub_18013D530(__int64 a1, const char* a2, __int64 a3)
{
	if (!IsBadStringPtrA(a2, 512))
	{
		if (strstr(a2, "RichPresence"))
			std::printf("logshit %s\n", a2);
	}

	return o(a1, a2, a3);
}

static nomad::base_function init([]()
{
	// quick dirty way of generating a IDC
	if (strstr(GetCommandLineA(), "-genidc"))
	{
		std::puts("Dumping functions to idc..");

		auto root = Utility::MakeAbsolutePathW(L"dawnhook\\dump");

		if (GetFileAttributesW(root.c_str()) == INVALID_FILE_ATTRIBUTES)
			CreateDirectoryW(root.c_str(), nullptr);

		g_file = _wfopen((root + L"\\lua_gen.idc").c_str(), L"w");

		fprintf(g_file,
			"#include <idc.idc>\n"
			"static main(void) {\n");

		// this is hacky too
		char *loc = nio::pattern("31 D2 8D 4A 38 E8 ? ? ? ? 48 8D 0D ? ? ? ? 45 31 C0").first(-0x13F + 2);

		// follow into CScriptCallBackSystem::RegisterProperties
		loc = nio::get_doublejmp(loc) + 115;

		char *loc2 = nio::get_call(loc);

		// store dest
		lua_pushclosure_orig = (decltype(lua_pushclosure_orig))nio::get_call(loc2);
		nio::put_ljump_inline(loc2, lua_pushclosure_hook);

		loc = nio::get_call(loc + 20);
		LuaRegisterFunction = (decltype(LuaRegisterFunction))nio::get_call(loc);
		nio::put_ljump_inline(loc, LuaRegisterFunction_Hook);
	}

	if (strstr(GetCommandLineA(), "-hirachydump"))
	{
		auto matches = nio::pattern("40 53 48 83 EC 20 48 83 3D ? ? ? ? ? 75 27 E8").count_hint(10674);

		for (size_t i = 0; i < matches.size(); i++)
		{
			char* loc = matches.get(i).get<char>();

			AnalyzeHirachy(loc);
		}
	}

	//nio::put_fatjmp(0x18013D530, &o, sub_18013D530);

	//MH_CreateHook((LPVOID)0x1864FA000, &sub_18013D530, (LPVOID*)&o);
	//MH_EnableHook(0);

#if 0
    // ugly pattern
    auto matches = nio::pattern("E8 ?? ?? ?? ?? 48 89 C3 48 85 C0 0F 84 ?? ?? ?? ?? 48 89 C1 48 89 7C 24 ?? E8").count(20);

    for (size_t i = 0; i < matches.size(); i++)
    {
        char *loc = matches.get(i).get<char>(-5 + (0x2C - 6));

        char *table = *(char**)(loc + *(int32_t*)loc + 4);

        loc += (6 + 5);

        printf("TABLE : %s\n", table);

        while (true)
        {
            if (*(uint16_t*)loc != 0x3145) break;
    
            printf("SUB : %p, %s\n", (char*)((loc + 3) + *(int32_t*)(loc + 3) + 4), *(char**)((loc + 21) + *(int32_t*)(loc + 21) + 4));

            loc += (33 + 5);
        }

        // node size 33 + 5
    }
#endif
});