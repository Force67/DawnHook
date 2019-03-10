
/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#include <Nomad/nomad_base_function.h>
#include <Hooking.h>
#include "Minhook.h"

const wchar_t*(__fastcall *sub_187CD2E80___)(__int64 a1, const char *a2, unsigned int a3);

const wchar_t* __fastcall sub_187CD2E80(__int64 a1, const char *a2, unsigned int a3)
{
    auto res = sub_187CD2E80___(a1, a2, a3);

    printf("STRING %s\n", a2);

    return res;
}

const wchar_t*(*__fastcall sub_1801DEDC0____)(__int64 a1);
const wchar_t *__fastcall sub_1801DEDC0(__int64 a1)
{
    auto res = sub_1801DEDC0____(a1);

    printf("STRING %S %p\n", res, _ReturnAddress());

    return res;
}

__int64(__fastcall *sub_180215430XD)(__int64 a1, __int64 a2);
__int64 __fastcall sub_180215430(__int64 a1, __int64 a2)
{
    auto loc = sub_180215430XD(a1, a2);

   // printf("GETLOCALIZEDTEXT %p\n", _ReturnAddress());

    auto ptr = *(const wchar_t **)(loc + 8) + 12i64;

    if (!IsBadReadPtr(ptr, 255)) printf("LABEL %S %p\n", *(const wchar_t **)(a2 + 8) + 12i64,  _ReturnAddress());

    return loc;
}

void(__fastcall *j_FUNC_SetMainMenuTranslationGameStringRelatedXD)(__int64 a1, __int64 localize_string_out, __int64 hash, __int64 a4, char a5);
void __fastcall j_FUNC_SetMainMenuTranslationGameStringRelated(__int64 a1, __int64 localize_string_out, __int64 hash, __int64 a4, char a5)
{
    j_FUNC_SetMainMenuTranslationGameStringRelatedXD(a1, localize_string_out, hash, a4, a5);

    printf(__FUNCTION__ " with string %S from %p\n", *(const wchar_t **)(localize_string_out + 8) + 12i64, _ReturnAddress());
}

__int64(__fastcall *Xsub_1816F7BD0)(__int64 a1, __int64 a2, unsigned int a3, __int64 a4, char a5);
__int64 __fastcall sub_1816F7BD0(__int64 a1, __int64 a2, unsigned int a3, __int64 a4, char a5)
{
    auto res = Xsub_1816F7BD0(a1, a2, a3, a4, a5);
    
    printf(__FUNCTION__ " with string %S from %p\n", *(const wchar_t **)(a2 + 8) + 12i64, _ReturnAddress());

    return res;
}

const wchar_t *KEK()
{
    return L"CRYHOOK5 IS BAE<OptionalImageData vspace='-4'hspace='0'/>";
}

static bool yea()
{
    return true;
}

static void*org;

#if 0
int __fastcall distorm_decode64(__int64 offset, __int64 code, int codelen, __int64 a4, __int64 a5, unsigned int a6, DWORD *a7)
{
	std::printf("distorm %p, %s\n", (void*)offset, (unsigned char*)code);

	return nio::call(offset, code, codelen, a4, a5, a6, a7);
}
#endif

#if 0
static void CodexHook()
{
	auto hl = LoadLibraryW(L"uplay_r1_loader64.dll");

	if (!hl)
	{
		std::puts("!!!!");
		return;
	}

	auto p = nio::module_pattern(hl, "48 83 EC 48 45 85 C0").first();
	MH_CreateHook(p, &distorm_decode64, (void**)&org);
}
#endif

static nomad::base_function init([]()
{
	//	CodexHook();

#if 0

    // dev mode, dont initialize bloomberg crash reporter
    nio::return_function(0x18E1C3A90);
    nio::return_function(0x18E1B9CD0);

#endif

   // nio::put_call(0x1813985EB, KEK);

   // no splash screen
   //  nio::nop(0x18E0A5D63, 5);
   //nio::return_function(0x18E0A5EF0);
   //nio::nop(0x18E0A5D91, 6);

   //   nio::put_call(0x18E0A5CEC, yea);

    //nio::nop(0x18AB8BEE0, 5);
#if 0
    auto loc = (char*)0x1801DEDC0;
  //  MH_CreateHook(loc, &sub_1801DEDC0, (void**)&sub_1801DEDC0____);
  //  MH_EnableHook(loc);

    loc = (char*)0x180215430;
  //  MH_CreateHook(loc, &sub_180215430, (void**)&sub_180215430XD);
  //  MH_EnableHook(loc);

    // demo hook
   // nio::put_call(0x1813985EB, KEK);

    loc = (char*)0x1816F7BD0;
  //  MH_CreateHook(loc, &sub_1816F7BD0, (void**)&Xsub_1816F7BD0);
  //   MH_EnableHook(loc);

  //  MH_CreateHook(loc, &j_FUNC_SetMainMenuTranslationGameStringRelated, (void**)&j_FUNC_SetMainMenuTranslationGameStringRelatedXD);
  //  MH_EnableHook(loc);
#endif
});
