// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_)
#define AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS

// #TODO: !!!
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
//////////////////////////////////////////////////////////////////////////

#include <platform.h>

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#define USE_NEWPOOL
#include <CryMemoryManager.h>

#if defined(LINUX)
#include <unistd.h>
#include <fcntl.h>
static inline int closesocket(int s) { return ::close(s); }
static inline int WSAGetLastError() { return errno; }
#endif

#include <map>
#include <INetwork.h>
#include "DatagramSocket.h"
#include <CrySizer.h>
// TODO: reference additional headers your program requires here


#define NOT_USE_UBICOM_SDK
//#define NOT_USE_PUNKBUSTER_SDK

#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#endif

#if _MSC_VER > 1000
#pragma warning( disable : 4786 )
//#pragma warning( disable : 4716 )
#endif // _MSC_VER > 1000

#ifdef PS2
typedef int	BOOL;
#endif

#ifndef NET____TRACE
#define NET____TRACE

#ifndef PS2
inline void __cdecl __NET_TRACE(const char* sFormat, ...)
{
	/*
	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	CRYASSERT(strlen(sTraceString) < 500)
	CryLogAlways( sTraceString );

	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	CRYASSERT(strlen(sTraceString) < 500)
	::OutputDebugString(sTraceString);*/

}
#else
inline void __NET_TRACE(const char* sFormat, ...)
{
	va_list vl;
	static char sTraceString[500];

	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);
	CRYASSERT(strlen(sTraceString) < 500)
		cout << sTraceString;

}

#endif	//PS2

#if 1

#define NET_TRACE __NET_TRACE

#else

#define NET_TRACE 1?(void)0 : __NET_TRACE;

#endif //NET____TRACE

#endif //_DEBUG

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0F62BDE9_4784_4F7A_A265_B7D1A71883D0__INCLUDED_)
