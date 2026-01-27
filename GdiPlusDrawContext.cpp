#include "stdafx.h"
#include "GdiPlusDrawContext.h"

using namespace Gdiplus;

// =========================================================
// CONSTRUCTOR / DESTRUCTOR
// =========================================================

GdiPlusDrawContext::GdiPlusDrawContext()
    : m_hdc(nullptr)
    , m_graphics(nullptr)
    , m_pen(nullptr)
    , m_brush(nullptr)
    , m_font(nullptr)
    , m_format(nullptr)
    , m_scaleX(1.0)
    , m_scaleY(1.0)
{
    m_format = new StringFormat();
    m_format->SetAlignment(StringAlignmentNear);
    m_format->SetLineAlignment(StringAlignmentNear);
}

GdiPlusDrawContext::~GdiPlusDrawContext()
{
    End();
    if (m_format) delete m_format;
}

// =========================================================
// KAYNAK YNETM
// =========================================================

void GdiPlusDrawContext::ClearResources()
{
    if (m_pen) { delete m_pen; m_pen = nullptr; }
    if (m_brush) { delete m_brush; m_brush = nullptr; }
    if (m_font) { delete m_font; m_font = nullptr; }
    if (m_graphics) { delete m_graphics; m_graphics = nullptr; }
}

Gdiplus::Color GdiPlusDrawContext::ToGdiColor(COLORREF c)
{
    return Gdiplus::Color(255, GetRValue(c), GetGValue(c), GetBValue(c));
}

// =========================================================
// BALAT / BTR
// =========================================================

bool GdiPlusDrawContext::Begin(HDC hdc)
{
    End();
    if (!hdc) return false;

    m_hdc = hdc;
    m_graphics = new Graphics(hdc);
    if (m_graphics->GetLastStatus() != Ok)
    {
        delete m_graphics;
        m_graphics = nullptr;
        return false;
    }

    // Yazıcı mı ekran mı kontrol et
    const int tech = ::GetDeviceCaps(hdc, TECHNOLOGY);
    const bool isPrinter = (tech == DT_RASPRINTER);

    // Yazıcı için en yüksek kalite ayarları
    if (isPrinter)
    {
        m_graphics->SetSmoothingMode(SmoothingModeHighQuality);
        m_graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        m_graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        m_graphics->SetCompositingQuality(CompositingQualityHighQuality);
        m_graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
    }
    else
    {
        // Ekran için optimize edilmiş ayarlar
        m_graphics->SetSmoothingMode(SmoothingModeAntiAlias);
        m_graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        m_graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        m_graphics->SetCompositingQuality(CompositingQualityHighSpeed);
    }
    
    m_graphics->SetPageUnit(UnitPixel);

    return true;
}

void GdiPlusDrawContext::End()
{
    ClearResources();
    m_hdc = nullptr;
}

bool GdiPlusDrawContext::BeginDocument(const CString& filePath) { return true; }
bool GdiPlusDrawContext::EndDocument() { return true; }

// =========================================================
// SAYFA AYARLARI (KRTK YAZDIRMA DZELTMES)
// =========================================================

void GdiPlusDrawContext::SetLogicalPageSize(double width, double height)
{
    if (!m_graphics || width <= 0 || height <= 0) return;

    // Reset previous transforms
    m_graphics->ResetTransform();

    // Determine target drawing area in pixels.
    // For printers, GetVisibleClipBounds() can be unreliable on some drivers.
    double targetW = 0.0;
    double targetH = 0.0;

    if (m_hdc)
    {
        const int tech = ::GetDeviceCaps(m_hdc, TECHNOLOGY);
        if (tech == DT_RASPRINTER)
        {
            // Printable area (in pixels)
            targetW = (double)::GetDeviceCaps(m_hdc, HORZRES);
            targetH = (double)::GetDeviceCaps(m_hdc, VERTRES);
        }
    }

    if (targetW <= 0.0 || targetH <= 0.0)
    {
        // Screen or fallback path
        RectF bounds;
        m_graphics->GetVisibleClipBounds(&bounds);
        targetW = bounds.Width;
        targetH = bounds.Height;
    }

    if (targetW <= 0.0 || targetH <= 0.0)
        return;

    const double scaleX = targetW / width;
    const double scaleY = targetH / height;
    const double scale  = (scaleX < scaleY) ? scaleX : scaleY;

    m_graphics->ScaleTransform((REAL)scale, (REAL)scale);
}



// =========================================================
// ARALAR (PEN, BRUSH, FONT)
// =========================================================

void GdiPlusDrawContext::SetPen(COLORREF color, float width)
{
    if (m_pen) delete m_pen;
    m_pen = new Pen(ToGdiColor(color), (REAL)width);
}

void GdiPlusDrawContext::SetBrush(COLORREF color)
{
    if (m_brush) delete m_brush;
    m_brush = new SolidBrush(ToGdiColor(color));
}

void GdiPlusDrawContext::SetFont(const FontDesc& f)
{
    if (m_font) delete m_font;

    int style = FontStyleRegular;
    if (f.bold)   style |= FontStyleBold;
    if (f.italic) style |= FontStyleItalic;

    FontFamily family(f.family.c_str());
    if (!family.IsAvailable())
    {
        FontFamily arial(L"Arial");
        m_font = new Font(&arial, (REAL)f.sizePt, style, UnitPoint);
    }
    else
    {
        m_font = new Font(&family, (REAL)f.sizePt, style, UnitPoint);
    }
}

// =========================================================
// ZM FONKSYONLARI
// =========================================================

void GdiPlusDrawContext::DrawLine(double x1, double y1, double x2, double y2)
{
    if (!m_graphics || !m_pen) return;
    m_graphics->DrawLine(m_pen, (REAL)x1, (REAL)y1, (REAL)x2, (REAL)y2);
}

void GdiPlusDrawContext::DrawRect(double x, double y, double w, double h, bool frameOnly)
{
    if (!m_graphics) return;
    RectF r((REAL)x, (REAL)y, (REAL)w, (REAL)h);

    if (!frameOnly && m_brush)
        m_graphics->FillRectangle(m_brush, r);

    if (m_pen)
        m_graphics->DrawRectangle(m_pen, r);
}

void GdiPlusDrawContext::FillRect(double x, double y, double w, double h, COLORREF color)
{
    if (!m_graphics) return;
    SolidBrush tempBrush(ToGdiColor(color));
    RectF r((REAL)x, (REAL)y, (REAL)w, (REAL)h);
    m_graphics->FillRectangle(&tempBrush, r);
}

void GdiPlusDrawContext::DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags)
{
    if (!m_graphics || !m_font || text.empty()) return;

    SolidBrush* pBrush = m_brush;
    SolidBrush defaultBrush(Color(255, 0, 0, 0));
    if (!pBrush) pBrush = &defaultBrush;

    RectF layoutRect((REAL)x, (REAL)y, (REAL)w, (REAL)h);
    StringFormat fmt;

    if (flags & DT_CENTER)      fmt.SetAlignment(StringAlignmentCenter);
    else if (flags & DT_RIGHT)  fmt.SetAlignment(StringAlignmentFar);
    else                        fmt.SetAlignment(StringAlignmentNear);

    if (flags & DT_VCENTER)     fmt.SetLineAlignment(StringAlignmentCenter);
    else if (flags & DT_BOTTOM) fmt.SetLineAlignment(StringAlignmentFar);
    else                        fmt.SetLineAlignment(StringAlignmentNear);

    if (flags & DT_WORDBREAK)   fmt.SetFormatFlags(0);
    else                        fmt.SetFormatFlags(StringFormatFlagsNoWrap);

    m_graphics->DrawString(text.c_str(), -1, m_font, layoutRect, &fmt, pBrush);
}

void GdiPlusDrawContext::DrawImage(double x, double y, double w, double h, const std::wstring& path)
{
    if (!m_graphics) return;
    Image img(path.c_str());
    if (img.GetLastStatus() == Ok)
    {
        RectF r((REAL)x, (REAL)y, (REAL)w, (REAL)h);
        m_graphics->DrawImage(&img, r);
    }
}