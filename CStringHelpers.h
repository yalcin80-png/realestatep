

#ifndef STRINGHELPER_H
#define STRINGHELPER_H

// --- CStringHelpers.h (GENEL YARDIMCI) ---
#pragma once
#include "stdafx.h"
#include "resource.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>

// CString (Wide) kullanarak UTC zaman damgası üretme
inline CString iso_now_utc_cstring() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_s(&tm, &t);
    // TCHAR kullanarak Win32++ uyumlu formatlama
    TCHAR buf[32]{ 0 };
    _tcsftime(buf, sizeof(buf) / sizeof(TCHAR), _T("%Y-%m-%dT%H:%M:%SZ"), &tm);
    return CString(buf);
}

// KRİTİK: CString (Wide/UTF-16) -> std::string (UTF-8) - JSON ve URL için
inline std::string CStringToUtf8(const CString& wideStr)
{
    if (wideStr.IsEmpty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &out[0], len - 1, nullptr, nullptr);
    return out;
}

// KRİTİK: std::string (UTF-8) -> CString (Wide/UTF-16) - API cevabı ve DB verisi için
inline CString Utf8ToCString(const std::string& utf8Str)
{
    if (utf8Str.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    CString out;
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, out.GetBuffer(len - 1), len - 1);
    out.ReleaseBuffer();
    return out;
}

// Merkezi Hata Loglama (Artık CString kullanıyor)
inline void LogCriticalError(const CString& component, const CString& message)
{
    CString fullMsg;
    fullMsg.Format(_T("[%s HATA] %s\n"), component.GetString(), message.GetString());
    // TRACE makrosu Win32++'ta genellikle OutputDebugString kullanır.
    OutputDebugString(fullMsg); 
}


#endif  