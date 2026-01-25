#pragma once
#ifndef JSONFORMRENDERER_H
#define JSONFORMRENDERER_H

#include "JsonTemplate.h"
#include "IDrawContext.h"
#include <map>

class JsonFormRenderer {
public:
    JsonFormRenderer();
    virtual ~JsonFormRenderer();

    // Ana Çizim Fonksiyonu
    bool Render(IDrawContext& ctx, const CString& jsonPath, const std::map<CString, CString>& data);

private:
    // Recursive Çizim (Dönüş değeri: Çizilen yüksekliği verir)
    int RenderElement(IDrawContext& ctx, const JElement& el, int offsetX, int offsetY,
        const std::map<CString, CString>& data,
        const std::map<std::wstring, FontDesc>& fonts);

    // Yardımcı: Renk Çevirici
    COLORREF GetColor(const std::wstring& colorName);
};

#endif