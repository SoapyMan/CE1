#ifdef USE_SDL
#include <SDL.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <sstream>
#include "Assert.h"


enum EMessageBoxButton
{
	MSGBOX_BUTTON_OK = 0,

	MSGBOX_BUTTON_YES,
	MSGBOX_BUTTON_NO,

	MSGBOX_BUTTON_ABORT,
	MSGBOX_BUTTON_RETRY,
	MSGBOX_BUTTON_IGNORE,
};

bool CryIsDebuggerPresent()
{
#ifdef _WIN32
	return IsDebuggerPresent();
#else
	const int statusFd = open("/proc/self/status", O_RDONLY);
	if (statusFd == -1)
		return false;

	char buf[4096];
	const ssize_t numRead = read(statusFd, buf, sizeof(buf) - 1);

	close(statusFd);
	if (statusFd <= 0)
		return false;

	buf[numRead] = '\0';

	constexpr char tracerPidString[] = "TracerPid:";
	const char* tracerPidPtr = strstr(buf, tracerPidString);
	if (!tracerPidPtr)
		return false;

	for (const char* characterPtr = tracerPidPtr + sizeof(tracerPidString) - 1; characterPtr <= buf + numRead; ++characterPtr)
	{
		if (CType::IsSpace(*characterPtr))
			continue;
		else
			return CType::IsDigit(*characterPtr) != 0 && *characterPtr != '0';
	}

	return false;
#endif
}

static int AssertMessageBox(const char* messageStr, const char* titleStr, bool canAbort)
{
#ifdef USE_SDL
	const SDL_MessageBoxButtonData yesNoButtons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, MSGBOX_BUTTON_YES, "Yes" },
		{ 0, MSGBOX_BUTTON_NO, "No"},
	};

	const SDL_MessageBoxButtonData abortButtons[] = {
		{ 0, MSGBOX_BUTTON_ABORT, "Abort" },
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, MSGBOX_BUTTON_RETRY, "Retry"},
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, MSGBOX_BUTTON_IGNORE, "Ignore" },
	};

	SDL_MessageBoxData msgBox{};
	msgBox.window = nullptr;
	msgBox.title = titleStr;
	msgBox.message = messageStr;

	if (canAbort)
	{
		msgBox.buttons = abortButtons;
		msgBox.numbuttons = sizeof(abortButtons) / sizeof(abortButtons[0]);
		msgBox.flags = SDL_MESSAGEBOX_ERROR;
	}
	else
	{
		msgBox.buttons = yesNoButtons;
		msgBox.numbuttons = sizeof(yesNoButtons) / sizeof(yesNoButtons[0]);
		msgBox.flags = SDL_MESSAGEBOX_INFORMATION;
	}

	int buttonId = -1;
	SDL_ShowMessageBox(&msgBox, &buttonId);

	return buttonId;
#elif _WIN32
	if (canAbort)
	{
		const int res = MessageBoxA(GetDesktopWindow(), messageStr, titleStr, MB_ABORTRETRYIGNORE);
		if (res == IDRETRY)
			return MSGBOX_BUTTON_RETRY;
		else if (res == IDIGNORE)
			return MSGBOX_BUTTON_IGNORE;
		else if (res == IDABORT)
			return MSGBOX_BUTTON_ABORT;
	}

	const int res = MessageBoxA(nullptr, messageStr, titleStr, MB_YESNO | MB_DEFBUTTON2);
	if (res == IDYES)
		return MSGBOX_BUTTON_YES;
	return MSGBOX_BUTTON_NO;
#endif
}

int _CryAssertMsg(const char* filename, int line, bool isSkipped, const char* expression, const char* statement, ...)
{
	if (isSkipped)
		return _CRYASSERT_SKIP;

#define MAX_WARNING_LENGTH	4096
	char szBuffer[MAX_WARNING_LENGTH + 32];

	std::ostringstream assertMessage;
	if (expression)
	{
		assertMessage << expression;
		assertMessage << "\n\n";
	}

	if (statement)
	{
		va_list argptr;
		va_start(argptr, statement);
		_vsnprintf(szBuffer, sizeof(szBuffer) - 32, statement, argptr);
		szBuffer[sizeof(szBuffer) - 8] = 0;
		va_end(argptr);

		assertMessage << szBuffer;
		assertMessage << "\n\n";
	}

	assertMessage << "file: ";
	assertMessage << filename;
	assertMessage << "\n";
	assertMessage << "line: ";
	assertMessage << line;

	if (!CryIsDebuggerPresent())
	{
		assertMessage << " - Display more asserts?";
		const int res = AssertMessageBox(assertMessage.str().c_str(), "Assertion failed", false);
		if (res != MSGBOX_BUTTON_YES)
			return _CRYASSERT_IGNORE_ALWAYS;
	}

#if ASSERT_DEBUGGER_PROMPT
	assertMessage << "\n -Press 'Abort' to Break the execution\n -Press 'Retry' to skip this CRYASSERT\n -Press 'Ignore' to suppress this message";

	const int res = AssertMessageBox(assertMessage.str().c_str(), "Assertion failed", true);
	if (res == MSGBOX_BUTTON_RETRY)
		return _CRYASSERT_SKIP;
	else if (res == MSGBOX_BUTTON_IGNORE)
		return _CRYASSERT_IGNORE_ALWAYS;
	else if (res == MSGBOX_BUTTON_ABORT)
		return _CRYASSERT_BREAK;

	return _CRYASSERT_SKIP;
#else
	return _CRYASSERT_BREAK;
#endif
}