WORKSPACE_NAME = "FarCry"

dofile "premake5-engine.lua"

group ""

project "Launcher"
    kind "WindowedApp"
	targetname "FarCry"

	--unitybuild "on"
    uses {
		"CryCommon",
		"SDL2"
	}
    files {
		"./SourceCode/FarCry/**",
	}

    filter "system:Windows"
		linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
		}
		
project "DedicatedServer"
    kind "WindowedApp"
	targetname "FarCry_WinSV"

	--unitybuild "on"
    uses {
		"CryCommon",
		"SDL2"
	}
    files {
		"./SourceCode/FarCry_WinSV/**",
	}
    filter "system:Windows"
		linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
		}

