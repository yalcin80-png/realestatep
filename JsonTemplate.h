#pragma once

#ifndef JSONTEMP_H
#define JSONTEMP_H

#include "stdafx.h"
#include <vector>
#include <string>
#include <map>
#include "IDrawContext.h"

// Element tipleri
enum class JElementType {
    Text,
    Field,
    Image,
    Rect,
    Line,
    Group,
    Checkbox,   // ✓ yeni
    Section,    // ✓ yeni
    Row         // ✓ yeni
};

// Hizalama
enum class JAlign { Left, Center, Right };

// Layout
enum class JLayoutMode { Absolute, Vertical };

// Element Modeli
struct JElement {
    JElementType type = JElementType::Text;

    int x = 0, y = 0, w = 0, h = 0;

    // Stil
    std::wstring fontId = L"Default";
    std::wstring color = L"Black";
    std::wstring textColor;     // ✓ yeni
    JAlign align = JAlign::Left;
    bool border = false;
    bool filled = false;

    // İçerik
    std::wstring text;
    std::wstring bindKey;
    std::wstring imagePath;

    // Çok satır
    bool multiline = false;     // ✓ yeni

    // Layout
    JLayoutMode layout = JLayoutMode::Absolute;
    int gap = 0;

    // SECTION
    std::wstring title;         // ✓ yeni: Section başlığı

    // Children
    std::vector<JElement> children;
};

// Sayfa
struct JPageDef {
    int width = 2100;
    int height = 2970;
    std::vector<JElement> elements;
};

// Template
struct JTemplate {
    std::map<std::wstring, FontDesc> fonts;
    std::map<std::wstring, COLORREF> colors;
    std::vector<JPageDef> pages;
};

#endif
