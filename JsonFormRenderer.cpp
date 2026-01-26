#include "stdafx.h"
#include "JsonFormRenderer.h"
#include "TemplateLoader.h"
#include "ThemeConfig.h"
#include <algorithm>

JsonFormRenderer::JsonFormRenderer() {}
JsonFormRenderer::~JsonFormRenderer() {}

// ---------------------------------------------------------
// HEX RENK PARSER
// ---------------------------------------------------------
static bool ParseHexColor(const std::wstring& hex, COLORREF& outColor)
{
    if (hex.size() == 7 && hex[0] == L'#')
    {
        unsigned int r, g, b;
        if (swscanf_s(hex.c_str() + 1, L"%02x%02x%02x", &r, &g, &b) == 3)
        {
            outColor = RGB(r, g, b);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------
// TEMEL RENK ALMA (HEX + İSİMSEL RENKLER)
// ---------------------------------------------------------
COLORREF JsonFormRenderer::GetColor(const std::wstring& name)
{
    COLORREF hexColor;
    if (ParseHexColor(name, hexColor))
        return hexColor;

    if (name == L"Red" || name == L"BrandRed") return Theme::Color_BrandRed;
    if (name == L"Blue" || name == L"HeaderBlue") return Theme::Color_HeaderBg;
    if (name == L"Gray") return RGB(128, 128, 128);
    if (name == L"White") return RGB(255, 255, 255);
    if (name == L"Black") return RGB(0, 0, 0);

    return RGB(0, 0, 0);
}

// ---------------------------------------------------------
// PENCERE RENDER
// ---------------------------------------------------------
bool JsonFormRenderer::Render(IDrawContext& ctx, const CString& jsonPath,
    const std::map<CString, CString>& data)
{
    TemplateLoader loader;
    JTemplate tmpl = loader.Load(jsonPath);

    if (tmpl.pages.empty()) return false;
    if (tmpl.fonts.find(L"Default") == tmpl.fonts.end())
        tmpl.fonts[L"Default"] = FontDesc(L"Arial", 10.f, false);

    for (auto& page : tmpl.pages)
    {
        ctx.SetLogicalPageSize(page.width, page.height);
        ctx.FillRect(0, 0, page.width, page.height, Theme::Color_PageBg);

        for (auto& el : page.elements)
        {
            RenderElement(ctx, el, 0, 0, data, tmpl.fonts);
        }
    }

    return true;
}

// ---------------------------------------------------------
// ELEMENT ÇİZİMİ
// ---------------------------------------------------------
int JsonFormRenderer::RenderElement(IDrawContext& ctx, const JElement& el,
    int offsetX, int offsetY,
    const std::map<CString, CString>& data,
    const std::map<std::wstring, FontDesc>& fonts)
{
    int absX = offsetX + el.x;
    int absY = offsetY + el.y;
    int finalHeight = el.h;

    // ---------------------------------------------------------
    // GRUP
    // ---------------------------------------------------------
    if (el.type == JElementType::Group)
    {
        if (el.border)
        {
            ctx.SetPen(GetColor(el.color), 1.0f);
            ctx.DrawRect(absX, absY, el.w, el.h > 0 ? el.h : 50, false);
        }

        // VERTICAL LAYOUT FIXED
        if (el.layout == JLayoutMode::Vertical)
        {
            int currentY = 0;

            for (auto& child : el.children)
            {
                int usedHeight =
                    RenderElement(ctx, child, absX, absY + currentY, data, fonts);

                currentY += child.y + usedHeight + el.gap;
            }

            if (finalHeight == 0)
                finalHeight = currentY;
        }
        else
        {
            // ABSOLUTE LAYOUT
            int maxBottom = 0;

            for (auto& child : el.children)
            {
                int h = RenderElement(ctx, child, absX, absY, data, fonts);
                int bottom = child.y + h;
                if (bottom > maxBottom)
                    maxBottom = bottom;
            }

            if (finalHeight == 0)
                finalHeight = maxBottom;
        }

        return finalHeight;
    }

    // ---------------------------------------------------------
    // FONT SEÇİMİ
    // ---------------------------------------------------------
    FontDesc fontToUse = fonts.count(el.fontId)
        ? fonts.at(el.fontId)
        : fonts.at(L"Default");
    ctx.SetFont(fontToUse);

    COLORREF bg = GetColor(el.color);
    COLORREF fg = GetColor(el.textColor.empty() ? L"Black" : el.textColor);

    ctx.SetPen(bg, 1.0f);
    ctx.SetBrush(bg);

    // ---------------------------------------------------------
    // TEXT
    // ---------------------------------------------------------
    if (el.type == JElementType::Text)
    {
        if (el.filled)
            ctx.FillRect(absX, absY, el.w, el.h, bg);

        ctx.SetBrush(fg);

        UINT align = 0;
        if (el.multiline)
        {
            align |= DT_WORDBREAK | DT_TOP;
        }
        else
        {
            align |= DT_SINGLELINE | DT_VCENTER;
        }

        if (el.align == JAlign::Center) align |= DT_CENTER;
        else if (el.align == JAlign::Right) align |= DT_RIGHT;
        else align |= DT_LEFT;

        CString textToDraw = el.text.c_str();
        
        // Bind key varsa ve veri varsa, veriyi kullan
        if (!el.bindKey.empty())
        {
            auto it = data.find(el.bindKey.c_str());
            if (it != data.end() && !it->second.IsEmpty())
                textToDraw = it->second;
        }

        // Sadece text varsa çiz
        if (!textToDraw.IsEmpty())
        {
            ctx.DrawTextW(textToDraw.GetString(), absX, absY, el.w, el.h, align);
        }
        
        return finalHeight;
    }

    // ---------------------------------------------------------
    // FIELD
    // ---------------------------------------------------------
    if (el.type == JElementType::Field)
    {
        CString val = L"";
        
        // Veri lookup - Case insensitive ve null check
        if (!el.bindKey.empty())
        {
            auto it = data.find(el.bindKey.c_str());
            if (it != data.end() && !it->second.IsEmpty()) {
                val = it->second;
            }
        }

        // Background
        ctx.FillRect(absX, absY, el.w, el.h, Theme::Color_BoxBg);

        // Border
        ctx.SetPen(Theme::Color_BorderLight, 1.0f);
        ctx.DrawRect(absX, absY, el.w, el.h, true);

        int labelHeight = 20;
        int textOffsetY = 0;
        int valueHeight = el.h;

        // LABEL
        if (!el.text.empty())
        {
            ctx.SetFont(fonts.count(L"Label")
                ? fonts.at(L"Label")
                : fontToUse);

            ctx.SetBrush(Theme::Color_TextLabel);
            ctx.DrawTextW(el.text.c_str(),
                absX + 8, absY + 4,
                el.w - 16, labelHeight,
                DT_LEFT | DT_TOP);

            textOffsetY = labelHeight;
            valueHeight = el.h - labelHeight;
        }

        // VALUE - Sadece değer varsa göster
        if (!val.IsEmpty())
        {
            ctx.SetFont(fontToUse);
            ctx.SetBrush(fg);
            ctx.DrawTextW(val.GetString(),
                absX + 8,
                absY + textOffsetY,
                el.w - 16,
                valueHeight,
                DT_LEFT | DT_VCENTER);
        }

        return el.h;
    }

    // ---------------------------------------------------------
    // LINE
    // ---------------------------------------------------------
    if (el.type == JElementType::Line)
    {
        ctx.DrawLine(absX, absY, absX + el.w, absY + el.h);
        return finalHeight;
    }

    // ---------------------------------------------------------
    // RECTANGLE
    // ---------------------------------------------------------
    if (el.type == JElementType::Rect)
    {
        if (el.filled)
            ctx.FillRect(absX, absY, el.w, el.h, bg);
        else
            ctx.DrawRect(absX, absY, el.w, el.h, true);

        return finalHeight;
    }

    // ---------------------------------------------------------
    // IMAGE
    // ---------------------------------------------------------
    if (el.type == JElementType::Image)
    {
        if (!el.imagePath.empty())
            ctx.DrawImage(absX, absY, el.w, el.h, el.imagePath);
        return finalHeight;
    }
    // ---------------------------------------------------------
// CHECKBOX
// ---------------------------------------------------------
    if (el.type == JElementType::Checkbox)
    {
        // Checkbox kutu boyutu
        const int boxSize = 26;
        const int padding = 4;

        // Değer okumak - Improved null/empty check
        bool checked = false;
        if (!el.bindKey.empty())
        {
            auto it = data.find(el.bindKey.c_str());
            if (it != data.end() && !it->second.IsEmpty())
            {
                CString v = it->second;
                v.MakeLower();
                v.Trim(); // Whitespace temizle
                if (v == L"1" || v == L"true" || v == L"x" || v == L"yes" || v == L"evet")
                    checked = true;
            }
        }

        // Kutu çiz
        ctx.SetPen(GetColor(el.color), 1.5f);
        ctx.SetBrush(RGB(255, 255, 255));
        ctx.DrawRect(absX, absY, boxSize, boxSize, false);

        // Eğer işaretli ise ✔ çiz
        if (checked)
        {
            ctx.SetPen(GetColor(el.color), 3.0f);

            // Çapraz çizim (simple check)
            ctx.DrawLine(absX + 5, absY + boxSize / 2,
                absX + boxSize / 2, absY + boxSize - 5);

            ctx.DrawLine(absX + boxSize / 2, absY + boxSize - 5,
                absX + boxSize - 5, absY + 5);
        }

        // Label çizimi
        if (!el.text.empty())
        {
            ctx.SetFont(fontToUse);
            ctx.SetBrush(GetColor(el.textColor.empty() ? L"Black" : el.textColor));

            int textOffsetX = absX + boxSize + padding;
            int textOffsetY = absY;

            ctx.DrawTextW(el.text.c_str(),
                textOffsetX, textOffsetY,
                el.w - (boxSize + padding),
                boxSize,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }

        // Eleman yüksekliği = kutu yüksekliği
        return boxSize;
    }

    return finalHeight;
}
