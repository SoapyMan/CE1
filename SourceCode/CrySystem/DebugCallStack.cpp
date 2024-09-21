////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   debugcallstack.cpp
//  Version:     v1.00
//  Created:     1/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DebugCallStack.h"

#ifdef WIN32

#include <ISystem.h>
#include <ILog.h>
#include "Mailer.h"
#include "System.h"

#include <Process.h>
#include <imagehlp.h>
#include <time.h>
#include <crtdbg.h>

#include "Resource.h"

//! Needs one external of DLL handle.
extern HMODULE gDLLHandle;

#define MAX_PATH_LENGTH 1024
#define MAX_SYMBOL_LENGTH 512

static HWND hwndException = 0;
static int	PrintException(EXCEPTION_POINTERS* pex);

void PutVersion(char* str);

struct exception_codes {
	DWORD		exCode;
	const char* exName;
	const char* exDescription;
};

static exception_codes except_info[] = {
	{EXCEPTION_ACCESS_VIOLATION,		"ACCESS VIOLATION",
	"The thread tried to read from or write to a virtual address for which it does not have the appropriate access."},

	{EXCEPTION_ARRAY_BOUNDS_EXCEEDED,	"ARRAY BOUNDS EXCEEDED",
	"The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking."},

	{EXCEPTION_BREAKPOINT,				"BREAKPOINT",
	"A breakpoint was encountered."},

	{EXCEPTION_DATATYPE_MISALIGNMENT,	"DATATYPE MISALIGNMENT",
	"The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on."},

	{EXCEPTION_FLT_DENORMAL_OPERAND,	"FLT DENORMAL OPERAND",
	"One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value. "},

	{EXCEPTION_FLT_DIVIDE_BY_ZERO,		"FLT DIVIDE BY ZERO",
	"The thread tried to divide a floating-point value by a floating-point divisor of zero. "},

	{EXCEPTION_FLT_INEXACT_RESULT,		"FLT INEXACT RESULT",
	"The result of a floating-point operation cannot be represented exactly as a decimal fraction. "},

	{EXCEPTION_FLT_INVALID_OPERATION,	"FLT INVALID OPERATION",
	"This exception represents any floating-point exception not included in this list. "},

	{EXCEPTION_FLT_OVERFLOW,			"FLT OVERFLOW",
	"The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type. "},

	{EXCEPTION_FLT_STACK_CHECK,			"FLT STACK CHECK",
	"The stack overflowed or underflowed as the result of a floating-point operation. "},

	{EXCEPTION_FLT_UNDERFLOW,			"FLT UNDERFLOW",
	"The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type. "},

	{EXCEPTION_ILLEGAL_INSTRUCTION,		"ILLEGAL INSTRUCTION",
	"The thread tried to execute an invalid instruction. "},

	{EXCEPTION_IN_PAGE_ERROR,			"IN PAGE ERROR",
	"The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network. "},

	{EXCEPTION_INT_DIVIDE_BY_ZERO,		"INT DIVIDE BY ZERO",
	"The thread tried to divide an integer value by an integer divisor of zero. "},

	{EXCEPTION_INT_OVERFLOW,			"INT OVERFLOW",
	"The result of an integer operation caused a carry out of the most significant bit of the result. "},

	{EXCEPTION_INVALID_DISPOSITION,		"INVALID DISPOSITION",
	"An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception. "},

	{EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION",
	"The thread tried to continue execution after a noncontinuable exception occurred. "},

	{EXCEPTION_PRIV_INSTRUCTION,		"PRIV INSTRUCTION",
	"The thread tried to execute an instruction whose operation is not allowed in the current machine mode. "},

	{EXCEPTION_SINGLE_STEP,				"SINGLE STEP",
	"A trace trap or other single-instruction mechanism signaled that one instruction has been executed. "},

	{EXCEPTION_STACK_OVERFLOW,			"STACK OVERFLOW",
	"The thread used up its stack. "}
};

static void GetExceptionStrings(DWORD code, const char** pName, const char** pDescription)
{
	for (int i = 0; i < sizeof(except_info) / sizeof(except_info[0]); i++)
	{
		if (code == except_info[i].exCode)
		{
			*pName = except_info[i].exName;
			*pDescription = except_info[i].exDescription;
			return;
		}
	}

	*pName = "Unknown exception";
	*pDescription = "n/a";
}


//=============================================================================
LONG __stdcall UnhandledExceptionHandler(EXCEPTION_POINTERS* pex)
{
	DebugCallStack::instance()->handleException(pex);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void CreateCrashDump(EXCEPTION_POINTERS* pep, bool fullDump)
{
	SYSTEMTIME t;
	GetSystemTime(&t);

	char dumpPath[2048];
	if(fullDump)
		snprintf(dumpPath, sizeof(dumpPath), "%s_%4d%02d%02d_%02d%02d%02d.dmp", "FarCry", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	else
		snprintf(dumpPath, sizeof(dumpPath), "%s_%4d%02d%02d_%02d%02d%02d.mdmp", "FarCry", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	HANDLE dumpFileFd = CreateFileA(dumpPath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!dumpFileFd || dumpFileFd == INVALID_HANDLE_VALUE)
	{
		CryLogAlways("Unable to create crash dump");
		return;
	}

	const MINIDUMP_TYPE dumpType = fullDump ? MiniDumpWithFullMemory : MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

	MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;
	dumpExceptionInfo.ThreadId = GetCurrentThreadId();
	dumpExceptionInfo.ExceptionPointers = pep;
	dumpExceptionInfo.ClientPointers = FALSE;
	
	const bool result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFileFd, dumpType, (pep != 0) ? &dumpExceptionInfo : 0, 0, 0);
	if (!result)
		CryLogAlways("Minidump write error\n");
	else
		CryLogAlways("Minidump saved to:\n%s\n", dumpPath);

	// Close the file
	CloseHandle(dumpFileFd);
}

//=============================================================================
// Class Statics
//=============================================================================
DebugCallStack* DebugCallStack::m_instance = 0;

// Return single instance of class.
DebugCallStack* DebugCallStack::instance()
{
	if (!m_instance) {
		m_instance = new DebugCallStack;
	}
	return m_instance;
}

//------------------------------------------------------------------------------------------------------------------------
// Sets up the symbols forfunctions in the debug 	file.
//------------------------------------------------------------------------------------------------------------------------
DebugCallStack::DebugCallStack()
{
	prevExceptionHandler = 0;
	m_pSystem = 0;
	m_symbols = false;
}

DebugCallStack::~DebugCallStack()
{
}

bool DebugCallStack::initSymbols()
{
	if (m_symbols) 
		return true;

	m_symbols = false;

	SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST);

	HANDLE process = GetCurrentProcess();
	HRESULT result = SymInitialize(process, nullptr, TRUE);
	if (result) {
		m_symbols = true;
	}
	else 
	{
		result = SymInitialize(process, nullptr , FALSE);
		if (!result)
		{
			CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "SymInitialize faield");
		}
	}

	if (!m_symbols)
		SymCleanup(process);

	return result != 0;
}

void DebugCallStack::doneSymbols()
{
	if (m_symbols) 
	{
		SymCleanup(GetCurrentProcess());
	}
	m_symbols = false;
}

void DebugCallStack::getCallStack(std::vector<string>& functions)
{
	functions = m_functions;
}

void DebugCallStack::updateCallStack()
{
	if (initSymbols())
	{
		m_functions.clear();

		// (Not used) Rise exception to call updateCallStack(exc) method.
		//riseException();
		//updateCallStack(GetExceptionInformation())

		CONTEXT context;
		memset(&context, 0, sizeof(context));
		context.ContextFlags = CONTEXT_FULL;
		if (GetThreadContext(GetCurrentThread(), &context))
		{
#ifndef WIN64
			FillStackTrace(context.Eip, context.Esp, context.Ebp, &context);
#else
			FillStackTrace(context.Rip, context.Rsp, context.Rbp, &context);
#endif
		}

		doneSymbols();
	}
}

//------------------------------------------------------------------------------------------------------------------------
int DebugCallStack::updateCallStack(void* exception_pointer)
{
	static int callCount = 0;
	if (callCount > 0)
	{
		if (prevExceptionHandler)
		{
			// uninstall our exception handler.
			SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)prevExceptionHandler);
		}
		// Immidiate termination of process.
		abort();
	}
	callCount++;
	EXCEPTION_POINTERS* pex = (EXCEPTION_POINTERS*)exception_pointer;

	HANDLE process = GetCurrentProcess();

	//! Find source line at exception address.
	//m_excLine = lookupFunctionName( (void*)pex->ExceptionRecord->ExceptionAddress,true );

	//! Find Name of .DLL from Exception address.
	strcpy(m_excModule, "<Unknown>");


	if (m_symbols) {
		DWORD dwAddr = SymGetModuleBase(process, (DWORD)pex->ExceptionRecord->ExceptionAddress);
		if (dwAddr) {
			char szBuff[MAX_PATH_LENGTH];
			if (GetModuleFileName((HINSTANCE)dwAddr, szBuff, MAX_PATH_LENGTH)) {
				strcpy(m_excModule, szBuff);
				string path, fname, ext;

				char fdir[_MAX_PATH];
				char fdrive[_MAX_PATH];
				char file[_MAX_PATH];
				char fext[_MAX_PATH];
				_splitpath(m_excModule, fdrive, fdir, file, fext);
				_makepath(fdir, nullptr, nullptr, file, fext);

				strcpy(m_excModule, fdir);
		}
			}
	}
#if defined(WIN64)
	// Fill stack trace info.
	FillStackTrace(pex->ContextRecord->Rip, pex->ContextRecord->Rsp, pex->ContextRecord->Rbp, pex->ContextRecord);
#else
	// Fill stack trace info.
	FillStackTrace(pex->ContextRecord->Eip, pex->ContextRecord->Esp, pex->ContextRecord->Ebp, pex->ContextRecord);
#endif
	return EXCEPTION_CONTINUE_EXECUTION;
}

//////////////////////////////////////////////////////////////////////////
void DebugCallStack::FillStackTrace(DWORD64 eip, DWORD64 esp, DWORD64 ebp, PCONTEXT pContext)
{
	HANDLE hThread = GetCurrentThread();
	HANDLE hProcess = GetCurrentProcess();

	int count;
	STACKFRAME64 stack_frame;
	BOOL b_ret = TRUE; //Setup stack frame 
	memset(&stack_frame, 0, sizeof(stack_frame));
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = eip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = esp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = 0;
	stack_frame.AddrFrame.Offset = ebp;

	m_functions.clear();

	PCONTEXT pContextRecord = nullptr;
	CONTEXT CpuContext;
	if (pContext)
	{
		pContextRecord = &CpuContext;
		CpuContext = *pContext;
	}

	//While there are still functions on the stack.. 
	for (count = 0; count < MAX_DEBUG_STACK_ENTRIES && b_ret == TRUE; count++)
	{
		b_ret = StackWalk64(IMAGE_FILE_MACHINE_I386, hProcess, hThread, &stack_frame, pContextRecord, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);

		if (m_symbols)
		{
			string funcName = LookupFunctionName((void*)stack_frame.AddrPC.Offset, true);
			if (funcName.empty()) {
				funcName = "<Unknown Function>";
			}
			m_functions.push_back(funcName);
		}
		else 
		{
			DWORD p = stack_frame.AddrPC.Offset;
			char str[80];
			sprintf(str, "function=0x%X", p);
			m_functions.push_back(str);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------
string DebugCallStack::LookupFunctionName(void* pointer, bool fileInfo)
{
	string symName = "";

	HANDLE process = GetCurrentProcess();
	char symbolBuf[sizeof(SYMBOL_INFO) + MAX_SYMBOL_LENGTH];
	memset(symbolBuf, 0, sizeof(symbolBuf));
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuf;

	DWORD displacement = 0;
	DWORD64 displacement64 = 0;
	pSymbol->SizeOfStruct = sizeof(symbolBuf);
	pSymbol->MaxNameLen = MAX_SYMBOL_LENGTH;
	if (SymFromAddr(process, (DWORD64)pointer, &displacement64, pSymbol))
	{
		symName = string(pSymbol->Name) + "()";

		if (fileInfo)
		{
			// Lookup Line in source file.
			IMAGEHLP_LINE64 lineImg;
			memset(&lineImg, 0, sizeof(lineImg));
			lineImg.SizeOfStruct = sizeof(lineImg);

			if (SymGetLineFromAddr64(process, (DWORD_PTR)pointer, &displacement, &lineImg))
			{
				char lineNum[1024];
				itoa(lineImg.LineNumber, lineNum, 10);
				string path;

				char file[1024];
				char fname[1024];
				char fext[1024];
				_splitpath(lineImg.FileName, nullptr, nullptr, fname, fext);
				_makepath(file, nullptr, nullptr, fname, fext);
				string fileName = file;
				/*
				string finfo = string("[" ) + fileName + ", line:" + lineNum + "]";
				//symName += string(" --- [" ) + fileName + ", line:" + lineNum + "]";
				//char finfo[1024];
				//sprintf( finfo,"[%s,line:%d]",fileName.
				char temp[4096];
				sprintf( temp,"%30s --- %s",finfo.c_str(),symName.c_str() );
				symName = temp;
				*/

				symName += string("  [") + fileName + ":" + lineNum + "]";
			}
		}
	}

	CryLogAlways(symName.c_str());
	return symName;
}

void DebugCallStack::registerErrorCallback(ErrorCallback callBack) {
	m_errorCallbacks.push_back(callBack);
}

void DebugCallStack::unregisterErrorCallback(ErrorCallback callBack) {
	m_errorCallbacks.remove(callBack);
}

void DebugCallStack::installErrorHandler(ISystem* pSystem)
{
	m_pSystem = pSystem;
	prevExceptionHandler = (void*)SetUnhandledExceptionFilter(UnhandledExceptionHandler);
}

//////////////////////////////////////////////////////////////////////////
int	DebugCallStack::handleException(void* exception_pointer)
{
	EXCEPTION_POINTERS* pex = (EXCEPTION_POINTERS*)exception_pointer;
	int ret = 0;
	static bool firstTime = true;

	// uninstall our exception handler.
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)prevExceptionHandler);

	if (!firstTime)
	{
		CryLogAlways("Critical Exception! Called Multiple Times!");
		// Exception called more then once.
		exit(1);
	}

	firstTime = false;
	hwndException = CreateDialog(gDLLHandle, MAKEINTRESOURCE(IDD_EXCEPTION), nullptr, nullptr);

	if (initSymbols())
	{
		CryLogAlways("<CRITICAL ERROR>");
		char excCode[80];
		char excAddr[80];
		sprintf(excAddr, "0x%04X:0x%p", pex->ContextRecord->SegCs, pex->ExceptionRecord->ExceptionAddress);
		sprintf(excCode, "0x%08X", pex->ExceptionRecord->ExceptionCode);
		CryLogAlways("Exception: %s, at Address: %s", excCode, excAddr);

		// Rise exception to call updateCallStack method.
		updateCallStack(exception_pointer);

		//! Print exception dialog.
		ret = PrintException(pex);

		doneSymbols();
		exit(0);
	}
	ret = PrintException(pex);

	if (ret == IDB_EXIT)
	{
		// Immidiate exit.
		exit(1);
	}

	// Continue;
	return EXCEPTION_CONTINUE_EXECUTION;
}


void DebugCallStack::dumpCallStack(std::vector<string>& funcs)
{
	CryLogAlways("=============================================================================");
	int len = (int)funcs.size();
	for (int i = 0; i < len; i++) {
		const char* str = funcs[i].c_str();
		CryLogAlways("%2d) %s", len - i, str);
	}
	// Call all error callbacks.
	for (std::list<DebugCallStack::ErrorCallback>::iterator it = m_errorCallbacks.begin(); it != m_errorCallbacks.end(); ++it)
	{
		DebugCallStack::ErrorCallback callback = *it;
		string desc, data;
		callback(desc.c_str(), data.c_str());
		CryLogAlways("%s", (const char*)desc.c_str());
		CryLogAlways("%s", (const char*)data.c_str());
	}
	CryLogAlways("=============================================================================");
}

//////////////////////////////////////////////////////////////////////////
void DebugCallStack::LogCallstack()
{
	updateCallStack();		// is updating m_functions

	CryLogAlways("=============================================================================");
	int len = (int)m_functions.size();
	for (int i = 0; i < len; i++) {
		const char* str = m_functions[i].c_str();
		CryLogAlways("%2d) %s", len - i, str);
	}
	CryLogAlways("=============================================================================");
}

INT_PTR CALLBACK ExceptionDialogProc(HWND hwndDlg, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	static EXCEPTION_POINTERS* pex;

	static char errorString[32768] = "";

	switch (message)
	{
	case WM_INITDIALOG:
	{
		pex = (EXCEPTION_POINTERS*)lParam;
		HWND h;

		if (!GetISystem()->IsEditor())
		{
			h = GetDlgItem( hwndDlg, IDB_SAVE );
			if (h) ShowWindow( h, SW_HIDE );
		}

		if (pex->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
			h = GetDlgItem(hwndDlg, IDB_DUMP);
			if (h) SendMessage(h, WM_SETTEXT, 0, (LPARAM)"Save Crash Dump");
		}

		// Time and Version.
		char versionbuf[1024];
		strcpy(versionbuf, "");
		PutVersion(versionbuf);
		strcat(errorString, versionbuf);
		strcat(errorString, "\n");

		//! Get call stack functions.
		DebugCallStack* cs = DebugCallStack::instance();
		std::vector<string> funcs;
		cs->getCallStack(funcs);

		// Init dialog.
		int iswrite = 0;
		DWORD accessAddr = 0;
		char excCode[80];
		char excAddr[80];
		sprintf(excAddr, "0x%04X:0x%p", pex->ContextRecord->SegCs, pex->ExceptionRecord->ExceptionAddress);
		sprintf(excCode, "0x%08X", pex->ExceptionRecord->ExceptionCode);
		string moduleName = DebugCallStack::instance()->getExceptionModule();
		const char* excModule = moduleName.c_str();

		const char* excNameStr;
		const char* excDescStr;
		GetExceptionStrings(pex->ExceptionRecord->ExceptionCode, &excNameStr, &excDescStr);

		char desc[2048]{ 0 };
		if (pex->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
			if (pex->ExceptionRecord->NumberParameters > 1) {
				int iswrite = pex->ExceptionRecord->ExceptionInformation[0];
				accessAddr = pex->ExceptionRecord->ExceptionInformation[1];
				if (iswrite) {
					sprintf(desc, "Attempt to write data to address 0x%08X\r\nThe memory could not be \"written\"", accessAddr);
				}
				else {
					sprintf(desc, "Attempt to read from address 0x%08X\r\nThe memory could not be \"read\"", accessAddr);
				}
			}
		}

		char excDesc[2048];
		sprintf(excDesc, "%s\r\n%s", excNameStr, desc);

		CryLogAlways("Exception Code: %s", excCode);
		CryLogAlways("Exception Addr: %s", excAddr);
		CryLogAlways("Exception Module: %s", excModule);
		CryLogAlways("Exception Description: %s", excDescStr);
		DebugCallStack::instance()->dumpCallStack(funcs);

		static char errs[32768];
		sprintf(errs, "Exception Code: %s\nException Addr: %s\nException Module: %s\nException Description: %s, %s\n",
			excCode, excAddr, excModule, excNameStr, excDescStr);

		// Level Info.
		//char szLevel[1024];
		//const char *szLevelName = GetIEditor()->GetGameEngine()->GetLevelName();
		//const char *szMissionName = GetIEditor()->GetGameEngine()->GetMissionName();
		//sprintf( szLevel,"Level %s, Mission %s\n",szLevelName,szMissionName );
		//strcat( errs,szLevel );

		strcat(errs, "\nCall Stack Trace:\n");

		h = GetDlgItem(hwndDlg, IDC_EXCEPTION_DESC);
		if (h) SendMessage(h, EM_REPLACESEL, FALSE, (LONG_PTR)excDesc);

		h = GetDlgItem(hwndDlg, IDC_EXCEPTION_CODE);
		if (h) SendMessage(h, EM_REPLACESEL, FALSE, (LONG_PTR)excCode);

		h = GetDlgItem(hwndDlg, IDC_EXCEPTION_MODULE);
		if (h) SendMessage(h, EM_REPLACESEL, FALSE, (LONG_PTR)excModule);

		h = GetDlgItem(hwndDlg, IDC_EXCEPTION_ADDRESS);
		if (h) SendMessage(h, EM_REPLACESEL, FALSE, (LONG_PTR)excAddr);

		// Fill call stack.
		HWND callStack = GetDlgItem(hwndDlg, IDC_CALLSTACK);
		if (callStack) {
			char str[32768];
			strcpy(str, "");
			for (unsigned int i = 0; i < funcs.size(); i++) {
				char temp[4096];
				sprintf(temp, "%2d) %s", funcs.size() - i, (const char*)funcs[i].c_str());
				strcat(str, temp);
				strcat(str, "\r\n");
				strcat(errs, temp);
				strcat(errs, "\n");
			}

			// Call all error callbacks.
			for (std::list<DebugCallStack::ErrorCallback>::iterator it = cs->m_errorCallbacks.begin(); it != cs->m_errorCallbacks.end(); ++it) {
				DebugCallStack::ErrorCallback callback = *it;
				string desc, data;
				callback(desc.c_str(), data.c_str());
				CryLogAlways("%s", (const char*)desc.c_str());
				CryLogAlways("%s", (const char*)data.c_str());

				strcat(str, "======================================================\r\n");
				strcat(str, (const char*)desc.c_str());
				strcat(str, "\r\n");
				strcat(str, (const char*)data.c_str());
				strcat(str, "\r\n");

				strcat(errs, (const char*)desc.c_str());
				strcat(errs, "\n");
				strcat(errs, (const char*)data.c_str());
				strcat(errs, "\n");
			}

			SendMessage(callStack, WM_SETTEXT, FALSE, (LPARAM)str);
		}

		strcat(errorString, errs);
		FILE* f = fopen("error.log", "wt");
		if (f) {
			fwrite(errorString, strlen(errorString), 1, f);
			fclose(f);
		}

		if (hwndException)
			DestroyWindow(hwndException);
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDB_DUMP:
		{
			bool fullDump = false;
			if (pex->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT) {
				fullDump = true;
			}

			CreateCrashDump(pex, fullDump);
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
		break;

		case IDB_SAVE:
		{
			int res = MessageBox(nullptr, "Warning!\r\nEditor has crashed and is now in unstable state,\r\nand may crash again during saving of document.\r\nProceed with Save?", "Save Level", MB_YESNO | MB_ICONEXCLAMATION);
			if (res == IDYES)
			{
				// Make one additional backup.
				CSystem* pSystem = (CSystem*)DebugCallStack::instance()->GetSystem();
				if (pSystem && pSystem->GetUserCallback())
					pSystem->GetUserCallback()->OnSaveDocument();
				MessageBox(nullptr, "Level has been sucessfully saved!\r\nPress Ok to terminate Editor.", "Save", MB_OK);
			}
		}
		break;


		case IDB_EXIT:
			// Fall through.
			//case IDB_CONTINUE:

			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}
	return FALSE;
}


static int	PrintException(EXCEPTION_POINTERS* pex)
{
	return  DialogBoxParam(gDLLHandle, MAKEINTRESOURCE(IDD_CRITICAL_ERROR), nullptr, ExceptionDialogProc, (LPARAM)pex);
}

static void PutVersion(char* str)
{
	char exe[_MAX_PATH];
	DWORD dwHandle;
	unsigned int len;

	char ver[1024 * 8];
	GetModuleFileName(nullptr, exe, _MAX_PATH);
	int fv[4], pv[4];

	int verSize = GetFileVersionInfoSize(exe, &dwHandle);
	if (verSize > 0)
	{
		GetFileVersionInfo(exe, dwHandle, 1024 * 8, ver);
		VS_FIXEDFILEINFO* vinfo;
		VerQueryValue(ver, "\\", (void**)&vinfo, &len);

		fv[0] = vinfo->dwFileVersionLS & 0xFFFF;
		fv[1] = vinfo->dwFileVersionLS >> 16;
		fv[2] = vinfo->dwFileVersionMS & 0xFFFF;
		fv[3] = vinfo->dwFileVersionMS >> 16;

		pv[0] = vinfo->dwProductVersionLS & 0xFFFF;
		pv[1] = vinfo->dwProductVersionLS >> 16;
		pv[2] = vinfo->dwProductVersionMS & 0xFFFF;
		pv[3] = vinfo->dwProductVersionMS >> 16;
	}

	//! Get time.
	time_t ltime;
	time(&ltime);
	tm* today = localtime(&ltime);

	char s[1024];
	//! Use strftime to build a customized time string.
	strftime(s, 128, "Logged at %#c\n", today);
	strcat(str, s);
	sprintf(s, "FileVersion: %d.%d.%d.%d\n", fv[3], fv[2], fv[1], fv[0]);
	strcat(str, s);
	sprintf(s, "ProductVersion: %d.%d.%d.%d\n", pv[3], pv[2], pv[1], pv[0]);
	strcat(str, s);
}

#endif //WIN32