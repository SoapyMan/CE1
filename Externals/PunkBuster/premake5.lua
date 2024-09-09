project "punkbuster"
	kind "StaticLib"
	language "C"
	includedirs {
		"./"
	}

	files {
		"./**.h",
		"./**.cpp"
	}

usage "punkbuster"	
	includedirs {
		"./"
	}
	links "punkbuster"