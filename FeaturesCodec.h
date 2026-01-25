#pragma once

#include "stdafx.h"

#include <set>
#include <vector>

// ============================================================================
//  FeaturesCodec - Home "Özellikler" alanlarını (multi-select) saklama/okuma
//  ----------------------------------------------------------------------------
//  - DB alanları: Facades, FeaturesInterior, FeaturesExterior, FeaturesNeighborhood,
//    FeaturesTransport, FeaturesView, HousingType, FeaturesAccessibility
//  - Saklama formatı: JSON array string (önerilen).
//  - Geriye dönük uyumluluk: CSV/";"/"|" ayracı da decode edilir.
//  - ATL/MFC (CT2A/CA2T) KULLANMAZ. Tamamen Win32++/WinAPI ile UTF dönüşümü yapar.
// ============================================================================

class FeaturesCodec
{
public:
    // JSON array string olarak encode eder: ["A","B"]
    static CString EncodeJsonArray(const std::vector<CString>& items);
    static CString EncodeJsonArray(const std::set<CString>& items);

    // JSON array veya CSV (",", ";", "|") decode eder.
    static std::set<CString> DecodeToSet(const CString& stored);

    // Tek seçimli alanlar için yardımcı ("A" veya ["A"] veya "A;B" -> "A")
    static CString NormalizeSingle(const CString& value);

private:
    static std::string Utf8FromCString(const CString& s);
    static CString CStringFromUtf8(const std::string& u8);
};
