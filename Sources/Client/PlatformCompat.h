// PlatformCompat.h: Cross-platform shim for Windows types
//
// On Windows: includes <windows.h> and <mmsystem.h>
// On Linux: provides DWORD/WORD/BYTE/BOOL/LONG typedefs and ZeroMemory macro
//
// This allows files that still use Win32 types to compile on both platforms
// without touching every declaration.
//////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32
    #ifndef _WINSOCKAPI_
        #define _WINSOCKAPI_   // Prevent winsock.h from being included by windows.h
    #endif
    #include <windows.h>
    #include <mmsystem.h>
#else
    #include <cstdint>
    #include <cstring>

    // Win32 type shims for cross-platform compilation
    typedef uint32_t DWORD;
    typedef uint16_t WORD;
    typedef uint8_t  BYTE;
    typedef int32_t  LONG;
    typedef int      BOOL;

    #ifndef TRUE
        #define TRUE 1
    #endif
    #ifndef FALSE
        #define FALSE 0
    #endif

    #ifndef ZeroMemory
        #define ZeroMemory(dest, size) std::memset((dest), 0, (size))
    #endif

    #ifndef MAX_PATH
        #define MAX_PATH 260
    #endif

    // MSVC calling conventions are no-ops on GCC/Linux
    #ifndef __fastcall
        #define __fastcall
    #endif
#endif
