project "freetype2"
	kind "StaticLib"
	language "C"
	includedirs {
		"./include"
	}

	files {
		"./src/autohint/autohint.c",
		"./src/base/ftbase.c",
		"./src/base/ftbbox.c",
		"./src/base/ftbdf.c",
		"./src/base/ftdebug.c",
		"./src/base/ftglyph.c",
		"./src/base/ftinit.c",
		"./src/base/ftmm.c",
		"./src/base/ftpfr.c",
		"./src/base/ftsystem.c",
		"./src/base/ftwinfnt.c",
		"./src/cache/ftcache.c",
		"./src/psaux/psaux.c",
		"./src/pshinter/pshinter.c",
		"./src/psnames/psnames.c",
		"./src/raster/raster.c",
		"./src/sfnt/sfnt.c",
		"./src/smooth/smooth.c",
		"./src/truetype/truetype.c",
		"./src/winfonts/winfnt.c"
	}

usage "freetype2"	
	includedirs {
		"./include"
	}
	links "freetype2"