# CryEngine 1 and Far Cry

## Key changes

* Using Premake5 for project files generation
* Converted code to build with Visual Studio 2022 and GCC
* Enabled C++17, removed legacy stuff
* Multiple buffer overflows found and fixed with ASAN
* SDL2 for handling window and input
* Code file structure cleanup

### Compatibilty changes

* CrySystem  - HTTPDownloader.cpp: commented 309-310 lines
* CryNetwork - Disabled ubisoft nerwork code

### Future work TODO

* Build editor and convert to Qt or ImGui
* Fix original bugs
* Optimize game loading times
* Replace FMOD 3.74 with OpenAL or Steam Audio
* Replace nvTriStrip with meshoptimizer
* Replace nvDXT with nvtt
* Replace/remove nvidia cg
* Rewrite renderers - to not rely on old D3D9 and OpenGL
* Linux port

### Legal notes

This project is intended for educational and non-commercial purposes only.

Based on source code published 24 june 2023.
https://archive.org/details/far-cry-1.34-complete
