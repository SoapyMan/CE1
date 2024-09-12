project "XT"
	kind "StaticLib"
	files {
		"./XTToolkit_Lib/StdAfx.cpp",
		"./XTToolkit_Lib/StdAfx.h",
		"./Src/**.cpp",
		"./Include/**.h"
	}
	includedirs {
		"./Src/",
		"./Src/Common.Src",
		"./XTToolkit_Lib/",
		"./Include",
		"./Include/Common.Include",
	}
	defines {
		"_AFXDLL",
		"_AFX_NO_OCC_SUPPORT",
		"_XT_STATICLINK",
		"_XTLIB_NOAUTOLINK"
	}
	
usage "XT"
	includedirs {
		"./Include"
	}
	defines {
		"_AFXDLL",
		"_XT_STATICLINK",
		"_XTLIB_NOAUTOLINK"
	}
	links "XT"