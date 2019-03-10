project "udis86"
	targetname "udis86"
		language "C"
		kind "StaticLib"
		
		-- dont question this output path!
		targetdir "../bin/vendor/%{cfg.buildcfg}"
	
		includedirs { "." }
	
		-- dont question those paths.
		files
		{
			"**.c",
			"**.h"
		}