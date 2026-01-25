
#ifndef HSPLITHELPERS_H
#define HSPLITHELPERS_H


#pragma once
#include "stdafx.h"
#include <vector>
#include <map>

// fields ? müþteri / mülk / notlar þeklinde ayrýlýr
void SplitDataToSections(
    const CString& tableName,
    const std::vector<std::pair<std::wstring,std::wstring>>& fields,

    std::vector<std::pair<std::wstring,std::wstring>>& customer,
    std::vector<std::pair<std::wstring,std::wstring>>& property,
    std::wstring& notes
);

// Tablo tahmini (Home, Villa, Field vb.)
CString GuessTableFromFields(
    const std::vector<std::pair<std::wstring,std::wstring>>& fields
);


#endif // HSPLITHELPERS_H