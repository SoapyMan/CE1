project "lua"
	kind "StaticLib"
	files {
		"./src/**.c"
	}
	includedirs {
		"./src",
	}
	removefiles {
		"./src/lua.c",
	}
	
	defines { "LUA_COMPAT_MODULE" } 

usage "lua"
	includedirs {
		"./src",
	}

	links { "lua" }

