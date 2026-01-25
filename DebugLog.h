#pragma once

// Simple debug logger for Win32++ projects.
// Writes to OutputDebugStringW (view with DebugView / Visual Studio Output window).

#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

inline void DebugLogW(const wchar_t* fmt, ...)
{
    if (!fmt) return;

    wchar_t msg[2048];
    msg[0] = L'\0';

    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(msg, _countof(msg), _TRUNCATE, fmt, ap);
    va_end(ap);

    OutputDebugStringW(msg);
    OutputDebugStringW(L"\r\n");
}

#endif // DEBUGLOG_H
