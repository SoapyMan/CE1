-- premake5.lua

require ".premake_modules/usage"
--require ".premake_modules/unitybuild"

local CAN_BUILD_TOOLS = (os.target() == "linux" or os.target() == "windows")
local CAN_BUILD_GUI_TOOLS = (--[[os.target() == "linux" or]] os.target() == "windows")

ENABLE_TOOLS = iif(ENABLE_TOOLS == nil, CAN_BUILD_TOOLS, ENABLE_TOOLS)
ENABLE_GUI_TOOLS = iif(ENABLE_GUI_TOOLS == nil, CAN_BUILD_GUI_TOOLS, ENABLE_GUI_TOOLS)
ENABLE_TESTS = iif(ENABLE_TESTS == nil, false, ENABLE_TESTS)
ENABLE_EDITOR = os.target() == "windows" 
WORKSPACE_NAME = (WORKSPACE_NAME or "CryEngine")

-- you can redefine dependencies
DependencyPath = {
	["zlib"] = os.getenv("ZLIB_DIR") or "Externals/zlib", 
	["libjpeg"] = os.getenv("JPEG_DIR") or "Externals/libjpeg",
	["libsdl"] = os.getenv("SDL2_DIR") or "Externals/SDL2",
	["openal"] = os.getenv("OPENAL_DIR") or "Externals/openal-soft",
	["fmod"] = os.getenv("FMOD_DIR") or "Externals/fmod",
}

-- default configuration capabilities
Groups = {
    core = "CryEngine",
    tools = "Tools",
    game = "FarCryGame",
}

-- folders for framework, libraries and tools
Folders = {

}

function prj_name(prj, def)
	if prj ~= nil then
		if prj.group ~= nil and string.len(prj.group) > 0 then
			return prj.group .. '/' .. prj.name
		end
		return prj.name
	end
	return def
end

-- Main workspace
workspace(WORKSPACE_NAME)
    language "C++"
	cppdialect "C++17"	-- required for sol2
    configurations {
		"Debug", 
		"Release",
		"ReleaseAsan",
		"Retail"
	}
	linkgroups 'On'
	
	--characterset "ASCII"
	objdir "build/obj"
	targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"

	if _ACTION ~= "vscode" then
		location "build/%{ prj_name(prj) }"
	end

	defines {
		"PROJECT_COMPILE_CONFIGURATION=%{cfg.buildcfg}",
		"PROJECT_COMPILE_PLATFORM=%{cfg.platform}"
	}
	
	-- for MSVC to be faster
	flags {
		"MultiProcessorCompile"
	}

    filter "system:linux"
		platforms { 
			"x86", "x64" -- maybe add ARM & ARM64 for RPi?
		}
		-- vscode_makefile "build/%{wks.name}"
		-- vscode_launch_cwd "${workspaceRoot}/../%{wks.name}/build/bin64linux"
		-- vscode_launch_environment {
		-- 	LD_LIBRARY_PATH = "${LD_LIBRARY_PATH}:${workspaceRoot}%{cfg.targetdir}:${workspaceRoot}/../%{wks.name}/build/bin64linux"
		-- }
		-- vscode_launch_visualizerFile "${workspaceRoot}/public/types.natvis"
        buildoptions {
            "-fpermissive",
			"-fexceptions",
			"-fpic",
        }
		disablewarnings {
			-- disable warnings which are emitted by my stupid (and not so) code
			"narrowing",
			"c++11-narrowing",
			"writable-strings",
			"logical-op-parentheses",
			"parentheses",
			"register",
			"unused-local-typedef",
			"nonportable-include-path",
			"format-security",
			"unused-parameter",
			"sign-compare",
			"ignored-attributes",	-- annyoing, don't re-enable
			"write-strings",		-- TODO: fix this
			"subobject-linkage"		-- TODO: fix this
		}
		links { "pthread" }

	filter "system:Windows"
		platforms { 
			"x86", 
			"x64"
		}
		characterset ("ASCII")
		disablewarnings { "4996", "4302", "4477", "4554", "4244", "4101", "4838", "4309" }
		enablewarnings { "26433" }
		defines { 
			"NOMINMAX", 
			"_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE"
		}

    filter "configurations:Debug"
        defines { 
            "_DEBUG"
        }
        symbols "On"

    filter "configurations:Release"
        defines {
            "NDEBUG",
        }
		optimize "On"
		symbols "On"
		
	filter "configurations:ReleaseAsan"
        defines {
            "NDEBUG",
        }
		optimize "On"
		symbols "On"
		sanitize "address"
		
	filter "configurations:Retail"
        defines {
			"NDEBUG",
			"_RETAIL"
        }
		optimize "On"

	filter "system:Linux"
		defines {
			"LINUX"
		}

	filter "system:Windows or system:Linux"
		defines { 
			"WIN32=1",
			"USE_SDL"
		}
		
	filter {"platforms:x64", "system:Windows"}
		defines { 
			"WIN64=1",
			"_AMD64_"
		}
		
	filter { "platforms:x64", "system:Linux" }
		defines { 
			"LINUX64",
			"_AMD64_"
		}

group "CryEngine"
	
----------------------------------------------

project "CryCommon"
    kind "StaticLib"
	uses {
		"SDL2",
	}
    files {
		"./SourceCode/CryCommon/**.cpp",
        "./SourceCode/CryCommon/**.h",
	}
	
	
usage "CryCommon"
	includedirs {
		"./SourceCode/CryCommon/"
	}
	links {
		"CryCommon"
	}

project "CrySystem"
    kind "SharedLib"
	--unitybuild "on"
	uses {
		"CryCommon",
		"SDL2",
		"zlib",
		"md5",
		"expat"
	}
	
    files {
		"./SourceCode/CrySystem/**.cpp",
		"./SourceCode/CrySystem/**.h"
	}
	defines {
		"CRYSYSTEM_EXPORTS"
	}
	filter "system:Windows"
		files {
			"./SourceCode/CrySystem/LuaDebugger/**.cpp",
			"./SourceCode/CrySystem/LuaDebugger/**.h"
		}
		links {
			"wininet"
		}
		
project "Cry3DEngine"
    kind "SharedLib"
	--unitybuild "on"
	uses "CryCommon"
	
    files {
        "./SourceCode/Cry3DEngine/*.cpp",
		"./SourceCode/Cry3DEngine/*.c",
        "./SourceCode/Cry3DEngine/*.h"
	}
	
	defines {
		"CRY3DENGINE_EXPORTS"
	}
	
project "CryAISystem"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
        "./SourceCode/CryAISystem/*.cpp",
		"./SourceCode/CryAISystem/*.c",
        "./SourceCode/CryAISystem/*.h"
	}
	
	defines {
		"CRYAISYSTEM_EXPORTS"
	}

project "CryAnimation"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
		"./SourceCode/CryAnimation/*.asm",
        "./SourceCode/CryAnimation/*.cpp",
		"./SourceCode/CryAnimation/*.c",
        "./SourceCode/CryAnimation/*.h"
	}
	
	defines { 
		"CRYANIMATION_EXPORTS",
		-- "DO_ASM",
		-- "PROC_INTEL",
	}

project "CryEntitySystem"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
        "./SourceCode/CryEntitySystem/*.cpp",
		"./SourceCode/CryEntitySystem/*.c",
        "./SourceCode/CryEntitySystem/*.h"
	}
	
	defines {
		"CRYENTITYDLL_EXPORTS"
	}
	
project "CryFont"
    kind "SharedLib"
	uses {
		"CryCommon",
		"freetype2"
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CryFont/*.cpp",
		"./SourceCode/CryFont/*.c",
        "./SourceCode/CryFont/*.h"
	}
	
	defines {
		"CRYFONT_EXPORTS"
	}
	
project "CryGame"
    kind "SharedLib"
	uses {
		"CryCommon",
		"ffmpeg",
		"SDL2",
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CryGame/*.cpp",
		"./SourceCode/CryGame/*.c",
        "./SourceCode/CryGame/*.h"
	}
	
	defines {
		"CRYGAME_EXPORTS",
		"NOT_USE_DIVX_SDK"
	}

	filter "system:Windows"
		links {
			"Wsock32"
		}
		
project "CryInput"
    kind "SharedLib"
	uses {
		"CryCommon",
		"SDL2"
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CryInput/*.cpp",
		"./SourceCode/CryInput/*.c",
        "./SourceCode/CryInput/*.h"
	}
	
	removefiles {
		"./SourceCode/CryInput/XGamepad*",
	}
	
	defines {
		"CRYINPUT_EXPORTS",
	}
	
	filter "system:linux"
		removefiles {
			"./SourceCode/CryInput/InputDirectInput*",
			"./SourceCode/CryInput/XKeyboard*",
			"./SourceCode/CryInput/XMouse*",
		}
	
project "CryAISystem"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
        "./SourceCode/CryAISystem/*.cpp",
		"./SourceCode/CryAISystem/*.c",
        "./SourceCode/CryAISystem/*.h"
	}
	
	defines {
		"CRYAISYSTEM_EXPORTS"
	}

project "CryMovie"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
        "./SourceCode/CryMovie/*.cpp",
		"./SourceCode/CryMovie/*.c",
        "./SourceCode/CryMovie/*.h"
	}
	
	defines {
		"CRYMOVIE_EXPORTS"
	}
	
project "CryNetwork"
	kind "SharedLib"
	uses {
		"CryCommon",
		"punkbuster"
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CryNetwork/*.cpp",
		"./SourceCode/CryNetwork/*.c",
        "./SourceCode/CryNetwork/*.h"
	}
	
	defines {
		"CRYNETWORK_EXPORTS"
	}

project "CryPhysics"
    kind "SharedLib"
	uses "CryCommon"
	
	--unitybuild "on"
    files {
        "./SourceCode/CryPhysics/*.cpp",
		"./SourceCode/CryPhysics/*.c",
        "./SourceCode/CryPhysics/*.h"
	}
	
	defines {
		"PHYSICS_EXPORTS",
		"ENTITY_PROFILER_ENABLED"
	}
	
	filter "platforms:x86"
		defines {
			"PIII_SSE"
		}
		
project "CryScriptSystem"
    kind "SharedLib"
	uses {
		"CryCommon",
		"lua",
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CryScriptSystem/*.cpp",
		"./SourceCode/CryScriptSystem/*.c",
        "./SourceCode/CryScriptSystem/*.h"
	}
	
	defines {
		"CRYSCRIPTSYSTEM_EXPORTS"
	}
	
project "CrySoundSystem"
    kind "SharedLib"
	uses {
		"CryCommon",
		"fmod",
		-- "openal-soft" TODO
		"minivorbis",
	}
	
	--unitybuild "on"
    files {
        "./SourceCode/CrySoundSystem/*.cpp",
		"./SourceCode/CrySoundSystem/*.c",
        "./SourceCode/CrySoundSystem/*.h"
	}
	
	defines {
		"CRYSOUNDSYSTEM_EXPORTS"
	}
	
	filter "system:Windows"
		links {
			"legacy_stdio_definitions"
		}
		
-- RenderDLL

usage "RenderDll"
    files {
		"./SourceCode/RenderDll/Common/**.cpp",
		"./SourceCode/RenderDll/Common/**.h",
		"./SourceCode/RenderDll/*.cpp",
        "./SourceCode/RenderDll/*.h",
	}
	removefiles {
		"./SourceCode/RenderDll/Common/Textures/Image/Xtf*",
	}
	includedirs {
		"./SourceCode/RenderDll/"
	}
	defines {
		"_RENDERER",
		"USE_3DC",
		"ENABLE_FRAME_PROFILER",
		"PROC_INTEL",
		"DO_RENDERLOG",
		"DO_RENDERSTATS",
	}
	filter "system:Windows"
		links {
			"legacy_stdio_definitions",
			-- "nvDXTlib" -- needed by ENABLE_DXT
		}
	filter "platforms:x86"
		defines {
			"DO_ASM"
		}

project "XRenderNULL"
    kind "SharedLib"
	uses {
		"CryCommon",
		"RenderDll",
		"nvtristrip",
		"libjpeg",
		"SDL2"
	}
	files {
		"./SourceCode/RenderDll/XRenderNULL/**.cpp",
        "./SourceCode/RenderDll/XRenderNULL/**.h",
	}
	defines {
		"NULL_RENDERER"
	}
	
project "XRenderOGL"
    kind "SharedLib"
	uses {
		"CryCommon",
		"RenderDll",
		"nvtristrip",
		"libjpeg",
		"SDL2",
		"cg"
	}
	files {
		"./SourceCode/RenderDll/XRenderOGL/**.cpp",
        "./SourceCode/RenderDll/XRenderOGL/**.h",
	}

	links {
		"glu32", "opengl32",
		"cg", "cgGL"
	}
	
	defines {
		"XRENDEROGL_EXPORTS",
		"OPENGL",
		"USE_CG",
	}

if os.target() == "windows" then

project "XRenderD3D9"
    kind "SharedLib"
	uses {
		"CryCommon",
		"RenderDll",
		"nvtristrip",
		"libjpeg",
		"SDL2",
		"cg"
	}
	files {
		"./SourceCode/RenderDll/XRenderD3D9/**.cpp",
        "./SourceCode/RenderDll/XRenderD3D9/**.h",
	}
	includedirs {
		"./SourceCode/RenderDll/Dx9Sdk/Include"
	}
	links {
		"d3d9", "d3dx9", "DXErr",
		"cg", "cgD3D9",
		"winmm", "version"
	}
	defines {
		"XRENDERD3D9_EXPORTS",
		"DIRECT3D9",
		"USE_CG",
	}
	filter "platforms:x86"
		libdirs {
			"./SourceCode/RenderDll/Dx9Sdk/Lib"
		}
	filter "platforms:x64"
		libdirs {
			"./SourceCode/RenderDll/Dx9Sdk/Lib64"
		}
		
end -- windows

------------------------------------------

group "Externals"
		
-- dependencies are in separate configuration
include "Externals/premake5.lua"

group ""

-- only build tools for big machines
if ENABLE_TOOLS then
	include "premake5-tools.lua"
end
