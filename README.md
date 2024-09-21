# CE1 (CryEngine 1)

Project aims to bring CryEngine 1 to modern systems, improve overall stability, add QoL changes while maintaining compatibility with Far Cry.

## Key changes

* Using Premake5 for project files generation
* Converted code to build with Visual Studio 2022 and GCC
* Enabled C++17, removed legacy stuff
* Sandbox Editor runs on latest MFC
* Multiple buffer overflows found and fixed with ASAN
* SDL2 for handling window and input
* Code file structure cleanup
* crysound replaced with FMOD, since it's the same thing
* Bink & DivX replaced with FFMpeg
* Updated Cg to 3.1

### Compatibilty changes

* CrySystem  - HTTPDownloader.cpp: commented 309-310 lines
* CryNetwork - Disabled ubisoft nerwork code

### Future work TODO

* Fix original bugs
* Bring multi-threading
* Optimize loading times
* Merge some bits of code from more recent CryEngine versions
* Support for importing FBX to use as static and skinned objects
* Replace FMOD 3.74 with OpenAL
* Replace nvTriStrip with meshoptimizer
* Replace nvDXT with nvtt
* Rewrite renderers and get rid of obsolete hardware support
* Remove x86 assembly code
* Remove custom data structures like TArray, list2, etc and replace with eastl or e2ds (Eq2 Engine Data Structures)
* Cross platform compiling and running

### Legal notes

This project is intended for educational and non-commercial purposes only.

Based on source code published 24 june 2023.
https://archive.org/details/far-cry-1.34-complete
