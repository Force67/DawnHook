
/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#include <Memory.h>
#include <Hooking.h>
#include <Nomad/nomad_base_function.h>

static void*(*CMemMng_Alloc)(size_t, size_t);
static void(*CMemMng_Free)(void*);

void * CMemMng::Alloc(size_t size, size_t align)
{
	return CMemMng_Alloc(size, align);
}

void CMemMng::Free(void *block)
{
	CMemMng_Free(block);
}

static nomad::base_function init([]()
{
	char *loc = nio::pattern("48 89 5C 24 ? 57 48 83 EC 20 BB ? ? ? ? 39 DA").first();
	CMemMng_Alloc = (decltype(CMemMng_Alloc))loc;

	loc = nio::pattern("48 85 C9 74 1B 48 3B 0D").first();
	CMemMng_Free = (decltype(CMemMng_Free))loc;
});