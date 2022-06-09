 /*++

	VER_LEGALCOPYRIGHT_STR

Module Name:

    silabs_sdkddkversion.h

Abstract:

    A top-level, across all repos, authoritative, Master include file for versioning Windows SDK/DDK.

Environment:

    User or kernel mode

--*/
//
#if (_MSC_VER > 1000)
#pragma once
#endif

#ifndef	HOST_COMMON_INCLUDE_SILABS_SDKDDKVERSION_H_INCLUDED_BVHHTNCO7E
#define	HOST_COMMON_INCLUDE_SILABS_SDKDDKVERSION_H_INCLUDED_BVHHTNCO7E

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
// "If you define NTDDI_VERSION, you must also define _WIN32_WINNT." -- MSDN
#include <WinSDKVer.h>

#if	! defined(_WIN32_WINNT)
#if defined(_M_ARM)
#define	_WIN32_WINNT	_WIN32_WINNT_WIN10
#else
#define	_WIN32_WINNT	_WIN32_WINNT_WIN7
#endif
#endif	// ! defined(_WIN32_WINNT)
#if	! defined(WINVER)
#define	WINVER 			_WIN32_WINNT
#endif	// ! defined(WINVER)
#if	! defined(NTDDI_VERSION)
#if defined(_M_ARM)
#define	NTDDI_VERSION	NTDDI_WIN10
#else
#define	NTDDI_VERSION	NTDDI_WIN7
#endif
#endif	// ! defined(NTDDI_VERSION)

#if WINVER != _WIN32_WINNT
#error "Unexpected: WINVER and _WIN32_WINNT expected to be the same"
#endif // defined(NTDDI_VERSION) && ! defined(_WIN32_WINNT)

// "If you define NTDDI_VERSION, you must also define _WIN32_WINNT." -- MSDN
#if	defined(NTDDI_VERSION) && ! defined(_WIN32_WINNT)
#error "If you define NTDDI_VERSION, you must also define _WIN32_WINNT."
#endif	// defined(NTDDI_VERSION) && ! defined(_WIN32_WINNT)

#include <SdkDdkVer.h>

#include <windows.h>

// On x86 and x64, Windows 7 is the minimum target operating system.
// Note: For historical reasons, x64 is defined as AMD64, not to be confused with ARM64 :(
// _WIN32_WINNT and WINVER == _WIN32_WINNT_WIN7; NTDDI_VERSION == NTDDI_WIN7 
// On ARM, Windows 10 is the minimum target operating system
// _WIN32_WINNT and WINVER == _WIN32_WINNT_WIN10; NTDDI_VERSION == NTDDI_WIN10
#if defined(i386) || defined(_X86_) || defined(AMD64) || defined(_AMD64_) || defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
#if ! ((_WIN32_WINNT == _WIN32_WINNT_WIN7) && (WINVER == _WIN32_WINNT_WIN7))
//#error "On x86 and x64, Windows 7 is the minimum target operating system"
#pragma message("Fix me: On x86 and x64, Windows 7 is the minimum target operating system")
#endif // ! ((_WIN32_WINNT == _WIN32_WINNT_WIN7) && (WINVER == _WIN32_WINNT_WIN7))
#if NTDDI_VERSION != NTDDI_WIN7
//#error "On x86 and x64, Windows 7 is the minimum target operating system"
#pragma message("Fix me: On x86 and x64, Windows 7 is the minimum target operating system")
#endif // NTDDI_VERSION != NTDDI_WIN7
#elif defined(ARM) || defined(_ARM_) || defined(ARM64) || defined(_ARM64_) || defined(_M_ARM)
#if (_WIN32_WINNT != _WIN32_WINNT_WIN10) && (WINVER != _WIN32_WINNT_WIN10)
#error "On ARM, Windows 10 is the minimum target operating system"
#endif // (_WIN32_WINNT != _WIN32_WINNT_WIN10) && (WINVER != _WIN32_WINNT_WIN10)
#if NTDDI_VERSION != NTDDI_WIN10
#error "On ARM, Windows 10 is the minimum target operating system"
#endif // NTDDI_VERSION != NTDDI_WIN10
#else
#error "Unexpected/unhandled CPU architecture"
#endif

#endif // !defined(HOST_COMMON_INCLUDE_SILABS_SDKDDKVERSION_H_INCLUDED_BVHHTNCO7E)

