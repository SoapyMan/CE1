project "md5"
	kind "StaticLib"
	language "C"
	includedirs {
		"./"
	}

	files {
		"./**.h",
		"./**.c"
	}

usage "md5"	
	includedirs {
		"./"
	}
	links "md5"