#pragma once
#include "stdafx.h"
#include "SchemaManager.h"
#include <map>
#include <string>

namespace HybridDlgHelpers
{
    inline std::string Utf8FromCString(const Win32xx::CString& s)
    {
        const wchar_t* ws = s.c_str();
        if (!ws) return {};
        int wlen = (int)wcslen(ws);
        if (wlen <= 0) return {};
        int len = ::WideCharToMultiByte(CP_UTF8, 0, ws, wlen, nullptr, 0, nullptr, nullptr);
        std::string out;
        out.resize(len);
        ::WideCharToMultiByte(CP_UTF8, 0, ws, wlen, out.data(), len, nullptr, nullptr);
        return out;
    }

    // data tipi T: .SetAttr(std::string key, CString value) ve Attributes alanı olmalı.
    template<typename T>
    inline void ApplyExtraAttributes(const Win32xx::CString& tableName,
                                    const std::map<Win32xx::CString, Win32xx::CString>& extraMap,
                                    T& data)
    {
        const auto& schema = SchemaManager::Get().GetSchema(tableName);
        for (const auto& f : schema)
        {
            if (!f.isAttribute) continue;
            auto it = extraMap.find(f.dbName);
            if (it == extraMap.end()) continue;
            data.SetAttr(Utf8FromCString(f.dbName), it->second);
        }
    }
}
