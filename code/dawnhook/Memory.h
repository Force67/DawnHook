#pragma once

/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

class CMemMng
{
public:

	static void *Alloc(size_t, size_t);
	static void Free(void*);

	template <typename Type, typename... TArgs>
	static Type* Alloc(TArgs... args)
	{
		return new (Alloc(sizeof(Type), 0)) Type(args ...);
	}
};
