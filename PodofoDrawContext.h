#pragma once

#include "IDrawContext.h"
#include <string>
#include <memory> // unique_ptr için þart

// PoDoFo Forward Declarations
namespace PoDoFo {
    class PdfMemDocument;
    class PdfPage;
    class PdfPainter;
    class PdfFont;
    class PdfImage;
}

class PodofoDrawContext : public IDrawContext
{
public:
    PodofoDrawContext();
    virtual ~PodofoDrawContext();

    // IDrawContext Arayüzü
    virtual bool BeginDocument(const CString& filePath) override;
    virtual bool EndDocument() override;
    virtual void SetLogicalPageSize(double width, double height) override;

    virtual void SetPen(COLORREF color, float width = 1.0f) override;
    virtual void SetBrush(COLORREF color) override;
    virtual void SetFont(const FontDesc& font) override;

    virtual void DrawLine(double x1, double y1, double x2, double y2) override;
    virtual void DrawRect(double x, double y, double w, double h, bool frameOnly) override;
    virtual void FillRect(double x, double y, double w, double h, COLORREF color) override;

    virtual void DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags) override;
    virtual void DrawImage(double x, double y, double w, double h, const std::wstring& filePath) override;
    std::vector<std::unique_ptr<PoDoFo::PdfImage>> m_images;
private:
    bool BeginPage(double w, double h);
    void EndPage();
    double MapY(double y) const;

    std::string WStringToTurkish(const std::wstring& str) const;
    std::string WStringToAnsi(const CString& str) const;

private:
    // PoDoFo 0.10.x pointer yönetimi deðiþti, raw pointer kullanacaðýz ama dikkatli olmalýyýz
    PoDoFo::PdfMemDocument* m_doc;
    PoDoFo::PdfPage* m_page;
    PoDoFo::PdfPainter* m_painter;
    PoDoFo::PdfFont* m_font; // 0.10'da fontlar smart pointer ile yönetilir ama raw tutabiliriz

    CString         m_filePath;
    double          m_logicalW;
    double          m_logicalH;
    float           m_currentFontSize;
};