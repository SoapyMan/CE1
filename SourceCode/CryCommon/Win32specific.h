// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   Win32specific.h
//  Version:     v1.00
//  Created:     31/03/2003 by Sergiy.
//  Compilers:   Visual Studio.NET
//  Description: Specific to Win32 declarations, inline functions etc.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#define RC_EXECUTABLE "rc.exe"
#define SIZEOF_PTR    4

//////////////////////////////////////////////////////////////////////////
// Standard includes.
//////////////////////////////////////////////////////////////////////////
#include <malloc.h>
#include <io.h>
#include <new.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <float.h>
//////////////////////////////////////////////////////////////////////////

// Special intrinsics
#include <intrin.h> // moved here to prevent assert from being redefined when included elsewhere
#include <process.h>

//////////////////////////////////////////////////////////////////////////
// Define platform independent types.
//////////////////////////////////////////////////////////////////////////
#include "BaseTypes.h"

#define THREADID_NULL 0

typedef uchar BYTE;
typedef uint  threadID;
typedef ulong DWORD;
typedef double        real;  //!< Biggest float-type on this machine.
typedef long          LONG;

#if CRY_PLATFORM_64BIT
typedef __int64 INT_PTR, *               PINT_PTR;
typedef unsigned __int64 UINT_PTR, *     PUINT_PTR;

typedef __int64 LONG_PTR, *              PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *    PULONG_PTR;

typedef ULONG_PTR DWORD_PTR, *           PDWORD_PTR;
#else
typedef __w64 int INT_PTR, *             PINT_PTR;
typedef __w64 uint UINT_PTR, *   PUINT_PTR;

typedef __w64 long LONG_PTR, *           PLONG_PTR;
typedef __w64 ulong ULONG_PTR, * PULONG_PTR;

typedef ULONG_PTR DWORD_PTR, *           PDWORD_PTR;
#endif

typedef void* THREAD_HANDLE;
typedef void* EVENT_HANDLE;

#ifndef FILE_ATTRIBUTE_NORMAL
	#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif

#define FP16_TERRAIN
#define TARGET_DEFAULT_ALIGN (0x4U)