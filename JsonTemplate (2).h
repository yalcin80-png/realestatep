#pragma once
#include <vector>
#include <string>
#include <map>

enum class JElementType { Text, Field, Image, Rect, Line, Group };

struct JFontDef {
    std::wstring name;
    std::wstring family;
    float size;
    bool bold = false;
    bool italic = false;
};

struct JElement {
    JElementType type;
    int x, y, w, h;
    std::wstring fontId;
    std::wstring color;
    std::wstring text;
    std::wstring bindKey;
    
    // --- YENİ EKLENEN ÖZELLİKLER ---
    std::string layout = "absolute"; // "absolute" (sabit) veya "vertical" (dikey sıralı)
    int gap = 0;                     // Elemanlar arası boşluk (Vertical ise)
    // -------------------------------

    std::vector<JElement> children; 
};

struct JPageDef {
    int width = 2100;
    int height = 2970;
    std::vector<JElement> elements;
};

struct JTemplate {
    std::map<std::wstring, JFontDef> fonts;
    std::vector<JPageDef> pages;
};