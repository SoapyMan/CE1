////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   platform.h
//  Version:     v1.00
//  Created:     11/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Platform dependend stuff.
//               Include this file instead of windows.h
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ProjectDefines.h"							// to get some defines available in every CryEngine project
#include "Assert.h"
#include <stdio.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !defined(LINUX)
typedef void* THREAD_HANDLE;
typedef void* EVENT_HANDLE;
#endif

#if defined(LINUX)
#include <stdlib.h> 
#include <time.h>
#ifdef __cplusplus
#include <map>
#include <vector>
#endif
#endif

#if defined(_WIN32)
#define RC_EXECUTABLE "rc.exe"
#elif defined(LINUX)
#define RC_EXECUTABLE "rc"
#endif

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(WIN64)
#define _CPU_AMD64
#include "Win64specific.h"
#else
#define _CPU_X86
#include "Win32specific.h"
#endif

#if defined(LINUX)

#elif defined(LINUX64)
#include "Linux64Specific.h"
#define _CPU_AMD64
#elif defined(LINUX32)
#define _CPU_X86
#include "Linux32Specific.h"
#endif

#endif

#define CPUF_SSE   1
#define CPUF_SSE2  2
#define CPUF_3DNOW 4
#define CPUF_MMX   8

#ifndef __linux
#define IS_DIR(X) X.attrib &_A_SUBDIR
#define FNAME(X) X.name
#else
#define IS_DIR(X) X.d_type == DT_DIR
#define FNAME(X) X.d_name
#endif

#define crymax(a,b)	(((a) > (b)) ? (a) : (b))
#define crymin(a,b)	(((a) < (b)) ? (a) : (b))

//////////////////////////////////////////////////////////////////////////
// Use our own memory manager.
// No Any STL includes must be before this line.
//////////////////////////////////////////////////////////////////////////
#ifndef NOT_USE_CRY_MEMORY_MANAGER
#define USE_NEWPOOL
#include "CryMemoryManager.h"
#endif // NOT_USE_CRY_MEMORY_MANAGER

#ifdef __cplusplus
// define the standard string
#include <string>

typedef std::string string;
typedef std::wstring wstring;

#endif // __cplusplus

#ifdef _XBOX
#define _CPU_X86
#include <xtl.h>
#include "XBoxSpecific.h"
#endif


// 32/64 Bit versions.
#define SIGN_MASK(x) ((intptr_t)(x) >> ((sizeof(size_t)*8)-1))

// macro for structure alignement
#ifdef LINUX
#define DEFINE_ALIGNED_DATA( type, name, alignment ) type name __attribute__ ((aligned(alignment)));
#define DEFINE_ALIGNED_DATA_STATIC( type, name, alignment ) static type name __attribute__ ((aligned(alignment)));
#define DEFINE_ALIGNED_DATA_CONST( type, name, alignment ) const type name __attribute__ ((aligned(alignment)));
#else
#define DEFINE_ALIGNED_DATA( type, name, alignment ) _declspec(align(alignment)) type name;
#define DEFINE_ALIGNED_DATA_STATIC( type, name, alignment ) static _declspec(align(alignment)) type name;
#define DEFINE_ALIGNED_DATA_CONST( type, name, alignment ) const _declspec(align(alignment)) type name;
#endif


///////////////////////////////////////////////////////////////////////////////
// common Typedef                                                                   //
///////////////////////////////////////////////////////////////////////////////
typedef double real;
typedef int index_t;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int* PUINT;

#if defined(WIN64) || defined(LINUX)
#ifdef __cplusplus
inline int64 GetTicks()
#else
static int64 GetTicks()
#endif
{
#if defined(WIN64)
	return __rdtsc();
#else
	typedef union _LARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			LONG HighPart;
		};
		struct
		{
			DWORD LowPart;
			LONG HighPart;
		} u;
		long long QuadPart;
	} LARGE_INTEGER;

	LARGE_INTEGER counter;
	__asm__ __volatile__("rdtsc" : "=a" (counter.u.LowPart), "=d" (counter.u.HighPart));
	return counter.QuadPart;
#endif
}
#endif

#if defined(LINUX)
#define RC_EXECUTABLE "rc"
//	#include <WinBase.h>
#ifdef __cplusplus
#include "Linux_Win32Wrapper.h"
#endif
#endif

#endif // _PLATFORM_H_
