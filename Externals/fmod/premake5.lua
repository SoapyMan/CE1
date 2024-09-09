usage "fmod"
	libdirs {
		"./lib"
	}
	includedirs {
		"./include"
	}

	filter "platforms:x86"
		links {
			"fmodvc",
		}
	filter "platforms:x64"
		links {
			"fmod64vc",
		}
	
