#ifndef DOC_GDIPLUSDRAWCONTEXT_H
#define DOC_GDIPLUSDRAWCONTEXT_H
#pragma once

#include "IDrawContext.h"
#include <gdiplus.h>
#include <windows.h>

// GDI+ Kütüphanesini Linkle
#pragma comment (lib,"Gdiplus.lib")

class GdiPlusDrawContext : public IDrawContext
{
public:
    GdiPlusDrawContext();
    virtual ~GdiPlusDrawContext();

    // ---------------------------------------------------------
    // ÖZEL BAŞLATICI (Ekran/Yazıcı İçin)
    // ---------------------------------------------------------
    // PDF'ten farklı olarak GDI+ bir HDC (Handle to Device Context) ister.
    bool Begin(HDC hdc);
    void End();

    // ---------------------------------------------------------
    // IDrawContext ARAYÜZÜNDEN GELENLER
    // ---------------------------------------------------------

    // GDI+ genelde dosyaya değil ekrana çizer, ama arayüz gereği bunları tanımlıyoruz.
    virtual bool BeginDocument(const CString& filePath) override;
    virtual bool EndDocument() override;
    virtual void SetLogicalPageSize(double width, double height) override;

    // Renk ve Araçlar
    virtual void SetPen(COLORREF color, float width = 1.0f) override;
    virtual void SetBrush(COLORREF color) override;
    virtual void SetFont(const FontDesc& font) override;

    // Çizim
    virtual void DrawLine(double x1, double y1, double x2, double y2) override;
    virtual void DrawRect(double x, double y, double w, double h, bool frameOnly) override;
    virtual void FillRect(double x, double y, double w, double h, COLORREF color) override;

    virtual void DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags) override;
    virtual void DrawImage(double x, double y, double w, double h, const std::wstring& path) override;

private:
    HDC m_hdc;

private:
    // Yardımcılar
    void ClearResources();
    Gdiplus::Color ToGdiColor(COLORREF c);

private:
    Gdiplus::Graphics* m_graphics;
    Gdiplus::Pen* m_pen;
    Gdiplus::SolidBrush* m_brush;
    Gdiplus::Font* m_font;
    Gdiplus::StringFormat* m_format;

    double m_scaleX;
    double m_scaleY;
};
#endif