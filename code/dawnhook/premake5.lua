project "DawnHook"
    language "C++"
    kind "SharedLib"
	targetextension ".dll"
    
    vpaths
    {
 
        ["*"] = "premake5.lua"
    }

	includedirs
	{
		"../shared",
		"./include",
		--"../vendor/lua",
		"../vendor/minhook",
		"../vendor/imgui",
		"../vendor/udis86",
		"."
	}

	links
	{
		--"lua",
		"minhook",
		"imgui",
		"udis86",
		"Shared"
	}

    files
    {
        "premake5.lua",
        "**.cpp",
        "**.hpp",        
        "**.h",      
        "**.c"
    }
