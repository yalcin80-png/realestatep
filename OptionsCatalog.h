#pragma once
#include "stdafx.h"
#include "wxx_cstring.h"
#include <vector>
#include <map>

class OptionsCatalog
{
public:
    enum class Kind { None, Multi, Single };

    struct Entry {
        Kind kind = Kind::None;
        std::vector<Win32xx::CString> options;
    };

    static const Entry& Get(const Win32xx::CString& tableName, const Win32xx::CString& fieldName);

private:
    static const std::map<std::pair<Win32xx::CString, Win32xx::CString>, Entry>& Catalog();
    static const Entry& Empty();
};
