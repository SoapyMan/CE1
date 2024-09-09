group "Tools"

project "ResourceCompiler"
    kind "ConsoleApp"

	--unitybuild "on"
    uses  "CryCommon"
    files {
		"./SourceCode/ResourceCompiler/**",
	}

    includedirs {
		"./SourceCode/CryCommon"
	}
	
	defines {
		"RESOURCECOMPILER_EXPORTS"
	}

    filter "system:Windows"
		links {
			"dbghelp"
		}

project "ResourceCompilerPC"
    kind "SharedLib"

	--unitybuild "on"
    uses {
		"CryCommon", 
		"nvtristrip"
	}
    files {
		"./SourceCode/ResourceCompilerPC/**",
	}

    includedirs {
		"./SourceCode/CryCommon",
		"./SourceCode/ResourceCompiler"
	}
	
	defines {
		"RESOURCECOMPILERPC_EXPORTS"
	}

    filter "system:Windows"
		links {
			"dbghelp"
		}
		
	filter "platforms:x64"
		files {
			"./SourceCode/CryAnimation/CrySkinAMD64.asm"
		}