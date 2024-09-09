project "nvtristrip"
	kind "StaticLib"
	language "C"
	includedirs {
		"./"
	}

	files {
		"./**.h",
		"./**.cpp"
	}

usage "nvtristrip"	
	includedirs {
		"./"
	}
	links "nvtristrip"