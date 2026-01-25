#include "stdafx.h"
#include "FeaturesCodec.h"

#include <windows.h>
#include <nlohmann/json.hpp>

#include <algorithm>

using json = nlohmann::json;

// ------------------------------------------------------------
// UTF-16 (CString) <-> UTF-8 (std::string) dönüşüm
// ------------------------------------------------------------
std::string FeaturesCodec::Utf8FromCString(const CString& s)
{
#ifdef UNICODE
    const wchar_t* w = s.c_str();
    if (!w) return {};

    int needed = ::WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};

    std::string out;
    out.resize(static_cast<size_t>(needed - 1));
    ::WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), needed, nullptr, nullptr);
    return out;
#else
    const char* a = s.c_str();
    if (!a) return {};

    int wlen = ::MultiByteToWideChar(CP_ACP, 0, a, -1, nullptr, 0);
    if (wlen <= 0) return {};
    std::wstring w;
    w.resize(static_cast<size_t>(wlen - 1));
    ::MultiByteToWideChar(CP_ACP, 0, a, -1, w.data(), wlen);

    int needed = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};
    std::string out;
    out.resize(static_cast<size_t>(needed - 1));
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), needed, nullptr, nullptr);
    return out;
#endif
}

CString FeaturesCodec::CStringFromUtf8(const std::string& u8)
{
    if (u8.empty()) return CString();

#ifdef UNICODE
    int needed = ::MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, nullptr, 0);
    if (needed <= 0) return CString();

    std::wstring w;
    w.resize(static_cast<size_t>(needed - 1));
    ::MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, w.data(), needed);

    return CString(w.c_str());
#else
    // UTF8 -> Wide -> ACP
    int wlen = ::MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return CString();

    std::wstring w;
    w.resize(static_cast<size_t>(wlen - 1));
    ::MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, w.data(), wlen);

    int alen = ::WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (alen <= 0) return CString();

    std::string a;
    a.resize(static_cast<size_t>(alen - 1));
    ::WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, a.data(), alen, nullptr, nullptr);

    return CString(a.c_str());
#endif
}

// ------------------------------------------------------------
// Encode
// ------------------------------------------------------------
CString FeaturesCodec::EncodeJsonArray(const std::vector<CString>& items)
{
    json arr = json::array();
    for (const auto& it : items)
    {
        std::string u8 = Utf8FromCString(it);
        if (!u8.empty()) arr.push_back(u8);
    }
    return CStringFromUtf8(arr.dump());
}

CString FeaturesCodec::EncodeJsonArray(const std::set<CString>& items)
{
    std::vector<CString> v;
    v.reserve(items.size());
    for (const auto& s : items) v.push_back(s);
    return EncodeJsonArray(v);
}

// ------------------------------------------------------------
// Decode (JSON array OR CSV)
// ------------------------------------------------------------
static inline void SplitCsvToSet(const CString& s, std::set<CString>& out)
{
    CString tmp = s;
    tmp.Trim();
    if (tmp.IsEmpty()) return;

    // Normalize separators to ';'
    CString norm = tmp;
    norm.Replace(_T(","), _T(";"));
    norm.Replace(_T("|"), _T(";"));

    int cur = 0;
    while (cur >= 0)
    {
        int pos = norm.Find(_T(';'), cur);
        CString token;
        if (pos < 0)
        {
            token = norm.Mid(cur);
            cur = -1;
        }
        else
        {
            token = norm.Mid(cur, pos - cur);
            cur = pos + 1;
        }
        token.Trim();
        if (!token.IsEmpty()) out.insert(token);
    }
}

std::set<CString> FeaturesCodec::DecodeToSet(const CString& stored)
{
    std::set<CString> out;

    CString s = stored;
    s.Trim();
    if (s.IsEmpty()) return out;

    // Try JSON first
    std::string u8 = Utf8FromCString(s);
    if (!u8.empty())
    {
        try
        {
            json j = json::parse(u8);
            if (j.is_array())
            {
                for (auto& v : j)
                {
                    if (v.is_string())
                        out.insert(CStringFromUtf8(v.get<std::string>()));
                }
                return out;
            }
            if (j.is_string())
            {
                out.insert(CStringFromUtf8(j.get<std::string>()));
                return out;
            }
        }
        catch (...) {}
    }

    // Fallback CSV
    SplitCsvToSet(s, out);
    return out;
}

CString FeaturesCodec::NormalizeSingle(const CString& value)
{
    CString v = value;
    v.Trim();
    if (v.IsEmpty()) return CString();

    // If it's JSON array, pick first
    std::set<CString> setv = DecodeToSet(v);
    if (!setv.empty())
        return *setv.begin();

    // Otherwise just return trimmed
    return v;
}
