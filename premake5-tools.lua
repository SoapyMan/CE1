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
		"./SourceCode/CryCommon",
		"./SourceCode/CryAnimation"
	}
	defines {
		"RESOURCECOMPILER_EXPORTS"
	}
	links {
		"CryAnimation"
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
		"./SourceCode/CryAnimation",
		"./SourceCode/ResourceCompiler"
	}
	defines {
		"RESOURCECOMPILERPC_EXPORTS"
	}
	links {
		"CryAnimation"
	}

    filter "system:Windows"
		links {
			"dbghelp"
		}
		
project "CgfDump"
    kind "ConsoleApp"
	
	--unitybuild "on"
    uses  "CryCommon"
    files {
		"./SourceCode/CgfDump/**",
	}
    includedirs {
		"./SourceCode/CryCommon",
		"./SourceCode/CryAnimation"
	}
	defines {
		"DONT_USE_FILE_MAPPING"
	}
	links {
		"CryAnimation"
	}

project "CcgDump"
    kind "ConsoleApp"

	--unitybuild "on"
    uses  "CryCommon"
    files {
		"./SourceCode/CcgDump/**",
	}
    includedirs {
		"./SourceCode/CryCommon",
		"./SourceCode/CryAnimation"
	}
	defines {
		"DONT_USE_FILE_MAPPING"
	}
	links {
		"CryAnimation"
	}
		
if ENABLE_EDITOR then

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
	
	flags {
		"NoManifest"
	}
end