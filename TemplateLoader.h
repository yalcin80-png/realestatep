
#pragma once

#ifndef WIN32XX_TEMPLOAD_H
#define WIN32XX_TEMPLOAD_H
#include "stdafx.h"

#include "JsonTemplate.h"
#include <fstream>
#include <nlohmann/json.hpp>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
#include "ThemeConfig.h"
#include "json.hpp"

using namespace Gdiplus;
using namespace std;
using json = nlohmann::json;




using json = nlohmann::json;

class TemplateLoader {
public:
    TemplateLoader();
    virtual ~TemplateLoader();

    // JSON dosyasını okur ve JTemplate yapısına çevirir
    JTemplate Load(const CString& filePath);
    JElement ParseElement(const json& j);

private:
    // İç içe grupları okumak için recursive fonksiyon
    //JElement ParseElement(const nlohmann::json& j);

    // Yardımcılar
    COLORREF ParseColor(const std::string& colorStr);
}; 

#endif  // WIN32XX_TEMPLOAD_H