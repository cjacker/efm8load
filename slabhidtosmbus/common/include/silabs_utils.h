 /*++

	VER_LEGALCOPYRIGHT_STR

Module Name:

    silabs_utils.h

Abstract:

    A top-level, across-all-repos, collection of some "useful" constructs

Environment:

    User or kernel mode

--*/
//
#if (_MSC_VER > 1000)
#pragma once
#endif

#ifndef	HOST_COMMON_INCLUDE_SILABS_UTILS_H_INCLUDED_DL027EJB47
#define	HOST_COMMON_INCLUDE_SILABS_UTILS_H_INCLUDED_DL027EJB47

#include	"silabs_sal.h"
#if	defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include "OsDep.h"

// Pointedly, specifically, lose higher bits during conversion to smaller types
// (pointedly, specifically, identifying that this is not a loss of (meaningful) data))
#define	SILABS_CONVERT_LARGER_TO_8BITS(i)	((BYTE)(0xFF & (i)))
#define	SILABS_CONVERT_LARGER_TO_16BITS(i)	((unsigned short)(0xFFFF & (i)))
#define SILABS_CONVERT_INT_TO_BYTE(i)	SILABS_CONVERT_LARGER_TO_8BITS(i)
#define SILABS_CONVERT_BOOL_TO_BYTE(i)	SILABS_CONVERT_INT_TO_BYTE(i)
#define SILABS_CONVERT_INT_TO_WORD(i)	SILABS_CONVERT_LARGER_TO_16BITS(i)

#define SILABS_MAKEWORD(h, l)	((WORD)(((SILABS_CONVERT_LARGER_TO_8BITS(h)) << 8) | (SILABS_CONVERT_LARGER_TO_8BITS(l))))
#define SILABS_LOBYTE(w)           (SILABS_CONVERT_LARGER_TO_8BITS(w))
#define SILABS_HIBYTE(w)           (SILABS_CONVERT_LARGER_TO_8BITS((w) >> 8))

#if	!defined(SIZEOF_ARRAY)
#define SIZEOF_ARRAY( _a ) (sizeof( (_a) ) / sizeof( (_a)[0]))
#endif


// HANDLE validation helpers
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
#if	defined(_WIN32)
static inline BOOL SiLabs_Utils_IsInvalidHandle (_In_opt_ const HANDLE h) { return (INVALID_HANDLE_VALUE == h) || ((const HANDLE) 0 == h); }
#else
static inline BOOL SiLabs_Utils_IsInvalidHandle (_In_opt_ const HANDLE h) { return ((const HANDLE) 0 == h); }
#endif
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidHandle (_In_opt_ const HANDLE h) { return ! SiLabs_Utils_IsInvalidHandle(h); }

// Now minus start time gives elapsed time, validate now is post-StartTime, etc.
static inline ULONGLONG SiLabs_Utils_GetStartTime(void)
{
#if	defined(_WIN32)
#if (_WIN32_WINNT >= 0x0600)
	return GetTickCount64();	// "Retrieves the number of milliseconds that have elapsed since the system was started." -- MSDN
#else
	return (ULONGLONG) GetTickCount();
#endif
#elif	defined(__APPLE__)
	mach_timebase_info_data_t info;
	
	mach_timebase_info(&info);
	
	return (ULONGLONG) ((mach_absolute_time() * info.numer / info.denom) / 1000000);
#else
	return GetTickCount();
#endif	// defined(_WIN32)
}

// Now minus start time gives elapsed time, validate now is post-StartTime, etc.
static inline ULONGLONG SiLabs_Utils_ElapsedTimeSince(_In_ const ULONGLONG StartTime)
{
	const ULONGLONG NowTime = SiLabs_Utils_GetStartTime();

	return StartTime < NowTime ? NowTime - StartTime : 0;
}



// "ExitThread is the preferred method of exiting a thread in C code. However, in C++ code, the thread
// is exited before any destructors can be called or any other automatic cleanup can be performed. Therefore,
// in C++ code, you should return from your thread function." -- MSDN
static inline DWORD SiLabs_Utils_ExitThread(_In_ const DWORD dwExitCode)
{
	// " If a thread returns STILL_ACTIVE (259) as an error code, applications that test for this value could
	// interpret it to mean that the thread is still running and continue to test for the completion of the thread
	// after the thread has terminated, which could put the application into an infinite loop." -- MSDN
	if (259 == dwExitCode)
	{
	}

#if ! defined(__cplusplus)
	ExitThread(dwExitCode);
#endif // defined(__cplusplus)

	return dwExitCode;
}

#ifdef __cplusplus	// These overloaded IsValidParam functions won't/don't work in C. They are C++ only
//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(LPDWORD)
//
// Checks validity of an LPDWORD pointer value.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(LPDWORD lpdwPointer)
{
	DWORD temp = 0;

	if (!lpdwPointer) return FALSE;

	try 
	{
		temp = *lpdwPointer;
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(LPWORD)
//
// Checks validity of an LPWORD pointer value.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(LPWORD lpwPointer)
{
	WORD temp = 0;

	if (!lpwPointer) return FALSE;

	try
	{
		temp = *lpwPointer;
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(LPBYTE)
//
// Checks validity of an LPBYTE pointer value.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(LPBYTE lpbPointer)
{
	BYTE temp = 0;

	if (!lpbPointer) return FALSE;

	try
	{
		temp = *lpbPointer;
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}


//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(LPVOID)
//
// Checks validity of an LPVOID pointer value.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(const void *lpVoidPointer)
{
	BYTE temp = 0;

	if (!lpVoidPointer) return FALSE;

	try
	{
		temp = *((BYTE*)lpVoidPointer);
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(HANDLE*, BOOL)
//
// Checks validity of an HANDLE* pointer value. If BOOL is true, also check validity of HANDLE.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(HANDLE* lpHandle, _In_ const BOOL bVerifyIsValidHandle = TRUE)
{
	HANDLE temp = 0;

	if (!lpHandle) return FALSE;

	try
	{
		temp = *lpHandle;
	}
	catch(...)
	{
		return FALSE;
	}
	return bVerifyIsValidHandle ? SiLabs_Utils_IsValidHandle(*lpHandle) : TRUE;
}

//------------------------------------------------------------------------
// ValidParam(LPVOID, LPBYTE)
//
// Checks validity of LPVOID, LPBYTE pair of pointer values.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(LPVOID lpVoidPointer, LPBYTE lpbPointer)
{
	return SiLabs_Utils_IsValidParam(lpVoidPointer) && SiLabs_Utils_IsValidParam(lpbPointer);
}

//------------------------------------------------------------------------
// SiLabs_Utils_IsValidParam(LPVOID, LPDWORD)
//
// Checks validity of LPVOID, LPDWORD pair of pointer values.
//------------------------------------------------------------------------
_Check_return_
_Ret_range_(FALSE,TRUE)
_Success_(return == TRUE)
static inline BOOL SiLabs_Utils_IsValidParam(LPVOID lpVoidPointer, LPDWORD lpdwPointer)
{
	return SiLabs_Utils_IsValidParam(lpVoidPointer) && SiLabs_Utils_IsValidParam(lpdwPointer);
}
#endif // __cplusplus

#if	defined(_WIN32)
#include <stdio.h>
#include <stdarg.h>
static inline void SiLabs_Utils_DebugPrint(_In_ _Printf_format_string_ const char *pszFormat, ...)
{
	va_list ap;
	char s[256];
	va_start(ap, pszFormat);
	(void) vsnprintf_s(s, SIZEOF_ARRAY(s) - 1, _TRUNCATE, pszFormat, ap);
	va_end(ap);
	OutputDebugStringA(s);

	return; // void
}
#endif	// defined(_WIN32)

#endif // !defined(HOST_COMMON_INCLUDE_SILABS_UTILS_H_INCLUDED_DL027EJB47)

