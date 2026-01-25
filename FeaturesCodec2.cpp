#include "stdafx.h"
#include "FeaturesCodec2.h"

#include <windows.h>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace
{
    std::string Utf8FromCString(const Win32xx::CString& ws)
    {
#ifdef UNICODE
        const wchar_t* w = ws.c_str();
        if (!w) return {};
        int needed = ::WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 0) return {};
        std::string out; out.resize(static_cast<size_t>(needed - 1));
        ::WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), needed, nullptr, nullptr);
        return out;
#else
        const char* a = ws.c_str();
        if (!a) return {};
        // ACP -> UTF8 via Wide
        int wlen = ::MultiByteToWideChar(CP_ACP, 0, a, -1, nullptr, 0);
        if (wlen <= 0) return {};
        std::wstring w; w.resize(static_cast<size_t>(wlen - 1));
        ::MultiByteToWideChar(CP_ACP, 0, a, -1, w.data(), wlen);
        int needed = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 0) return {};
        std::string out; out.resize(static_cast<size_t>(needed - 1));
        ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), needed, nullptr, nullptr);
        return out;
#endif
    }

    Win32xx::CString CStringFromUtf8(const std::string& s)
    {
        if (s.empty()) return Win32xx::CString();
#ifdef UNICODE
        int needed = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        if (needed <= 0) return Win32xx::CString();
        std::wstring w; w.resize(static_cast<size_t>(needed - 1));
        ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), needed);
        return Win32xx::CString(w.c_str());
#else
        // UTF8 -> Wide -> ACP
        int wlen = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        if (wlen <= 0) return Win32xx::CString();
        std::wstring w; w.resize(static_cast<size_t>(wlen - 1));
        ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), wlen);
        int alen = ::WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (alen <= 0) return Win32xx::CString();
        std::string a; a.resize(static_cast<size_t>(alen - 1));
        ::WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, a.data(), alen, nullptr, nullptr);
        return Win32xx::CString(a.c_str());
#endif
    }
}

Win32xx::CString FeaturesCodec2::EncodeJsonArray(const std::vector<Win32xx::CString>& items)
{
    json arr = json::array();
    for (const auto& it : items)
    {
        std::string u8 = Utf8FromCString(it);
        if (!u8.empty()) arr.push_back(u8);
    }
    return CStringFromUtf8(arr.dump());
}

Win32xx::CString FeaturesCodec2::EncodeJsonArray(const std::set<Win32xx::CString>& items)
{
    std::vector<Win32xx::CString> v(items.begin(), items.end());
    return EncodeJsonArray(v);
}

std::vector<Win32xx::CString> FeaturesCodec2::DecodeJsonArray(const Win32xx::CString& jsonArray)
{
    std::vector<Win32xx::CString> out;
    std::string u8 = Utf8FromCString(jsonArray);
    if (u8.empty()) return out;
    try {
        json j = json::parse(u8);
        if (!j.is_array()) return out;
        for (auto& v : j) if (v.is_string()) out.push_back(CStringFromUtf8(v.get<std::string>()));
    } catch (...) { }
    return out;
}

std::set<Win32xx::CString> FeaturesCodec2::DecodeToSet(const Win32xx::CString& jsonArray)
{
    std::set<Win32xx::CString> s;
    for (auto& v : DecodeJsonArray(jsonArray)) s.insert(v);
    return s;
}

Win32xx::CString FeaturesCodec2::NormalizeSingle(const Win32xx::CString& value)
{
    Win32xx::CString s = value;
    s.Trim();
    return s;
}
