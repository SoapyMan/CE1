project "expat"
	kind "StaticLib"
	language "C"
	includedirs {
		"./"
	}

	files {
		"./**.h",
		"./**.c"
	}
	removefiles {
		"./xmltok_impl.*",
		"./xmltok_ns.*"
	}

usage "expat"	
	includedirs {
		"./"
	}
	links "expat"