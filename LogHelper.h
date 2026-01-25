

#ifndef LOGHELPER_H 
#define LOGHELPER_H

// --- LogHelper.h ---

#pragma once
#include <string>
#include <sstream>

// Kritik hatalar için Win32++ TRACE (veya özel loglama fonksiyonunuz) kullanılır.
inline void LogCriticalError(const std::string& component, const std::string& message)
{
    std::ostringstream oss;
    oss << "[" << component << " HATA] " << message << "\n";
    // Win32++ TRACE yerine standart Win32 Debug çıkışı
    OutputDebugStringA(oss.str().c_str());
}

// Güvenlik uyarısı için
inline void LogSecurityWarning(const std::string& component, const std::string& message)
{
    std::ostringstream oss;
    oss << "[" << component << " GÜVENLİK UYARISI] " << message << "\n";
    OutputDebugStringA(oss.str().c_str());
}

#endif  // WIN32XX_DOC_H