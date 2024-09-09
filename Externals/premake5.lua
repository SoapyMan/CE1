include(DependencyPath.zlib.."/premake5.lua")
include(DependencyPath.libjpeg.."/premake5.lua")
--include(DependencyPath.openal.."/premake5.lua")
include(DependencyPath.libsdl.."/premake5.lua")

if ENABLE_TESTS then
include("gtest/premake5.lua")
end

--include("BinkSDK/premake5.lua")
include("ffmpeg/premake5.lua")
include("FreeType2/premake5.lua")
include("PunkBuster/premake5.lua")

include("fmod/premake5.lua")
include("cg/premake5.lua")
include("lua41/premake5.lua")
include("cv_sdk/premake5.lua")
include("minivorbis/premake5.lua")
include("md5/premake5.lua")
include("Expat/premake5.lua")
--include("meshoptimizer/premake5.lua")
include("NvTriStrip/premake5.lua")

