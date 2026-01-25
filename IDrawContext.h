#pragma once

#ifndef IDRAWCONTEXT_H
#define IDRAWCONTEXT_H

#include "stdafx.h" // Eğer projenizde kullanılıyorsa
#include <string>
#include <gdiplus.h> // COLORREF ve UINT için gerekli olabilir, ancak standart WinAPI de yeterli.
// (ListingPageLayout.cpp dosyasında GDI+ başlığı zaten vardı.)

// ------------------------------------------------------------
// WinAPI Metin Hizalama ve Düzenleme Bayrakları (IDrawContext'te kullanılacak)
// Bu bayraklar GDI'dan alınmıştır, tüm uygulamalar (GDI+, Haru) bunları desteklemelidir.
// ------------------------------------------------------------
#ifndef DT_TOP
#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_WORDBREAK        0x00000010 // Birden fazla satıra böl
#define DT_SINGLELINE       0x00000020
#define DT_CALCRECT         0x00000400
// ... İhtiyaç duyulan diğer DT_ bayrakları eklenebilir.
#endif




// ------------------------------------------------------------
// Renk yapısı (float RGBA)
// ------------------------------------------------------------
struct Color4f
{
    float r, g, b, a;

    Color4f() : r(0), g(0), b(0), a(1.0f) {}

    Color4f(float _r, float _g, float _b, float _a = 1.0f)
        : r(_r), g(_g), b(_b), a(_a) {
    }

    // KOLAYLIK: COLORREF'ten dönüştürücü (ListingPageLayout::FromColorRef'e benzer)
    Color4f(COLORREF cr, float _a = 1.0f)
        : r(GetRValue(cr) / 255.0f),
        g(GetGValue(cr) / 255.0f),
        b(GetBValue(cr) / 255.0f),
        a(_a) {
    }
};

// ------------------------------------------------------------
// Font tanımı
// ------------------------------------------------------------
struct FontDesc
{
    std::wstring family;    // Ör: L"Arial"
    float        sizePt;    // 10.0f, 12.0f vb.
    bool         bold;
    bool         italic;
    bool         underline; // Yeni eklendi (Gerekli olabilir)

    FontDesc(const std::wstring& fam = L"Arial",
        float sz = 10.0f,
        bool b = false,
        bool i = false,
        bool u = false)
        : family(fam), sizePt(sz), bold(b), italic(i), underline(u)
    {
    }
};

class IDrawContext
{
public:
    virtual ~IDrawContext() {}

    // Belge Yönetimi
    virtual bool BeginDocument(const CString& filePath) = 0;
    virtual bool EndDocument() = 0;
    virtual void SetLogicalPageSize(double width, double height) = 0;

    // --- YENİ RENK SİSTEMİ (COLORREF) ---
    virtual void SetPen(COLORREF color, float width = 1.0f) = 0;
    virtual void SetBrush(COLORREF color) = 0;
    virtual void SetFont(const FontDesc& font) = 0;

    // Çizim
    virtual void DrawLine(double x1, double y1, double x2, double y2) = 0;
    virtual void DrawRect(double x, double y, double w, double h, bool frameOnly) = 0;

    // Kolaylık: Tek komutla kutu boyama
    virtual void FillRect(double x, double y, double w, double h, COLORREF color) = 0;

    virtual void DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags) = 0;
    virtual void DrawImage(double x, double y, double w, double h, const std::wstring& path) = 0;
};
#endif // IDRAWCONTEXT_H