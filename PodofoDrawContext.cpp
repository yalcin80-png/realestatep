#include "stdafx.h"
#include "PodofoDrawContext.h"
#include "ThemeConfig.h" 

// --- Windows ve PoDoFo Çakýþmalarýný Önle ---
#ifdef DrawText
#undef DrawText
#endif
#ifdef GetObject
#undef GetObject
#endif
#ifdef RGB
#undef RGB
#endif

#include <podofo/podofo.h>
#include <iostream>

using namespace PoDoFo;

// =========================================================
// YARDIMCI: RENK DÖNÜÞÜMÜ (0-255 -> 0.0-1.0)
// =========================================================
PdfColor MakeSafeColor(COLORREF c)
{
    double r = (double)GetRValue(c) / 255.0;
    double g = (double)GetGValue(c) / 255.0;
    double b = (double)GetBValue(c) / 255.0;
    return PdfColor(r, g, b);
}

// YARDIMCI: SÝSTEM FONT YOLU BULUCU
std::string GetSystemFontPath(const std::wstring& family, bool bold, bool italic) {
    std::string base = "C:\\Windows\\Fonts\\";
    std::string file = "arial.ttf";

    if (family == L"Arial" || family == L"Helvetica") {
        if (bold && italic) file = "arialbi.ttf";
        else if (bold) file = "arialbd.ttf";
        else if (italic) file = "ariali.ttf";
    }
    else if (family == L"Times New Roman") {
        file = "times.ttf";
        if (bold && italic) file = "timesbi.ttf";
        else if (bold) file = "timesbd.ttf";
        else if (italic) file = "timesi.ttf";
    }
    return base + file;
}

// =========================================================
// CONSTRUCTOR / DESTRUCTOR
// =========================================================

PodofoDrawContext::PodofoDrawContext()
    : m_doc(nullptr), m_page(nullptr), m_painter(nullptr), m_font(nullptr),
    m_logicalW(0), m_logicalH(0), m_currentFontSize(10.0f)
{
}

PodofoDrawContext::~PodofoDrawContext()
{
    if (m_painter) delete m_painter;
    if (m_doc) delete m_doc;
}

// =========================================================
// STRING DÖNÜÞÜMLERÝ
// =========================================================
std::string PodofoDrawContext::WStringToAnsi(const CString& str) const {
    if (str.IsEmpty()) return "";
    int size = WideCharToMultiByte(CP_ACP, 0, str, -1, nullptr, 0, nullptr, nullptr);
    std::string res(size, 0);
    WideCharToMultiByte(CP_ACP, 0, str, -1, &res[0], size, nullptr, nullptr);
    if (!res.empty() && res.back() == '\0') res.pop_back();
    return res;
}

std::string PodofoDrawContext::WStringToTurkish(const std::wstring& str) const {
    if (str.empty()) return "";
    // Türkçe (CP1254) dönüþümü
    int size = WideCharToMultiByte(1254, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string res(size, 0);
    WideCharToMultiByte(1254, 0, str.c_str(), -1, &res[0], size, nullptr, nullptr);
    if (!res.empty() && res.back() == '\0') res.pop_back();
    return res;
}

double PodofoDrawContext::MapY(double y) const { return m_logicalH - y; }

// =========================================================
// BELGE YÖNETÝMÝ
// =========================================================

bool PodofoDrawContext::BeginDocument(const CString& filePath) {
    m_filePath = filePath;
    try {
        m_doc = new PdfMemDocument();
        return true;
    }
    catch (const PdfError&) { return false; }
}

bool PodofoDrawContext::EndDocument() {
    if (!m_doc) return false;

    EndPage(); // Açýk sayfa varsa kapat

    try {
        std::string pathA = WStringToAnsi(m_filePath);
        m_doc->Save(pathA);

        delete m_doc; m_doc = nullptr;
        return true;
    }
    catch (const PdfError&) { return false; }
}

// =========================================================
// SAYFA YÖNETÝMÝ
// =========================================================

bool PodofoDrawContext::BeginPage(double w, double h) {
    if (!m_doc) return false;
    try {
        // PoDoFo 0.10.x DÜZELTMESÝ: PdfRect yerine Rect
        PdfPage& pageRef = m_doc->GetPages().CreatePage(PoDoFo::Rect(0, 0, w, h));
        m_page = &pageRef;

        if (m_painter) delete m_painter;
        m_painter = new PdfPainter();
        m_painter->SetCanvas(*m_page);

        return true;
    }
    catch (const PdfError&) { return false; }
}

void PodofoDrawContext::EndPage() {
    if (m_painter) {
        m_painter->FinishDrawing();
        delete m_painter; m_painter = nullptr;
    }
}

void PodofoDrawContext::SetLogicalPageSize(double w, double h) {
    m_logicalW = w; m_logicalH = h;
    EndPage();                 // önceki sayfayý kapat
    m_page = nullptr;
    BeginPage(w, h);           // yeni sayfa aç
}
// =========================================================
// AYARLAR (FONT, PEN, BRUSH)
// =========================================================

void PodofoDrawContext::SetFont(const FontDesc& f) {
    if (!m_doc) return;
    try {
        std::string fontPath = GetSystemFontPath(f.family, f.bold, f.italic);

        // PoDoFo 0.10.x DÜZELTMESÝ: LoadFont yerine SearchFont
        const PdfFont* pFont = m_doc->GetFonts().SearchFont(fontPath);

        // Bulamazsa varsayýlan fontlardan birini dene (örn: Arial)
        if (!pFont) {
            pFont = m_doc->GetFonts().SearchFont("Arial");
        }

        // Pointer'ý sakla (const_cast gerekebilir çünkü TextState non-const pointer isteyebilir)
        m_font = const_cast<PdfFont*>(pFont);

        if (m_font && m_painter) {
            m_currentFontSize = f.sizePt;
            m_painter->TextState.SetFont(*m_font, f.sizePt);
        }
    }
    catch (...) {}
}

void PodofoDrawContext::SetPen(COLORREF c, float width) {
    if (m_painter) {
        m_painter->GraphicsState.SetStrokingColor(MakeSafeColor(c));
        m_painter->GraphicsState.SetLineWidth(width);
    }
}

void PodofoDrawContext::SetBrush(COLORREF c) {
    if (m_painter) {
        m_painter->GraphicsState.SetNonStrokingColor(MakeSafeColor(c));
    }
}

// =========================================================
// ÇÝZÝM FONKSÝYONLARI
// =========================================================

void PodofoDrawContext::DrawLine(double x1, double y1, double x2, double y2) {
    if (m_painter) {
        m_painter->DrawLine(x1, MapY(y1), x2, MapY(y2));
    }
}

void PodofoDrawContext::DrawRect(double x, double y, double w, double h, bool frameOnly) {
    if (!m_painter) return;
    double py = MapY(y) - h;

    // PoDoFo 0.10.x DÜZELTMESÝ: Enum isimleri ve kullaným
    if (frameOnly) {
        // Sadece Çerçeve
        m_painter->DrawRectangle(x, py, w, h, PdfPathDrawMode::Stroke);
    }
    else {
        // Hem Dolgu Hem Çerçeve
        // Güvenli yöntem: Önce doldur, sonra çiz
        m_painter->DrawRectangle(x, py, w, h, PdfPathDrawMode::Fill);
        m_painter->DrawRectangle(x, py, w, h, PdfPathDrawMode::Stroke);
    }
}

void PodofoDrawContext::FillRect(double x, double y, double w, double h, COLORREF color) {
    if (!m_painter) return;

    m_painter->Save();
    m_painter->GraphicsState.SetNonStrokingColor(MakeSafeColor(color));

    double py = MapY(y) - h;
    m_painter->DrawRectangle(x, py, w, h, PdfPathDrawMode::Fill);

    m_painter->Restore();
}

void PodofoDrawContext::DrawTextW(const std::wstring& text, double x, double y, double w, double h, UINT flags)
{
    if (!m_painter || !m_font) return;

    // Türkçe karakter (CP1254)
    std::string trText = WStringToTurkish(text);

    // PoDoFo 0.10.x DÜZELTMESÝ: PdfString yerine direkt string kullanýmý
    double startX = x;
    double pdfY = MapY(y) - m_currentFontSize;

    if (flags & DT_VCENTER) {
        pdfY = (MapY(y) - h / 2.0) - (m_currentFontSize * 0.35);
    }

    m_painter->DrawText(trText, startX, pdfY);
}

void PodofoDrawContext::DrawImage(double x, double y, double w, double h, const std::wstring& filePath) {
    if (!m_doc || !m_painter) return;
    try {
        std::string pathA = WStringToAnsi(filePath.c_str());

        // PoDoFo 0.10.x DÜZELTMESÝ: CreateImage unique_ptr döner
        auto imagePtr = m_doc->CreateImage();
        imagePtr->Load(pathA);

        double py = MapY(y) - h;

        double imgW = imagePtr->GetWidth();
        double imgH = imagePtr->GetHeight();
        double scX = (imgW > 0) ? w / imgW : 1.0;
        double scY = (imgH > 0) ? h / imgH : 1.0;

        // Pointer'ýn içeriðini (*imagePtr) referans olarak geçiriyoruz
        m_painter->DrawImage(*imagePtr, x, py, scX, scY);
        m_images.push_back(std::move(imagePtr)); // export bitene kadar yaþasýn


    }
    catch (...) {}
}