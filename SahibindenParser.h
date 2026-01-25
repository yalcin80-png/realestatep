#pragma once
#include "stdafx.h"
#include <map>
#include <vector>
#include <string>

class SahibindenParser
{
public:
    // Dışarıya açık tek fonksiyon
    static std::map<CString, CString> Parse(const CString& rawText);


    static bool ParseFromHtml(const CString& html, std::map<CString, CString>& out);


private:
    // Alan tanımı: canonical key + label eşanlamlıları + çok satırlı mı
    struct FieldDef
    {
        CString canonical;            // Map'te kullanılacak key
        std::vector<CString> labels;  // Metinde görebileceğimiz etiketler
        bool multiLine;               // Değer alt satırlarda devam edebilir mi?
    };

    // İç yardımcılar
    static void CleanText(CString& text);
    static std::vector<CString> SplitToLines(const CString& text);
    static const std::vector<FieldDef>& GetFieldDefs();

    // Satır özel analizcileri
    static bool TryParsePrice(const CString& line, std::map<CString, CString>& out);
    static bool TryParseAddress(const CString& line, std::map<CString, CString>& out);

    // Label ile başlıyor mu? (key + value ayrıştırır)
    static bool StartsWithLabel(const CString& line, const CString& label, CString& valuePart);


    // GA tracking json parse yardımcıları
    static bool ExtractGaJsonFromHtml(const CString& html, CString& gaJson);
    static bool ParseGaJson(const CString& gaJson, std::map<CString, CString>& out);
    static void ParseKeyValueArray(const CString& section,
        bool lastTokenIsIndex,
        std::map<CString, CString>& out);



};
