#pragma once

#define ASSERT_DEBUGGER_PROMPT 1

#ifndef _WIN32

#include <signal.h>
#define _DEBUG_BREAK raise(SIGINT)

#else

#if _MSC_VER >= 1400
#define _DEBUG_BREAK __debugbreak()
#else
#define _DEBUG_BREAK _asm int 0x03
#endif

#endif

enum ECryAssertType {
	_CRYASSERT_IGNORE_ALWAYS = -1,
	_CRYASSERT_BREAK = 1,
	_CRYASSERT_SKIP = 0,	// only when debugger is not present
};


#if defined(_RETAIL) || defined(_PROFILE)

#define	CRYASSERT_MSG(x, msgFmt, ...)	{ }
#define	CRYASSERT(x)					{ }
#define CRYASSERT_FAIL(msgFmt, ...)	{ }

#else

int _CryAssertMsg(const char* filename, int line, bool isSkipped, const char* expression, const char* statement, ...);

#define _CRYASSERT_BODY(expression, msgFmt, ...) \
	{ \
		static bool _ignoreAssert = false; \
		const int _assertResult = _CryAssertMsg(__FILE__, __LINE__, _ignoreAssert, expression, msgFmt, ##__VA_ARGS__ ); \
		if (_assertResult == _CRYASSERT_BREAK) { _DEBUG_BREAK; } \
		else if (_assertResult == _CRYASSERT_IGNORE_ALWAYS) { _ignoreAssert = true; } \
	}

#define	CRYASSERT_MSG(x, msgFmt, ...) \
	{ if (!(x)) _CRYASSERT_BODY(#x, msgFmt, ##__VA_ARGS__ ) }

#define	CRYASSERT(x)				CRYASSERT_MSG(x, nullptr)
#define CRYASSERT_FAIL(msgFmt, ...)	_CRYASSERT_BODY(nullptr, "%s: " msgFmt, __func__, ##__VA_ARGS__ )

#endif // _RETAIL || _PROFILE