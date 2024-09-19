usage "cg"
	includedirs {
		"./include"
	}
	filter "platforms:x86"
		libdirs {
			"./lib"
		}
	filter "platforms:x64"
		libdirs {
			"./lib.x64"
		}