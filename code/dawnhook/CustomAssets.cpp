
/*
 * Copyright (C) Force67
 * Licensing information can be found in LICENSE.md
 * This file is part of DawnHook
 * Author: Force67
 * Started: 2019-03-07
 */

#include <locale>
#include <cstdio>
#include <filesystem>
#include <utility>
#include <ShlObj.h>

#include <MinHook.h>
#include <Hooking.h>
#include <Nomad/nomad_base_function.h>
#include <Utility/PathUtils.h>

#include <Memory.h>

enum class SeekPosition : int32_t
{
	Cur = 0,
	End = 1,
	Start = 2
};

enum class FileOpenFlags : int32_t
{
	Unknown = 1,
	Unknown1 = 0x10,
	Unknown2 = 0x8001,
};

class IFile
{
public:

	virtual ~IFile() = default;

	virtual char** GetHirachyInfo() = 0;
	virtual uint64_t Read(uint8_t* pOut, uint32_t uiSize) = 0; //v
	virtual int32_t Write(uint8_t*, uint32_t uiSize) = 0;
	virtual uint64_t Seek(int64_t iDelta, SeekPosition) = 0;
	virtual uint64_t GetSize() = 0;
	virtual bool m6() = 0;

	// m7 is a read function used for FAT
	virtual int32_t m7(int64_t, int64_t, uint32_t) = 0;

	virtual uint64_t m8(int64_t, int64_t) = 0;
	virtual int32_t m9(int64_t, int32_t) = 0;
	virtual int32_t m10() = 0;
};

#define SUB_FROM printf(__FUNCTION__ "from %p\n", _ReturnAddress())
//#define SUB_FROM char buf[256]; sprintf(buf, __FUNCTION__ " from %p\n", _ReturnAddress()); MessageBoxA(0, buf, 0,0);

class FileDumper : public IFile
{
	IFile *base;
	FILE *handle;
	 
	void OpenFile(std::string &from)
	{
		auto base = Utility::MakeAbsolutePathA("dawnhook\\dump\\" + from);

		// ensure that dir is present
		{
			auto dir = base.substr(0, base.find_last_of('\\'));

			if (GetFileAttributesA(dir.c_str()) == INVALID_FILE_ATTRIBUTES)
				SHCreateDirectoryExA(nullptr, dir.c_str(), nullptr);

			std::printf("Dumping %s to %s\n", from.c_str(), base.c_str());
		}

		fopen_s(&handle, base.c_str(), "w");

		if (!handle)
			std::printf("Failed to open dump device!");
	}

public:

	FileDumper(IFile *base, const char *path) :
		base(base),
		handle(nullptr)
	{
		std::string str = std::string(path);

		OpenFile(str);
	}

	virtual ~FileDumper()
	{
		if (handle)
			fclose(handle);
	};

	char** GetHirachyInfo() override
	{
		return base->GetHirachyInfo();
	}

	uint64_t Read(uint8_t* pOut, uint32_t uiSize) override
	{
		SUB_FROM;

		auto result = base->Read(pOut, uiSize);

		if (handle)
		{
			fwrite(pOut, uiSize, 1, handle);
			fflush(handle);	
		}

		return result;
	}

	int32_t Write(uint8_t* pContent, uint32_t uiSize) override
	{
		SUB_FROM;
		return base->Write(pContent, uiSize);
	}

	uint64_t Seek(int64_t iDelta, SeekPosition pos) override
	{
		SUB_FROM;
		return base->Seek(iDelta, pos);
	}

	uint64_t GetSize() override
	{
		SUB_FROM; 
		return base->GetSize();
	}

	bool m6() override
	{
		SUB_FROM;
		return base->m6();
	}

	int32_t m7(int64_t a1, int64_t a2, uint32_t a3) override
	{
		SUB_FROM;
		return base->m7(a1, a2, a3);
	}

	uint64_t m8(int64_t a1, int64_t a2) override
	{
		SUB_FROM;
		return base->m8(a1, a2);
	}

	int32_t m9(int64_t a1, int32_t a2) override
	{
		SUB_FROM;
		return base->m9(a1, a2);
	}

	int32_t m10() override
	{
		SUB_FROM;
		return base->m10();
	}
};

// hack 
class CustomFile;

static std::unordered_map<uint64_t, std::string> g_overrides;

//static std::unordered_map<uint64_t, CustomFile*> g_overrides;

class CustomFile : public IFile
{
	IFile *base;
	FILE *handle;

	//std::string path;

public:

	CustomFile(IFile *base, const std::string &path) :
		base(base),
		handle(nullptr)
	{
		fopen_s(&handle, path.c_str(), "r");

		// we end up using the basedevice
		// if we cannot open the handle
		if (!handle)
			std::printf("CustomFile: Failed to open %s\n", path.c_str());
		else
			std::printf("Opened customFile for %s\n", path.c_str());
	}

	/*explicit CustomFile(std::string path) :
		base(nullptr), 
		handle(nullptr),
		path(std::move(path))
	{
	}*/

	virtual ~CustomFile()
	{
		if (handle)
			fclose(handle);
	}

	/*void Open(IFile *base)
	{
		this->base = base;

		fopen_s(&handle, path.c_str(), "r");

		// we end up using the basedevice
		// if we cannot open the handle
		if (!handle)
			std::printf("CustomFile: Failed to open %s\n", path.c_str());
		else
			std::printf("Opened customFile for %s\n", path.c_str());
	}*/

	char** GetHirachyInfo() override
	{
		return base->GetHirachyInfo();
	}

	uint64_t Read(uint8_t* pOut, uint32_t uiSize) override
	{
		//SUB_FROM;

		if (handle)
		{
			fread(pOut, uiSize, 1, handle);
			fflush(handle);

			return uiSize;
		}

		return base->Read(pOut, uiSize);
	}

	int32_t Write(uint8_t* pContent, uint32_t uiSize) override
	{
		return base->Write(pContent, uiSize);
	}

	uint64_t Seek(int64_t iDelta, SeekPosition pos) override
	{
		if (handle)
		{
			if (pos == SeekPosition::Cur)
				fseek(handle, iDelta, SEEK_CUR);

			if (pos == SeekPosition::End)
				fseek(handle, iDelta, SEEK_END);

			if (pos == SeekPosition::Start)
				fseek(handle, iDelta, SEEK_SET);

			return ftell(handle);
		}

		return base->Seek(iDelta, pos);
	}

	uint64_t GetSize() override
	{
		return base->GetSize();
	}

	bool m6() override
	{
		return base->m6();
	}

	// TODO: handle this for .FAT SUPPORT
	int32_t m7(int64_t a1, int64_t a2, uint32_t a3) override
	{
		return base->m7(a1, a2, a3);
	}

	uint64_t m8(int64_t a1, int64_t a2) override
	{
		return base->m8(a1, a2);
	}

	int32_t m9(int64_t a1, int32_t a2) override
	{
		return base->m9(a1, a2);
	}

	int32_t m10() override
	{
		return base->m10();
	}
};

static uint64_t(*CreateCRC64NoCase)(const char*);
static uint32_t(*CreateCRC32NoCase)(const char*);

static void FindOverrides()
{
	auto path = Utility::MakeAbsolutePathA("dawnhook\\mods");

	for (auto& entry : std::filesystem::recursive_directory_iterator(path))
	{
		if (!std::filesystem::is_regular_file(entry))
			continue;

		// hash the relative path
		auto vfs_dir = entry.path().string().substr(path.length() + 1);
		auto hash = CreateCRC64NoCase(vfs_dir.c_str());

		std::printf("Created hash %llx for %s\n", hash, vfs_dir.c_str());

		// add it to overrides

		// even more hacky
	//	CustomFile *f = new CustomFile(entry.path().string());

		g_overrides.emplace(hash, entry.path().string());
	}
}

static FILE* g_DumpFile{nullptr};

static IFile*(*CFileManager__FileOpen)(__int64, const char*, FileOpenFlags, char);
static IFile*(*OpenFileByHash)(__int64 self, const uint64_t &hash, bool a3, bool a4);
static int(*OpenDeviceForWriting)(__int64, const char *, __int64);;

static IFile *CFileManager__FileOpen_Stub(__int64 self, const char *name, FileOpenFlags flags, bool unknown)
{
	IFile *file = CFileManager__FileOpen(self, name, flags, unknown);

#if 1
	// determine the hash
	uint64_t hash = CreateCRC64NoCase(name);

	// is the file overridden ?
	auto it = g_overrides.find(hash);
	if (it != g_overrides.end())
	{
	/*	CustomFile *custom = it->second;

		custom->Open(file);

		return custom;*/

		return new CustomFile(file, it->second);

	/*	void *mem = CMemMng::Alloc(sizeof(CustomFile), 0);

		return new (mem) CustomFile(file, it->second);*/
	}
#endif

/*	if (strstr(name, ".xml"))
	{
		return new FileDumper(file, name);
	}

	std::printf("CFileManager::FileOpen %s (%d)\n", name, flags);
	*/


	return file;
}

static IFile *OpenFileByHash_Stub(__int64 self, const uint64_t &hash, bool a3, bool a4)
{
	std::printf("OpenFileByHash %llx\n", hash);

	return OpenFileByHash(self, hash, a3, a4);
}

void * (*xj_CNomadArchive__CreateIFileFor)(__int64 a1, const uint64_t &hash, __int64 a3, __int64 a4);
void * j_CNomadArchive__CreateIFileFor(__int64 a1, const uint64_t &hash, __int64 a3, __int64 a4)
{
	std::printf("CNomadArchive::CreateIFileFor %llx\n", hash);
	return xj_CNomadArchive__CreateIFileFor(a1, hash, a3, a4);
}

#if 0
static nomad::base_function init([]()
{
	char *loc = nio::pattern("0F BA E7 11 73 70").first();
	nio::set_call(&CreateCRC64NoCase, loc + 0x3F);

	MH_CreateHook(loc - 0xCE, &CFileManager__FileOpen_Stub, (LPVOID*)&CFileManager__FileOpen);
	//MH_CreateHook((LPVOID)0x186550220, &OpenFileByHash_Stub, (LPVOID*)&OpenFileByHash);
	//`//MH_CreateHook((LPVOID)0x186551040, &j_CNomadArchive__CreateIFileFor, (LPVOID*)&xj_CNomadArchive__CreateIFileFor);

	MH_EnableHook(nullptr);
	FindOverrides();
});
#endif

static int OpenDeviceForWriteing_Stub(__int64 a1, const char *str, __int64 a3)
{
	std::printf("openforwrite %s\n", str);

	return OpenDeviceForWriting(a1, str, a3);
}

// for debugging purposes
static uint64_t CreateCRC64NoCase_Stub(const char *str)
{
	uint64_t hash = CreateCRC64NoCase(str);

	if (g_DumpFile)
	{
		fprintf(g_DumpFile, "%p |%llx          |%s\n", _ReturnAddress(), hash, str);
		fflush(g_DumpFile);
	}

	return hash;
}

static uint64_t CreateCRC32NoCase_Stub(const char *str)
{
	uint64_t hash = CreateCRC32NoCase(str);

	if (g_DumpFile)
	{
		fprintf(g_DumpFile, "%p |%llx          |%s\n", _ReturnAddress(), hash, str);
		fflush(g_DumpFile);
	}

	return hash;
}

#if 0
static nomad::base_function init_debug_stuff([]()
{
	//MH_CreateHook((LPVOID)0x1952F7540, &OpenDeviceForWriteing_Stub, (LPVOID*)&OpenDeviceForWriting);

	/*{
		MH_CreateHook((LPVOID)0x186544670, &CreateCRC64NoCase_Stub, (LPVOID*)&CreateCRC64NoCase);
		_wfopen_s(&g_DumpFile, Utility::MakeAbsolutePathW(L"dawnhook\\dump\\crc64dump.txt").c_str(), L"w");
		fputs("Address |   CRC64  |  Name", g_DumpFile);
	}*/

	{
		MH_CreateHook((LPVOID)0x0186544BE0, &CreateCRC32NoCase_Stub, (LPVOID*)&CreateCRC32NoCase);
		_wfopen_s(&g_DumpFile, Utility::MakeAbsolutePathW(L"dawnhook\\dump\\crc32dump.txt").c_str(), L"w");
		fputs("Address |   CRC32  |  Name", g_DumpFile);
	}
});
#endif