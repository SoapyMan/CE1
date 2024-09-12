// CryInput.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"

#ifndef _XBOX 
_ACCESS_POOL;
#endif //_XBOX

#include <stdio.h>
#include <ILog.h>
#include <IInput.h>

#include <SDL_syswm.h>
#include "InputSDL.h"

#ifndef LINUX
#include "InputDirectInput.h"
#endif // LINUX

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#ifdef _WIN32
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif //WIN32
#endif

//////////////////////////////////////////////////////////////////////////
// Pointer to Global ISystem.
static ISystem* gISystem = 0;
ISystem* GetISystem()
{
	return gISystem;
}
//////////////////////////////////////////////////////////////////////////

#ifndef _XBOX
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}
#endif //_XBOX

IInput* CreateInput(ISystem* pSystem, void* hinst, void* hwnd, bool usedinput)
{
	gISystem = pSystem;
	
	IInput* pInput = nullptr;

#ifdef _WIN32
	if (usedinput)
		pInput = new CInputDirectInput();
	else
#endif
		pInput = new CInputSDL();

	void* realhwnd = hwnd;

#ifdef _WIN32
	if(!IsWindow((HWND)hwnd))
#endif
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo((SDL_Window*)hwnd, &wmInfo);
		realhwnd = wmInfo.info.win.window;
	}

	if (!pInput->Init(pSystem, hinst, realhwnd))
	{
		delete pInput;
		return nullptr;
	}
	return pInput;
}

#include <CrtDebugStats.h>
