// Platform.h: Cross-platform compatibility header
//
// On Windows: includes the standard Windows headers (winsock2, windows, etc.)
// On Linux: provides shims for Windows types, APIs, and constants
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32

// ============================================================================
// Windows build path — include native headers
// ============================================================================
#define _WINSOCKAPI_   // Prevent old winsock.h from loading
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>  // Must be before windows.h
#include <windows.h>
#include <windowsx.h>
#include <winbase.h>
#include <mmsystem.h>
#include <direct.h>
#include <process.h>

// Windows winsock2 doesn't define socklen_t
typedef int socklen_t;

// SD_SEND may not be defined in all winsock2 versions
#ifndef SD_SEND
#define SD_SEND 1
#endif

#else // !_WIN32

// ============================================================================
// Linux/POSIX build path — shim Windows types and APIs
// ============================================================================

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <fnmatch.h>
#include <strings.h>
#include <limits.h>

// --------------------------------------------------------------------------
// Basic Windows types
// --------------------------------------------------------------------------
typedef uint32_t    DWORD;
typedef uint16_t    WORD;
typedef uint8_t     BYTE;
typedef int         BOOL;
typedef unsigned int UINT;
typedef long        LONG;
typedef unsigned long ULONG;
typedef short       SHORT;
typedef char*       LPSTR;
typedef const char* LPCSTR;
typedef void*       LPVOID;
typedef const void* LPCVOID;
typedef BYTE*       LPBYTE;
typedef unsigned char* PUCHAR;

struct POINT { LONG x; LONG y; };
struct RECT  { LONG left; LONG top; LONG right; LONG bottom; };

inline BOOL SetRect(RECT* rc, int left, int top, int right, int bottom)
{
    if (!rc) return 0;
    rc->left = left; rc->top = top; rc->right = right; rc->bottom = bottom;
    return 1;
}

// ZeroMemory -> memset
#define ZeroMemory(dst, size) memset((dst), 0, (size))

// SecureZeroMemory -> explicit_bzero
#define SecureZeroMemory(dst, size) explicit_bzero((dst), (size))

// wsprintf -> sprintf (truncating variant)
#define wsprintf sprintf

// Calling conventions (meaningless on Linux)
#define __fastcall
#define __cdecl
#define __stdcall
#define FAR
#define CALLBACK
#define APIENTRY
#define WINAPI

// MSVC __int64
typedef int64_t __int64;

typedef void*       HWND;
typedef void*       HANDLE;
typedef void*       HINSTANCE;
typedef void*       HMODULE;
typedef void*       HICON;
typedef void*       HCURSOR;
typedef void*       HBRUSH;
typedef void*       HMENU;
typedef void*       WNDPROC;

typedef long        LRESULT;
typedef uintptr_t   WPARAM;
typedef intptr_t    LPARAM;
typedef uintptr_t   DWORD_PTR;

typedef uint32_t    MMRESULT;

// NTSTATUS for BCrypt compat (not used on Linux but may be referenced)
typedef long        NTSTATUS;

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define MAX_PATH 4096
#define TRUE 1
#define FALSE 0

// --------------------------------------------------------------------------
// File attribute constants
// --------------------------------------------------------------------------
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL    0x80

// --------------------------------------------------------------------------
// CreateFile / ReadFile / WriteFile / GetFileSize / CloseHandle shims
// --------------------------------------------------------------------------
#define GENERIC_READ     0x80000000
#define GENERIC_WRITE    0x40000000
#define OPEN_EXISTING    3
#define OPEN_ALWAYS      4
#define FILE_SHARE_READ  1
#define FILE_SHARE_WRITE 2
#define FILE_BEGIN       0
#define FILE_END         2

inline HANDLE CreateFile(const char* path, DWORD access, DWORD shareMode,
                         void* secAttr, DWORD creation, DWORD flags, void* hTemplate)
{
    (void)shareMode; (void)secAttr; (void)flags; (void)hTemplate;
    int oflags = 0;
    if ((access & GENERIC_READ) && (access & GENERIC_WRITE))
        oflags = O_RDWR;
    else if (access & GENERIC_WRITE)
        oflags = O_WRONLY;
    else
        oflags = O_RDONLY;

    if (creation == OPEN_ALWAYS)
        oflags |= O_CREAT;
    // OPEN_EXISTING: no extra flags needed — open() fails if file doesn't exist

    int fd = open(path, oflags, 0644);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    return (HANDLE)(intptr_t)fd;
}

inline BOOL ReadFile(HANDLE h, void* buf, DWORD nBytes, DWORD* bytesRead, void* overlapped)
{
    (void)overlapped;
    int fd = (int)(intptr_t)h;
    ssize_t r = read(fd, buf, nBytes);
    if (r < 0) { if (bytesRead) *bytesRead = 0; return 0; }
    if (bytesRead) *bytesRead = (DWORD)r;
    return 1;
}

inline BOOL WriteFile(HANDLE h, const void* buf, DWORD nBytes, DWORD* bytesWritten, void* overlapped)
{
    (void)overlapped;
    int fd = (int)(intptr_t)h;
    ssize_t w = write(fd, buf, nBytes);
    if (w < 0) { if (bytesWritten) *bytesWritten = 0; return 0; }
    if (bytesWritten) *bytesWritten = (DWORD)w;
    return 1;
}

inline DWORD GetFileSize(HANDLE h, DWORD* high)
{
    (void)high;
    int fd = (int)(intptr_t)h;
    off_t cur = lseek(fd, 0, SEEK_CUR);
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, cur, SEEK_SET); // restore position
    if (size < 0) return (DWORD)-1;
    return (DWORD)size;
}

inline DWORD SetFilePointer(HANDLE h, LONG low, LONG* high, DWORD method)
{
    (void)high;
    int fd = (int)(intptr_t)h;
    int whence = SEEK_SET;
    if (method == FILE_END) whence = SEEK_END;
    off_t pos = lseek(fd, low, whence);
    return (DWORD)pos;
}

inline BOOL CloseHandle(HANDLE h)
{
    int fd = (int)(intptr_t)h;
    return close(fd) == 0;
}

// --------------------------------------------------------------------------
// Filesystem shims
// --------------------------------------------------------------------------
inline DWORD GetFileAttributes(const char* path)
{
    struct stat st;
    if (stat(path, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    DWORD attrs = 0;
    if (S_ISDIR(st.st_mode)) attrs |= FILE_ATTRIBUTE_DIRECTORY;
    return attrs;
}
// Alias for the "A" variant used in some places
inline DWORD GetFileAttributesA(const char* path) { return GetFileAttributes(path); }

inline DWORD GetModuleFileNameA(void* hModule, char* buf, DWORD size)
{
    (void)hModule;
    ssize_t len = readlink("/proc/self/exe", buf, size - 1);
    if (len < 0) return 0;
    buf[len] = '\0';
    return (DWORD)len;
}

inline BOOL CreateDirectoryA(const char* path, void* secAttr)
{
    (void)secAttr;
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

inline int _mkdir(const char* path)
{
    return (mkdir(path, 0755) == 0 || errno == EEXIST) ? 0 : -1;
}

inline DWORD GetFullPathNameA(const char* fileName, DWORD bufSize, char* buf, char** filePart)
{
    char* result = realpath(fileName, nullptr);
    if (result) {
        DWORD len = (DWORD)strlen(result);
        if (len < bufSize) {
            strcpy(buf, result);
        } else {
            strncpy(buf, result, bufSize - 1);
            buf[bufSize - 1] = '\0';
            len = bufSize - 1;
        }
        free(result);
        if (filePart) *filePart = nullptr;
        return len;
    }
    // File doesn't exist yet — just copy the input
    DWORD len = (DWORD)strlen(fileName);
    if (len < bufSize) {
        strcpy(buf, fileName);
    } else {
        strncpy(buf, fileName, bufSize - 1);
        buf[bufSize - 1] = '\0';
        len = bufSize - 1;
    }
    if (filePart) *filePart = nullptr;
    return len;
}

inline DWORD GetLastError()
{
    return (DWORD)errno;
}

// --------------------------------------------------------------------------
// WIN32_FIND_DATA / FindFirstFile / FindNextFile / FindClose
// --------------------------------------------------------------------------
struct WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    char cFileName[MAX_PATH];
    // internal state
    DIR* _dir;
    char _pattern[MAX_PATH];
    char _dirpath[MAX_PATH];
};

inline void _FindDataFillAttrs(WIN32_FIND_DATA* fd)
{
    char fullpath[MAX_PATH * 2];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", fd->_dirpath, fd->cFileName);
    struct stat st;
    fd->dwFileAttributes = 0;
    if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode))
        fd->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
}

inline HANDLE FindFirstFile(const char* lpFileName, WIN32_FIND_DATA* lpFindFileData)
{
    // Split "dir/pattern" into directory and glob pattern
    char dirpath[MAX_PATH];
    char pattern[MAX_PATH];
    strncpy(dirpath, lpFileName, MAX_PATH - 1);
    dirpath[MAX_PATH - 1] = '\0';

    char* lastSlash = strrchr(dirpath, '/');
    if (!lastSlash) lastSlash = strrchr(dirpath, '\\');
    if (lastSlash) {
        strncpy(pattern, lastSlash + 1, MAX_PATH - 1);
        pattern[MAX_PATH - 1] = '\0';
        *(lastSlash) = '\0';
    } else {
        strncpy(pattern, dirpath, MAX_PATH - 1);
        pattern[MAX_PATH - 1] = '\0';
        strcpy(dirpath, ".");
    }

    DIR* dir = opendir(dirpath);
    if (!dir) return INVALID_HANDLE_VALUE;

    strncpy(lpFindFileData->_pattern, pattern, MAX_PATH - 1);
    lpFindFileData->_pattern[MAX_PATH - 1] = '\0';
    strncpy(lpFindFileData->_dirpath, dirpath, MAX_PATH - 1);
    lpFindFileData->_dirpath[MAX_PATH - 1] = '\0';
    lpFindFileData->_dir = dir;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (fnmatch(pattern, entry->d_name, 0) == 0) {
            strncpy(lpFindFileData->cFileName, entry->d_name, MAX_PATH - 1);
            lpFindFileData->cFileName[MAX_PATH - 1] = '\0';
            _FindDataFillAttrs(lpFindFileData);
            return (HANDLE)lpFindFileData;
        }
    }

    closedir(dir);
    lpFindFileData->_dir = nullptr;
    return INVALID_HANDLE_VALUE;
}

inline BOOL FindNextFile(HANDLE hFindFile, WIN32_FIND_DATA* lpFindFileData)
{
    (void)hFindFile;
    if (!lpFindFileData->_dir) return 0;

    struct dirent* entry;
    while ((entry = readdir(lpFindFileData->_dir)) != nullptr) {
        if (fnmatch(lpFindFileData->_pattern, entry->d_name, 0) == 0) {
            strncpy(lpFindFileData->cFileName, entry->d_name, MAX_PATH - 1);
            lpFindFileData->cFileName[MAX_PATH - 1] = '\0';
            _FindDataFillAttrs(lpFindFileData);
            return 1;
        }
    }
    return 0;
}

inline BOOL FindClose(HANDLE hFindFile)
{
    // hFindFile points to the WIN32_FIND_DATA which has the DIR*
    WIN32_FIND_DATA* fd = (WIN32_FIND_DATA*)hFindFile;
    if (fd && fd->_dir) {
        closedir(fd->_dir);
        fd->_dir = nullptr;
    }
    return 1;
}

// --------------------------------------------------------------------------
// SYSTEMTIME and GetLocalTime
// --------------------------------------------------------------------------
struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

inline void GetLocalTime(SYSTEMTIME* st)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm t;
    localtime_r(&ts.tv_sec, &t);
    st->wYear         = (WORD)(t.tm_year + 1900);
    st->wMonth        = (WORD)(t.tm_mon + 1);
    st->wDay          = (WORD)t.tm_mday;
    st->wHour         = (WORD)t.tm_hour;
    st->wMinute       = (WORD)t.tm_min;
    st->wSecond       = (WORD)t.tm_sec;
    st->wMilliseconds = (WORD)(ts.tv_nsec / 1000000);
    st->wDayOfWeek    = (WORD)t.tm_wday;
}

// --------------------------------------------------------------------------
// Process creation shims
// --------------------------------------------------------------------------
#define STILL_ACTIVE    259
#define CREATE_NEW_CONSOLE 0x00000010

struct STARTUPINFOA {
    DWORD cb;
};

struct PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD  dwProcessId;
    DWORD  dwThreadId;
};

inline BOOL GetExitCodeProcess(HANDLE hProcess, DWORD* lpExitCode)
{
    pid_t pid = (pid_t)(intptr_t)hProcess;
    int status = 0;
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0) {
        // Still running
        if (lpExitCode) *lpExitCode = STILL_ACTIVE;
        return 1;
    }
    if (result == pid) {
        if (lpExitCode) *lpExitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        return 1;
    }
    return 0;
}

inline BOOL CreateProcessA(
    const char* lpApplicationName,
    char* lpCommandLine,
    void* lpProcessAttributes,
    void* lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    void* lpEnvironment,
    const char* lpCurrentDirectory,
    STARTUPINFOA* lpStartupInfo,
    PROCESS_INFORMATION* lpProcessInformation)
{
    (void)lpApplicationName; (void)lpProcessAttributes; (void)lpThreadAttributes;
    (void)bInheritHandles; (void)dwCreationFlags; (void)lpEnvironment;
    (void)lpCurrentDirectory; (void)lpStartupInfo;

    pid_t pid = fork();
    if (pid < 0) return 0;
    if (pid == 0) {
        // Child — execute command via shell
        execl("/bin/sh", "sh", "-c", lpCommandLine, (char*)nullptr);
        _exit(127);
    }
    // Parent
    if (lpProcessInformation) {
        lpProcessInformation->hProcess = (HANDLE)(intptr_t)pid;
        lpProcessInformation->hThread = nullptr;
        lpProcessInformation->dwProcessId = (DWORD)pid;
        lpProcessInformation->dwThreadId = 0;
    }
    return 1;
}

// --------------------------------------------------------------------------
// Threading / timing
// --------------------------------------------------------------------------
inline void Sleep(DWORD ms) { usleep(ms * 1000); }
inline void _endthread() { pthread_exit(nullptr); }

// GetTickCount / timeGetTime — use GameClock
#include "CommonTypes.h"
inline DWORD GetTickCount()
{
    return GameClock::GetTimeMS();
}
inline DWORD timeGetTime()
{
    return GameClock::GetTimeMS();
}

// --------------------------------------------------------------------------
// String compat
// --------------------------------------------------------------------------
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

// strcpy_s 2-arg form: strcpy_s(dst, src) where dst is a fixed-size char array
// The template version captures the array size automatically.
template<size_t N>
inline int strcpy_s(char (&dst)[N], const char* src)
{
    if (!src) { dst[0] = '\0'; return 22; } // EINVAL
    strncpy(dst, src, N);
    dst[N - 1] = '\0';
    return 0;
}

// strcpy_s 3-arg form: strcpy_s(dst, size, src)
inline int strcpy_s(char* dst, size_t dstSize, const char* src)
{
    if (!dst || dstSize == 0) return 22;
    if (!src) { dst[0] = '\0'; return 22; }
    strncpy(dst, src, dstSize);
    dst[dstSize - 1] = '\0';
    return 0;
}

// _TRUNCATE for strncpy_s
#define _TRUNCATE ((size_t)-1)

// strncpy_s: 4-arg form used by ASIOSocket.cpp
// When count == _TRUNCATE, copies up to dstSize-1 chars and null-terminates.
inline int strncpy_s(char* dst, size_t dstSize, const char* src, size_t count)
{
    if (!dst || dstSize == 0) return 22; // EINVAL
    if (!src) { dst[0] = '\0'; return 22; }
    if (count == _TRUNCATE || count >= dstSize)
        count = dstSize - 1;
    strncpy(dst, src, count);
    dst[count] = '\0';
    return 0;
}

// strncpy_s: template form for fixed-size arrays
template<size_t N>
inline int strncpy_s(char (&dst)[N], const char* src, size_t count)
{
    return strncpy_s(dst, N, src, count);
}

// strtok_s -> strtok_r
inline char* strtok_s(char* str, const char* delim, char** context)
{
    return strtok_r(str, delim, context);
}

// --------------------------------------------------------------------------
// Console constants and stubs
// --------------------------------------------------------------------------
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_INPUT_HANDLE  ((DWORD)-10)

#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008

#define ENABLE_WINDOW_INPUT  0x0008

#define KEY_EVENT              0x0001
#define WINDOW_BUFFER_SIZE_EVENT 0x0004

#define VK_RETURN 0x0D
#define VK_BACK   0x08
#define VK_LEFT   0x25
#define VK_RIGHT  0x27
#define VK_HOME   0x24
#define VK_END    0x23
#define VK_DELETE 0x2E
#define VK_F1     0x70
#define VK_F2     0x71
#define VK_F3     0x72
#define VK_F4     0x73
#define VK_F5     0x74
#define VK_F6     0x75
#define VK_F7     0x76
#define VK_F8     0x77
#define VK_F9     0x78
#define VK_F10    0x79
#define VK_F11    0x7A
#define VK_F12    0x7B
#define VK_INSERT 0x2D

struct COORD {
    SHORT X;
    SHORT Y;
};

struct SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
};

struct CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
};

struct KEY_EVENT_RECORD {
    BOOL  bKeyDown;
    WORD  wRepeatCount;
    WORD  wVirtualKeyCode;
    WORD  wVirtualScanCode;
    union {
        char  AsciiChar;
        WORD  UnicodeChar;
    } uChar;
    DWORD dwControlKeyState;
};

struct WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
};

struct INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
    } Event;
};

inline HANDLE GetStdHandle(DWORD h) { (void)h; return (HANDLE)(intptr_t)1; }

inline BOOL GetConsoleScreenBufferInfo(HANDLE h, CONSOLE_SCREEN_BUFFER_INFO* info)
{
    (void)h;
    if (info) {
        info->wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        info->srWindow.Left = 0;
        info->srWindow.Top = 0;
        info->srWindow.Right = 79;
        info->srWindow.Bottom = 24;
        info->dwCursorPosition.X = 0;
        info->dwCursorPosition.Y = 0;
        info->dwSize.X = 80;
        info->dwSize.Y = 25;
    }
    return 1;
}

inline BOOL SetConsoleTextAttribute(HANDLE h, WORD attr) { (void)h; (void)attr; return 1; }
inline BOOL AllocConsole() { return 1; }
inline BOOL SetConsoleTitle(const char* t) { (void)t; return 1; }
inline BOOL SetConsoleTitleA(const char* t) { (void)t; return 1; }
inline BOOL GetConsoleMode(HANDLE h, DWORD* mode) { (void)h; if (mode) *mode = 0; return 1; }
inline BOOL SetConsoleMode(HANDLE h, DWORD mode) { (void)h; (void)mode; return 1; }

inline BOOL GetNumberOfConsoleInputEvents(HANDLE h, DWORD* numEvents)
{
    (void)h;
    if (numEvents) *numEvents = 0;
    return 1;
}

inline BOOL ReadConsoleInput(HANDLE h, INPUT_RECORD* buf, DWORD len, DWORD* numRead)
{
    (void)h; (void)buf; (void)len;
    if (numRead) *numRead = 0;
    return 1;
}

inline BOOL WriteConsoleA(HANDLE h, const void* buf, DWORD len, DWORD* written, void* reserved)
{
    (void)h; (void)reserved;
    size_t w = fwrite(buf, 1, len, stdout);
    if (written) *written = (DWORD)w;
    return 1;
}

inline BOOL FillConsoleOutputCharacterA(HANDLE h, char ch, DWORD len, COORD pos, DWORD* written)
{
    (void)h; (void)ch; (void)len; (void)pos;
    if (written) *written = len;
    return 1;
}

inline BOOL FillConsoleOutputAttribute(HANDLE h, WORD attr, DWORD len, COORD pos, DWORD* written)
{
    (void)h; (void)attr; (void)len; (void)pos;
    if (written) *written = len;
    return 1;
}

inline BOOL SetConsoleCursorPosition(HANDLE h, COORD pos)
{
    (void)h; (void)pos;
    return 1;
}

// --------------------------------------------------------------------------
// Window stubs (needed by Wmain.cpp to compile)
// --------------------------------------------------------------------------

// Window class
#define CS_HREDRAW   0x0002
#define CS_VREDRAW   0x0001
#define CS_OWNDC     0x0020
#define CS_DBLCLKS   0x0008
#define COLOR_WINDOW 5

struct WNDCLASS {
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName;
    LPCSTR    lpszClassName;
};

inline BOOL RegisterClass(const WNDCLASS* wc) { (void)wc; return 1; }

// Window styles
#define WS_OVERLAPPED  0x00000000
#define HWND_MESSAGE   ((HWND)(intptr_t)-3)
#define SW_SHOW        5
#define IDC_ARROW      ((LPCSTR)(intptr_t)32512)

// Message constants
#define WM_USER         0x0400
#define WM_DESTROY      0x0002
#define WM_CLOSE        0x0010
#define WM_CREATE       0x0001
#define WM_KEYDOWN      0x0100
#define WM_KEYUP        0x0101
#define WM_PAINT        0x000F
#define PM_NOREMOVE     0x0000
#define MB_OK           0x00000000
#define MB_ICONEXCLAMATION 0x00000030

struct MSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
};

inline HWND CreateWindowEx(DWORD exStyle, LPCSTR cls, LPCSTR title, DWORD style,
                           int x, int y, int w, int h, HWND parent,
                           HMENU menu, HINSTANCE inst, LPVOID param)
{
    (void)exStyle; (void)cls; (void)title; (void)style;
    (void)x; (void)y; (void)w; (void)h; (void)parent;
    (void)menu; (void)inst; (void)param;
    return (HWND)(intptr_t)1; // dummy non-null
}

inline BOOL GetCursorPos(POINT* pt) { if (pt) { pt->x = 0; pt->y = 0; } return 1; }
inline BOOL InvalidateRect(HWND h, const RECT* r, BOOL erase) { (void)h; (void)r; (void)erase; return 1; }
inline LRESULT SendMessage(HWND h, UINT m, WPARAM w, LPARAM l) { (void)h; (void)m; (void)w; (void)l; return 0; }
inline BOOL DeleteFile(const char* path) { return (remove(path) == 0) ? 1 : 0; }

inline HCURSOR LoadCursor(HINSTANCE inst, LPCSTR name) { (void)inst; (void)name; return nullptr; }
inline HMODULE GetModuleHandle(void* p) { (void)p; return nullptr; }
inline LRESULT DefWindowProc(HWND h, UINT m, WPARAM w, LPARAM l) { (void)h; (void)m; (void)w; (void)l; return 0; }
inline void PostQuitMessage(int code) { (void)code; }
inline BOOL PostMessage(HWND h, UINT m, WPARAM w, LPARAM l) { (void)h; (void)m; (void)w; (void)l; return 1; }
inline BOOL PeekMessage(MSG* msg, HWND h, UINT min, UINT max, UINT remove)
{
    (void)msg; (void)h; (void)min; (void)max; (void)remove;
    return 0; // no messages
}
inline BOOL GetMessage(MSG* msg, HWND h, UINT min, UINT max)
{
    (void)msg; (void)h; (void)min; (void)max;
    return 0;
}
inline BOOL TranslateMessage(const MSG* msg) { (void)msg; return 0; }
inline LRESULT DispatchMessage(const MSG* msg) { (void)msg; return 0; }

inline int MessageBox(HWND h, const char* text, const char* caption, UINT type)
{
    (void)h; (void)type;
    fprintf(stderr, "%s: %s\n", caption ? caption : "Message", text ? text : "");
    return 0;
}

// --------------------------------------------------------------------------
// Multimedia timer stubs
// --------------------------------------------------------------------------
struct TIMECAPS {
    UINT wPeriodMin;
    UINT wPeriodMax;
};

#define TIME_PERIODIC 1

inline MMRESULT timeGetDevCaps(TIMECAPS* caps, UINT size)
{
    (void)size;
    if (caps) { caps->wPeriodMin = 1; caps->wPeriodMax = 1000; }
    return 0;
}
inline MMRESULT timeBeginPeriod(UINT period) { (void)period; return 0; }
inline MMRESULT timeEndPeriod(UINT period) { (void)period; return 0; }
inline MMRESULT timeSetEvent(UINT delay, UINT res, void* cb, DWORD_PTR user, UINT flags)
{
    (void)delay; (void)res; (void)cb; (void)user; (void)flags;
    return 1; // dummy non-zero timer ID
}
inline MMRESULT timeKillEvent(UINT id) { (void)id; return 0; }

#endif // !_WIN32
