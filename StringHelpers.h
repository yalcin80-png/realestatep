

#ifndef ISTRINGHELPER_H
#define ISTRINGHELPER_H

#pragma once
#include <string>
#include <windows.h>
#include "wxx_cstring.h" // Win32++ CString için

namespace FormEngine {

    // std::string (UTF-8) -> CString (Wide/Unicode)
    inline CString ToCString(const std::string& str) {
        if (str.empty()) return CString(L"");
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return CString(wstrTo.c_str());
    }

    // CString (Wide) -> std::string (UTF-8)
    inline std::string ToUtf8(const CString& str) {
        if (str.IsEmpty()) return "";
        std::wstring wstr = str.GetString();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    // std::string (UTF-8/ANSI) -> std::wstring (GÜVENLİ)
    inline std::wstring ToWString(const std::string& str) {
        if (str.empty()) return std::wstring();

        auto convert = [&](UINT cp, DWORD flags) -> std::wstring {
            int n = MultiByteToWideChar(cp, flags, str.data(), (int)str.size(), nullptr, 0);
            if (n <= 0) return std::wstring();                // <-- kritik guard
            std::wstring out(n, L'\0');
            MultiByteToWideChar(cp, flags, str.data(), (int)str.size(), out.data(), n);
            return out;
            };

        // 1) Önce “gerçek UTF-8” diye dene (hatalı UTF-8’i yakalasın)
        std::wstring w = convert(CP_UTF8, MB_ERR_INVALID_CHARS);
        if (!w.empty()) return w;

        // 2) UTF-8 değilse: ANSI/aktif codepage fallback
        w = convert(CP_ACP, 0);
        if (!w.empty()) return w;

        // 3) Son çare: flags=0 ile UTF-8 dene (bazı durumlarda replacement üretir)
        w = convert(CP_UTF8, 0);
        return w;
    }

    //// std::string (UTF-8/ANSI) -> CString (Wide)
    //inline CString ToCString(const std::string& str) {
    //    std::wstring w = ToWString(str);
    //    return CString(w.c_str());
    //}

    //inline CString GetExePath() {
    //    TCHAR buffer[MAX_PATH];
    //    ::GetModuleFileName(NULL, buffer, MAX_PATH);
    //    CString path(buffer);
    //    int pos = path.ReverseFind('\\');
    //    if (pos != -1) path = path.Left(pos);
    //    return path;
    //}

    inline CString GetExePath()
    {
        TCHAR buffer[MAX_PATH];
        // 1. Windows API'den Exe'nin tam yolunu al (C:\Klasor\Program.exe)
        ::GetModuleFileName(NULL, buffer, MAX_PATH);

        CString path = buffer;

        // 2. Sondaki dosya ismini (Program.exe) kes at, sadece klasör kalsın
        int pos = path.ReverseFind(_T('\\'));
        if (pos != -1) {
            path = path.Left(pos);
        }
        return path;
    }
}

#endif  // WIN32XX_DOC_H