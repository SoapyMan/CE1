group "Tools"

project "ResourceCompiler"
    kind "ConsoleApp"
    targetname "rc"

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
		
if false then -- os.target() == "windows" then

project "Editor"
	kind "WindowedApp"

	--unitybuild "on"
	uses  {
		"CryCommon",
		"XT",
		"zlib",
		"expat"
	}
	files {
		"./SourceCode/Editor/**",
	}
	removefiles {
		"./SourceCode/Editor/Controls/NewMenu*",
		"./SourceCode/Editor/Building*",
		"./SourceCode/Editor/StatObjPanel*",
		"./SourceCode/Editor/Objects/Building*",
		"./SourceCode/Editor/Objects/StatObj*",
		"./SourceCode/Editor/Brush/BrushIndoor*",
	}

	includedirs {
		"./SourceCode/CryCommon",
		"./SourceCode/Editor",
		"./SourceCode/Editor/Include",
	}
end