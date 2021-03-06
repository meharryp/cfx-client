/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
//#include "GameInit.h"
#include <ICoreGameInit.h>

#define ERR_NORMAL 0 // will continue game execution, but not here
#define ERR_FATAL 1

#define BUFFER_LENGTH 32768

void SysError(const char* buffer)
{
#ifdef WIN32
	HWND wnd = nullptr;

#ifdef GTA_NY
	wnd = *(HWND*)0x1849DDC;
#else
    wnd = FindWindow(L"grcWindow", nullptr);
#endif

#ifdef _M_IX86
#ifdef _DEBUG
	if (IsDebuggerPresent())
	{
		__asm int 3
	}
#endif
#endif

	MessageBoxA(wnd, buffer, "Fatal Error", MB_OK | MB_ICONSTOP);

#ifdef _DEBUG
	assert(!"breakpoint time");
#endif

	TerminateProcess(GetCurrentProcess(), 1);
#else
	fprintf(stderr, "%s", buffer);

	abort();
#endif
}

static void GlobalErrorHandler(int eType, const char* buffer)
{
	static bool inError = false;

	trace("GlobalError: %s\n", buffer);

	if (inError)
	{
		static bool inRecursiveError = false;

		if (inRecursiveError)
		{
			SysError("RECURSIVE RECURSIVE ERROR");
		}

		inRecursiveError = true;
		SysError(va("Recursive error: %s", buffer));
	}

	inError = true;

	if (eType == ERR_NORMAL)
	{
#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
		// TODO: UI killer for pre-connected state
		ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

		if (gameInit && gameInit->GetGameLoaded())
		{
			static wchar_t wbuffer[BUFFER_LENGTH];
			mbstowcs(wbuffer, buffer, _countof(wbuffer));

			gameInit->KillNetwork(wbuffer);
		}
		else
#endif
		{
			SysError(buffer);
		}
	}
	else
	{
		SysError(buffer);
	}

	inError = false;
}

void GlobalError(const char* string, const fmt::ArgList& formatList)
{
	GlobalErrorHandler(ERR_NORMAL, fmt::sprintf(string, formatList).c_str());
}

void FatalError(const char* string, const fmt::ArgList& formatList)
{
	GlobalErrorHandler(ERR_FATAL, fmt::sprintf(string, formatList).c_str());
}
