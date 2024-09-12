-- SDL2 as a usage
usage "XT"

	
	includedirs {
		"./Include"
	}
	libdirs { 
		"./lib",
	}

	filter "platforms:x64"
		links {
			"XT3100LibDynStatic64",
		}
		
	filter "platforms:x86"
		links {
			"XT3100LibDynStatic",
		}
	