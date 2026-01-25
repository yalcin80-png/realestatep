#ifndef DOC_HARUDRAWCONTEXT_H
#define DOC_HARUDRAWCONTEXT_H

#pragma once

#include "IDrawContext.h"
#include "hpdf.h"
#include <string>
#include <map>
#include <vector>


class HaruDrawContext : public IDrawContext
{
public:
    HaruDrawContext();
    virtual ~HaruDrawContext();

    // IDrawContext Arayüzü
    virtual bool BeginDocument(const CString& filePath) override;
    virtual bool EndDocument() override;
    virtual void SetLogicalPageSize(double width, double height) override;

    // YENÝ RENK SÝSTEMÝ (COLORREF)
    virtual void SetPen(COLORREF color, float width = 1.0f) override;
    virtual void SetBrush(COLORREF color) override;
    virtual void SetFont(const FontDesc& font) override;

    virtual void DrawLine(double x1, double y1, double x2, double y2) override;
    virtual void DrawRect(double x, double y, double w, double h, bool frameOnly) override;
    virtual void FillRect(double x, double y, double w, double h, COLORREF color) override;

    virtual void DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags) override;
    virtual void DrawImage(double x, double y, double w, double h, const std::wstring& filePath) override;

private:
    bool BeginPage(double w, double h);
    void EndPage();
    double MapY(double y) const;

    // String Dönüþümleri
    std::string WStringToUTF8(const std::wstring& w) const;
    std::string ConvertToAnsiPath(const CString& path) const;

private:
    HPDF_Doc  m_pdf;
    HPDF_Page m_page;
    HPDF_Font m_currentFont;

    CString   m_filePath;
    double    m_logicalW;
    double    m_logicalH;
    float     m_currentFontSize;
};

#endif // DOC_HARUDRAWCONTEXT_H