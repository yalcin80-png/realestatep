#pragma once

#ifndef PODOFO_PDF_H
#define PODOFO_PDF_H
//
#include "stdafx.h"
#include <podofo/podofo.h>
#include "IDrawContext.h"
#include "GdiPlusDrawContext.h"


//
//
//// Form verilerini taşıyacak yapı
//struct ContractData {
//    // İş Sahibi
//    CString OwnerName;
//    CString OwnerTC;
//    CString OwnerPhone;
//    CString OwnerEmail;
//    CString OwnerAddress;
//    bool IsCorporate = false;
//
//    // Mülk (Tapu) Bilgileri
//    CString City;
//    CString District;
//    CString Neighborhood;
//    CString Pafta;
//    CString Ada;
//    CString Parsel;
//    CString MapArea; // m2
//    CString FullAddress;
//    CString PropertyType; // Konut, Arsa vb.
//
//    // Fiyat ve Şartlar
//    CString Price;
//    CString PriceText; // Yazıyla
//    CString ServiceFeeRate = _T("%2"); // Varsayılan hizmet bedeli
//
//    // Firma
//    CString AgentName = _T("GAZİŞEHİR GAYRİMENKUL");
//};
//
//class ContractPageLayout
//{
//public:
//    ContractPageLayout();
//    virtual ~ContractPageLayout();
//
//    void SetData(const ContractData& data);
//    void Render(IDrawContext& ctx);
//
//private:
//    ContractData m_data;
//
//    // Sayfa Sabitleri
//    static const int PAGE_W = 2480; // A4 @ 300 DPI mantığı
//    static const int PAGE_H = 3508;
//    static const int MARGIN = 100;
//    static const int ROW_H = 50;
//
//    // Yardımcılar
//    std::wstring ToW(const CString& s);
//    FontDesc MakeFont(float sizePt, bool bold = false, bool italic = false, bool underline = false);
//
//    // Çizim Parçaları
//    void RenderHeader(IDrawContext& ctx, int& y);
//    void RenderOwnerSection(IDrawContext& ctx, int& y);
//    void RenderPropertySection(IDrawContext& ctx, int& y);
//    void RenderTermsSection(IDrawContext& ctx, int& y);
//    void RenderSignatures(IDrawContext& ctx, int& y);
//
//    // Araçlar
//    void DrawSectionHeader(IDrawContext& ctx, const std::wstring& title, int& y);
//    void DrawLabelValue(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value);
//    void DrawCheckOption(IDrawContext& ctx, int x, int y, const std::wstring& label, bool checked);
//};
//
//
//
//
//
//
//







//#include "dataIsMe.h"
//#include "PreviewPanel.h" 
//#include <vector>
//#include <string>
//#include <memory>
//#include <algorithm>
//
//// PoDoFo dahil etme
//#include <podofo/podofo.h> 
//using namespace PoDoFo;
//
//// Render Config sabitleri (Point - 72 DPI)
//namespace RenderConfig
//{
//    // A4 boyutları (Point cinsinden PoDoFo standardı)
//    const float P_A4W = 595.0f;
//    const float P_A4H = 842.0f;
//    const float MARGIN = 40.0f; // 40 pt kenar boşluğu
//    const float PADDING = 5.0f;
//    const int COL_COUNT = 4;
//    const float ROW_HEIGHT = 20.0f; // 20 pt satır yüksekliği
//
//    // Kurumsal Renkler (RGB olarak tutulur)
//    const COLORREF COL_PRIMARY = RGB(200, 30, 45);
//    const COLORREF COL_HEADER = RGB(45, 45, 45);
//    const COLORREF COL_TEXT = RGB(25, 25, 25);
//    const COLORREF COL_META = RGB(110, 110, 110);
//    const COLORREF COL_ZEBRA = RGB(242, 242, 242);
//    const COLORREF COL_LINE = RGB(200, 200, 200);
//}
//
//// ----------------------------------------------------
//// Arayüz: IFormBlock
//// ----------------------------------------------------
//class IFormBlock
//{
//public:
//    virtual ~IFormBlock() = default;
//    // currentY: Üstten boşluğun Point değeri (GDI gibi). Dönüş: Yeni Y konumu.
//    virtual float Render(PdfPainter& painter, float currentY) = 0;
//
//protected:
//    // CString'den PoDoFo'nun beklediği UTF-8 dönüşümü
//    std::string CStringtoUTF8(const CString& w) const;
//
//    // PoDoFo'da Renk Ayarı
//    void SetRGBFill(PdfPainter& painter, COLORREF rgb);
//    void SetRGBStroke(PdfPainter& painter, COLORREF rgb);
//
//    // PoDoFo ile Metin Çizimi Yardımcısı
//    void DrawTextStyled(PdfPainter& painter, PdfFont* font, float size, COLORREF color,
//        const CString& text, float x, float y, float width, int align = PdfPainter::Left);
//};
//
//// ----------------------------------------------------
//// ANA RENDER SINIFI: CPDFFormRenderer
//// ----------------------------------------------------
//class CPDFFormRenderer
//{
//public:
//    using PreviewItem = CPreviewDlg::PreviewItem;
//
//    CPDFFormRenderer();
//    ~CPDFFormRenderer();
//
//    void SetData(const PreviewItem& data);
//    bool ExportToPDF(const CString& filePath);
//
//private:
//    PdfMemDocument m_doc;
//    PreviewItem m_data;
//
//    // Fontlar (TTF ile Türkçe karakter desteği için)
//    PdfFont* m_defaultFont = nullptr;
//    PdfFont* m_boldFont = nullptr;
//
//    std::vector<std::unique_ptr<IFormBlock>> m_blocks;
//
//    // Yardımcı metotlar
//    void LoadFonts();
//    void DrawHeaderBlock(PdfPainter& painter, float& yPos);
//    void DrawFooterBlock(PdfPainter& painter);
//
//    // Dosya yolu için ANSI dönüşümü
//    std::string W2A(const CString& w) const;
//};
//
//// ----------------------------------------------------
//// BAĞIMSIZ BÖLÜM SINIFLARI (Constructor'lar doğru referans başlatır)
//// ----------------------------------------------------
//
//class CCustomerBlock : public IFormBlock
//{
//public:
//    CCustomerBlock(PdfFont* dFont, PdfFont* bFont, const std::vector<std::pair<CString, CString>>& data);
//    float Render(PdfPainter& painter, float currentY) override;
//private:
//    const std::vector<std::pair<CString, CString>>& m_data;
//    PdfFont* m_defaultFont;
//    PdfFont* m_boldFont;
//    float DrawGridRow4(PdfPainter& painter, const std::pair<CString, CString>* kvs, float y, bool zebra);
//};
//
//class CPropertyBlock : public IFormBlock
//{
//public:
//    CPropertyBlock(PdfFont* dFont, PdfFont* bFont, const std::vector<std::pair<CString, CString>>& data);
//    float Render(PdfPainter& painter, float currentY) override;
//private:
//    const std::vector<std::pair<CString, CString>>& m_data;
//    PdfFont* m_defaultFont;
//    PdfFont* m_boldFont;
//    float DrawGridRow4(PdfPainter& painter, const std::pair<CString, CString>* kvs, float y, bool zebra);
//};
//
//class CMetricBlock : public IFormBlock
//{
//public:
//    CMetricBlock(PdfFont* dFont, PdfFont* bFont);
//    float Render(PdfPainter& painter, float currentY) override;
//private:
//    PdfFont* m_defaultFont;
//    PdfFont* m_boldFont;
//    void DrawCheckBox(PdfPainter& painter, float x, float y, const CString& label, float size);
//};
//
//class CNotesBlock : public IFormBlock
//{
//public:
//    CNotesBlock(PdfFont* dFont, const CString& notes);
//    float Render(PdfPainter& painter, float currentY) override;
//private:
//    const CString& m_notes;
//    PdfFont* m_defaultFont;
//};
//
#endif // PODOFO_PDF_H