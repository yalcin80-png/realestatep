#pragma once
#ifndef CAIRODRAWCONTEXT_H
#define CAIRODRAWCONTEXT_H
#pragma once

#include "IDrawContext.h"
#include <string>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-win32.h>

class CairoDrawContext : public IDrawContext
{
public:
    CairoDrawContext();
    virtual ~CairoDrawContext();

    // IDrawContext Arayüzü
    virtual bool BeginDocument(const CString& filePath) override;
    virtual bool EndDocument() override;
    virtual void SetLogicalPageSize(double width, double height) override;

    // YENİ RENK SİSTEMİ (COLORREF)
    virtual void SetPen(COLORREF color, float width = 1.0f) override;
    virtual void SetBrush(COLORREF color) override;
    virtual void SetFont(const FontDesc& font) override;

    // Çizim
    virtual void DrawLine(double x1, double y1, double x2, double y2) override;
    virtual void DrawRect(double x, double y, double w, double h, bool frameOnly) override;
    virtual void FillRect(double x, double y, double w, double h, COLORREF color) override;

    virtual void DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags) override;
    virtual void DrawImage(double x, double y, double w, double h, const std::wstring& filePath) override;

private:
    void BeginPage(double w, double h);
    void EndPage();

    // Yardımcılar
    std::string WStringToUTF8(const std::wstring& w) const;
    void SetSourceColor(COLORREF c); // Ortak renk ayarlayıcı

private:
    cairo_surface_t* m_surface;
    cairo_t* m_cr;

    CString m_filePath;
    double  m_width;
    double  m_height;

    // Aktif Fırça ve Kalem renklerini saklamamız gerekebilir
    COLORREF m_penColor;
    float    m_penWidth;
    COLORREF m_brushColor;

    float    m_fontSize;
};
#endif // CAIRODRAWCONTEXT_H
