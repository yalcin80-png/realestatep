#pragma once

#include "wxx_cstring.h"
#include <set>
#include <vector>

// A small helper for JSON array string storage without ATL/MFC.
class FeaturesCodec2
{
public:
    static Win32xx::CString EncodeJsonArray(const std::vector<Win32xx::CString>& items);
    static Win32xx::CString EncodeJsonArray(const std::set<Win32xx::CString>& items);
    static std::vector<Win32xx::CString> DecodeJsonArray(const Win32xx::CString& jsonArray);
    static std::set<Win32xx::CString> DecodeToSet(const Win32xx::CString& jsonArray);
    static Win32xx::CString NormalizeSingle(const Win32xx::CString& value);
};
