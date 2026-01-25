#include "stdafx.h"
#include "CPDFFormGenerator.h"

//
//
//// --------------------------------------------------------
//// Yardımcılar
//// --------------------------------------------------------
//std::wstring ContractPageLayout::ToW(const CString& s) {
//#ifdef UNICODE
//    return std::wstring(s.GetString());
//#else
//    int len = ::MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
//    if (len <= 1) return std::wstring();
//    std::wstring ws(static_cast<size_t>(len - 1), L'\0');
//    ::MultiByteToWideChar(CP_ACP, 0, s, -1, &ws[0], len - 1);
//    return ws;
//#endif
//}
//
//FontDesc ContractPageLayout::MakeFont(float sizePt, bool bold, bool italic, bool underline) {
//    FontDesc fd;
//    fd.family = L"Arial";
//    fd.sizePt = sizePt;
//    fd.bold = bold;
//    fd.italic = italic;
//    fd.underline = underline;
//    return fd;
//}
//
//ContractPageLayout::ContractPageLayout() {}
//ContractPageLayout::~ContractPageLayout() {}
//
//void ContractPageLayout::SetData(const ContractData& data) {
//    m_data = data;
//}
//
//// --------------------------------------------------------
//// RENDER (ANA DÖNGÜ)
//// --------------------------------------------------------
//void ContractPageLayout::Render(IDrawContext& ctx)
//{
//    ctx.SetLogicalPageSize(PAGE_W, PAGE_H);
//    ctx.FillRect(0, 0, PAGE_W, PAGE_H, Color4f(RGB(255, 255, 255)));
//
//    int y = MARGIN;
//
//    RenderHeader(ctx, y);
//    //RenderOwnerInfo(ctx, y);
//    //RenderPropertyInfo(ctx, y);
//    //RenderTermsAndPrice(ctx, y);
//    RenderSignatures(ctx, y);
//}
//
//// --------------------------------------------------------
//// 1. HEADER
//// --------------------------------------------------------
//void ContractPageLayout::RenderHeader(IDrawContext& ctx, int& y)
//{
//    // Sol Üst: Logo / İsim
//    ctx.SetFont(MakeFont(16, true));
//    ctx.SetBrush(Color4f(RGB(0, 0, 0)));
//    ctx.DrawTextW(L"REALTY WORLD", MARGIN, y, 400, 50, DT_LEFT);
//
//    // Sağ Üst: Acente
//    ctx.SetFont(MakeFont(12, true));
//    ctx.DrawTextW(ToW(m_data.AgentName), PAGE_W - MARGIN - 500, y, 500, 50, DT_RIGHT);
//
//    y += 80;
//
//    // Başlık
//    ctx.SetFont(MakeFont(20, true));
//    ctx.SetBrush(Color4f(RGB(180, 0, 0)));
//    ctx.DrawTextW(L"SATILIK YETKİLENDİRME SÖZLEŞMESİ", MARGIN, y, PAGE_W - 2 * MARGIN, 60, DT_CENTER);
//
//    y += 80;
//    // Çizgi
//    ctx.SetPen(Color4f(RGB(200, 200, 200)), 1.0f);
//    ctx.DrawLine(MARGIN, y, PAGE_W - MARGIN, y);
//    y += 20;
//}
//
//// --------------------------------------------------------
//// 2. İŞ SAHİBİ BİLGİLERİ
//// --------------------------------------------------------
//void ContractPageLayout::RenderOwnerSection(IDrawContext& ctx, int& y)
//{
//    DrawSectionHeader(ctx, L"İŞ SAHİBİ BİLGİLERİ", y);
//
//    // Checkboxlar
//    int cbY = y;
//    DrawCheckOption(ctx, MARGIN + 20, cbY, L"GERÇEK KİŞİ", !m_data.IsCorporate);
//    DrawCheckOption(ctx, MARGIN + 400, cbY, L"TÜZEL KİŞİ", m_data.IsCorporate);
//    y += 60;
//
//    // Satırlar
//    int w = PAGE_W - 2 * MARGIN;
//    int hw = w / 2;
//
//    DrawLabelValue(ctx, MARGIN, y, hw, ROW_H, L"ADI SOYADI", ToW(m_data.OwnerName));
//    DrawLabelValue(ctx, MARGIN + hw, y, hw, ROW_H, L"T.C. / KİMLİK NO", ToW(m_data.OwnerTC));
//    y += ROW_H;
//
//    DrawLabelValue(ctx, MARGIN, y, hw, ROW_H, L"TELEFON", ToW(m_data.OwnerPhone));
//    DrawLabelValue(ctx, MARGIN + hw, y, hw, ROW_H, L"E-POSTA", ToW(m_data.OwnerEmail));
//    y += ROW_H;
//
//    DrawLabelValue(ctx, MARGIN, y, w, ROW_H * 2, L"ADRES", ToW(m_data.OwnerAddress));
//    y += (ROW_H * 2) + 20;
//}
//
//// --------------------------------------------------------
//// 3. TAPU KAYDI
//// --------------------------------------------------------
//void ContractPageLayout::RenderPropertySection(IDrawContext& ctx, int& y)
//{
//    DrawSectionHeader(ctx, L"GAYRİMENKULÜN TAPU KAYDI", y);
//
//    int w = PAGE_W - 2 * MARGIN;
//    int col3 = w / 3;
//
//    // Satır 1
//    DrawLabelValue(ctx, MARGIN, y, col3, ROW_H, L"İLİ", ToW(m_data.City));
//    DrawLabelValue(ctx, MARGIN + col3, y, col3, ROW_H, L"İLÇESİ", ToW(m_data.District));
//    DrawLabelValue(ctx, MARGIN + 2 * col3, y, col3, ROW_H, L"MAHALLESİ", ToW(m_data.Neighborhood));
//    y += ROW_H;
//
//    // Satır 2
//    DrawLabelValue(ctx, MARGIN, y, col3, ROW_H, L"PAFTA", ToW(m_data.Pafta));
//    DrawLabelValue(ctx, MARGIN + col3, y, col3, ROW_H, L"ADA", ToW(m_data.Ada));
//    DrawLabelValue(ctx, MARGIN + 2 * col3, y, col3, ROW_H, L"PARSEL", ToW(m_data.Parsel));
//    y += ROW_H;
//
//    // Satır 3
//    DrawLabelValue(ctx, MARGIN, y, col3, ROW_H, L"PARSEL ALANI (M2)", ToW(m_data.MapArea));
//    DrawLabelValue(ctx, MARGIN + col3, y, col3 * 2, ROW_H, L"NİTELİĞİ", ToW(m_data.PropertyType));
//    y += ROW_H + 10;
//
//    // Açık Adres
//    DrawLabelValue(ctx, MARGIN, y, w, ROW_H, L"GAYRİMENKUL ADRESİ", ToW(m_data.FullAddress));
//    y += ROW_H + 20;
//}
//
//// --------------------------------------------------------
//// 4. ŞARTLAR VE FİYAT
//// --------------------------------------------------------
//void ContractPageLayout::RenderTermsSection(IDrawContext& ctx, int& y)
//{
//    DrawSectionHeader(ctx, L"HİZMETİN NİTELİĞİ VE FİYAT", y);
//
//    // Sözleşme metni (PF11212.pdf'ten özet)
//    std::wstring terms =
//        L"1. İşbu yetkilendirme sözleşmesi özellikleri belirtilen gayrimenkulün satışına aracılık edilmesi amacıyla düzenlenmiştir.\n"
//        L"2. İşletme, bu gayrimenkulün pazarlanması için uygun gördüğü her türlü pazarlama aracını kullanma hakkına sahiptir.\n"
//        L"3. İş sahibi, gayrimenkulün satış bedeli olarak aşağıdaki tutarı kabul eder.\n"
//        L"4. Hizmet gerçekleştiğinde, iş sahibi belirtilen bedel üzerinden %2 + KDV hizmet bedeli ödemeyi taahhüt eder.";
//
//    ctx.SetFont(MakeFont(10, false));
//    ctx.SetBrush(Color4f(RGB(60, 60, 60)));
//    ctx.DrawTextW(terms, MARGIN, y, PAGE_W - 2 * MARGIN, 300, DT_LEFT | DT_WORDBREAK);
//
//    y += 250; // Metin boşluğu
//
//    // Fiyat Kutusu
//    int w = PAGE_W - 2 * MARGIN;
//    ctx.SetPen(Color4f(RGB(0, 0, 0)), 2.0f);
//    ctx.DrawRect(MARGIN, y, w, 120, false);
//
//    ctx.SetFont(MakeFont(12, true));
//    ctx.DrawTextW(L"GAYRİMENKUL BEDELİ", MARGIN + 20, y + 10, 400, 30, DT_LEFT);
//
//    ctx.SetFont(MakeFont(14, true));
//    ctx.DrawTextW(L"RAKAM İLE: " + ToW(m_data.Price) + L" TL", MARGIN + 20, y + 50, w - 40, 40, DT_LEFT);
//
//    ctx.SetFont(MakeFont(12, false, true)); // İtalik
//    ctx.DrawTextW(L"YAZI İLE: " + ToW(m_data.PriceText), MARGIN + 20, y + 85, w - 40, 30, DT_LEFT);
//
//    y += 150;
//}
//
//// --------------------------------------------------------
//// 5. İMZALAR
//// --------------------------------------------------------
//void ContractPageLayout::RenderSignatures(IDrawContext& ctx, int& y)
//{
//    int boxW = 600;
//    int boxH = 200;
//
//    // Sol Kutu (Emlakçı)
//    ctx.SetPen(Color4f(RGB(150, 150, 150)), 1.0f);
//    ctx.DrawRect(MARGIN, y, boxW, boxH, true);
//
//    ctx.SetFont(MakeFont(11, true));
//    ctx.DrawTextW(L"EMLAK İŞLETMESİ", MARGIN, y + 10, boxW, 30, DT_CENTER);
//    ctx.SetFont(MakeFont(9, false));
//    ctx.DrawTextW(ToW(m_data.AgentName), MARGIN, y + 40, boxW, 30, DT_CENTER);
//    ctx.DrawTextW(L"(İmza / Kaşe)", MARGIN, y + 150, boxW, 30, DT_CENTER);
//
//    // Sağ Kutu (İş Sahibi)
//    int rightX = PAGE_W - MARGIN - boxW;
//    ctx.DrawRect(rightX, y, boxW, boxH, true);
//
//    ctx.SetFont(MakeFont(11, true));
//    ctx.DrawTextW(L"İŞ SAHİBİ", rightX, y + 10, boxW, 30, DT_CENTER);
//    ctx.SetFont(MakeFont(9, false));
//    ctx.DrawTextW(ToW(m_data.OwnerName), rightX, y + 40, boxW, 30, DT_CENTER);
//    ctx.DrawTextW(L"(İmza)", rightX, y + 150, boxW, 30, DT_CENTER);
//}
//
//// --------------------------------------------------------
//// GRAFİK FONKSİYONLARI
//// --------------------------------------------------------
//void ContractPageLayout::DrawSectionHeader(IDrawContext& ctx, const std::wstring& title, int& y) {
//    ctx.SetFont(MakeFont(12, true));
//    ctx.SetBrush(Color4f(RGB(100, 100, 100)));
//    ctx.DrawTextW(title, MARGIN, y, PAGE_W, 40, DT_LEFT);
//    y += 30;
//    ctx.SetPen(Color4f(RGB(200, 200, 200)), 1.0f);
//    ctx.DrawLine(MARGIN, y, PAGE_W - MARGIN, y);
//    y += 10;
//}
//
//void ContractPageLayout::DrawLabelValue(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value) {
//    ctx.SetPen(Color4f(RGB(220, 220, 220)), 1.0f);
//    ctx.DrawRect(x, y, w, h, true); // Çerçeve
//
//    // Label
//    ctx.SetFont(MakeFont(8, true));
//    ctx.SetBrush(Color4f(RGB(120, 120, 120)));
//    ctx.DrawTextW(label, x + 5, y + 2, w - 10, 20, DT_LEFT);
//
//    // Value
//    ctx.SetFont(MakeFont(10, false));
//    ctx.SetBrush(Color4f(RGB(0, 0, 0)));
//    ctx.DrawTextW(value, x + 10, y + 18, w - 20, h - 18, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
//}
//
//void ContractPageLayout::DrawCheckOption(IDrawContext& ctx, int x, int y, const std::wstring& label, bool checked) {
//    ctx.SetPen(Color4f(RGB(0, 0, 0)), 1.0f);
//    ctx.DrawRect(x, y, 25, 25, false); // Kutu
//
//    if (checked) {
//        ctx.SetFont(MakeFont(14, true));
//        ctx.DrawTextW(L"X", x + 2, y - 2, 25, 25, DT_CENTER);
//    }
//
//    ctx.SetFont(MakeFont(10, false));
//    ctx.DrawTextW(label, x + 35, y + 2, 200, 25, DT_LEFT | DT_VCENTER);
//}
//
//
//
//







//#include "PreviewPanel.h" // SplitDataToSections için
//#include <algorithm>
//#include <cmath>
//
//// --- IFormBlock Yardımcıları ---
//
//std::string IFormBlock::CStringtoUTF8(const CString& w) const
//{
//    if (w.IsEmpty()) return {};
//    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
//    std::string result(sizeNeeded, 0);
//    WideCharToMultiByte(CP_UTF8, 0, w, -1, &result[0], sizeNeeded, nullptr, nullptr);
//    if (!result.empty()) result.pop_back();
//    return result;
//}
//
//void IFormBlock::SetRGBFill(PdfPainter& painter, COLORREF rgb)
//{
//    float r = (float)GetRValue(rgb) / 255.0f;
//    float g = (float)GetGValue(rgb) / 255.0f;
//    float b = (float)GetBValue(rgb) / 255.0f;
//    painter.SetFillingColor(r, g, b);
//}
//
//void IFormBlock::SetRGBStroke(PdfPainter& painter, COLORREF rgb)
//{
//    float r = (float)GetRValue(rgb) / 255.0f;
//    float g = (float)GetGValue(rgb) / 255.0f;
//    float b = (float)GetBValue(rgb) / 255.0f;
//    painter.SetStrokingColor(r, g, b);
//}
//
//void IFormBlock::DrawTextStyled(PdfPainter& painter, PdfFont* font, float size, COLORREF color,
//    const CString& text, float x, float y, float width, int align)
//{
//    SetRGBFill(painter, color);
//    painter.SetFont(font);
//    painter.SetFontSize(size);
//
//    // Y koordinatı GDI'dan (üstten) geliyor. PoDoFo sol alttan başladığı için ters çevrilir.
//    float drawY = RenderConfig::P_A4H - y;
//
//    // PoDoFo'nun DrawText(x, y, w, h, text, align) metodu kullanılır. 
//    // Y, metin kutusunun alt kenarıdır.
//    painter.DrawText(x, drawY - size, width, size, CStringtoUTF8(text), align);
//}
//
//// ----------------------------------------------------
//// CPDFFormRenderer (Yönetici) Implementasyonu
//// ----------------------------------------------------
//
//std::string CPDFFormRenderer::W2A(const CString& w) const
//{
//    if (w.IsEmpty()) return {};
//    int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, w, -1, nullptr, 0, nullptr, nullptr);
//    std::string result(sizeNeeded, 0);
//    WideCharToMultiByte(CP_ACP, 0, w, -1, &result[0], sizeNeeded, nullptr, nullptr);
//    if (!result.empty()) result.pop_back();
//    return result;
//}
//
//CPDFFormRenderer::CPDFFormRenderer()
//{
//    LoadFonts();
//}
//
//CPDFFormRenderer::~CPDFFormRenderer()
//{
//    // PdfMemDocument otomatik temizlenir.
//}
//
//void CPDFFormRenderer::LoadFonts()
//{
//    // TTF yükleme (Türkçe Karakterler için KRİTİK)
//    try
//    {
//        // Arial fontu sistemde varsa yüklenir. PoDoFo'nun TTF yükleme fonksiyonu
//        // UTF-8 ve tüm Türkçe karakterleri destekler.
//        m_defaultFont = m_doc.CreateFont("C:\\Windows\\Fonts\\Arial.ttf");
//        m_boldFont = m_doc.CreateFont("C:\\Windows\\Fonts\\Arialbd.ttf");
//
//        // Fontları UTF-8 olarak işaretle
//        m_defaultFont->SetEncoding(PdfEncodingFactory::Global()->GetEncoding("UTF-8"));
//        m_boldFont->SetEncoding(PdfEncodingFactory::Global()->GetEncoding("UTF-8"));
//    }
//    catch (const PdfError& e)
//    {
//        TRACE(L"[PODOFO HATA] Font yükleme hatası: %S\n", e.what());
//        // Fallback: Standart PoDoFo fontunu kullan
//        m_defaultFont = m_doc.CreateStandardFont("Helvetica");
//        m_boldFont = m_doc.CreateStandardFont("Helvetica-Bold");
//    }
//}
//
//void CPDFFormRenderer::SetData(const PreviewItem& data)
//{
//    m_data = data;
//}
//
//void CPDFFormRenderer::DrawHeaderBlock(PdfPainter& painter, float& yPos)
//{
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float X = RenderConfig::MARGIN;
//
//    // 1. Firma Bilgileri (Sol Üst)
//    float textY = yPos - 15.0f;
//    float colWidth = W / 2.0f;
//
//    DrawTextStyled(painter, m_defaultFont, 10.0f, RenderConfig::COL_META, L"Realty World Gazişehir Gayrimenkul", X, textY, colWidth);
//    textY -= 15.0f;
//    DrawTextStyled(painter, m_defaultFont, 8.0f, RenderConfig::COL_META, L"15 Temmuz Mah. Prof. Necmettin Erbakan Cad.", X, textY, colWidth);
//    textY -= 12.0f;
//    DrawTextStyled(painter, m_defaultFont, 8.0f, RenderConfig::COL_META, L"Tel: 0342 341 88 66 | GSM: 0534 938 15 10", X, textY, colWidth);
//
//    // 2. Form Başlığı (Sağ Üst)
//    float titleX = X + colWidth;
//
//    // Sağ Hizalama
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(16.0f);
//    SetRGBFill(painter, RenderConfig::COL_PRIMARY);
//
//    float titleDrawY = RenderConfig::P_A4H - yPos + 20;
//    painter.DrawText(titleX, titleDrawY - 16, colWidth, 16, CStringtoUTF8(L"GAYRİMENKUL BİLGİ FORMU"), PdfPainter::Right);
//
//    // 3. Alt Ayırıcı Çizgi
//    float lineY = yPos - 60.0f;
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.0f);
//
//    painter.DrawLine(X, RenderConfig::P_A4H - lineY, X + W, RenderConfig::P_A4H - lineY);
//    painter.Stroke();
//
//    yPos = lineY - 10.0f;
//}
//
//void CPDFFormRenderer::DrawFooterBlock(PdfPainter& painter)
//{
//    const float X = RenderConfig::MARGIN;
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float FooterY = RenderConfig::MARGIN - 15;
//
//    painter.SetFont(m_defaultFont);
//    painter.SetFontSize(8.0f);
//    SetRGBFill(painter, RenderConfig::COL_META);
//
//    painter.DrawText(X, FooterY, W, 10, CStringtoUTF8(L"© Realty World Gazişehir Gayrimenkul - Yönetim Sistemi"), PdfPainter::Center);
//}
//
//
//bool CPDFFormRenderer::ExportToPDF(const CString& filePath)
//{
//    try
//    {
//        // 1. Veriyi Bölümlere Ayır
//        CString table = m_data.tableName;
//        std::vector<std::pair<CString, CString>> customerFields;
//        std::vector<std::pair<CString, CString>> propertyFieldsOrdered;
//        CString otherNotes;
//
//        // CPreviewDlg::SplitDataToSections'ın statik bir helper olduğunu varsayıyoruz.
//        CPreviewDlg::SplitDataToSections(table, m_data.fields,
//            customerFields, propertyFieldsOrdered, otherNotes);
//
//        // 2. Sayfa ve Painter Hazırlığı
//        PdfPage* page = m_doc.CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_A4));
//        PdfPainter painter;
//        painter.SetPage(page);
//
//        float currentY = RenderConfig::P_A4H - RenderConfig::MARGIN;
//
//        // 3. Header Çiz
//        DrawHeaderBlock(painter, currentY);
//
//        // 4. Modüler Blokları Oluştur ve Sırayla Çiz
//        m_blocks.clear();
//
//        if (!customerFields.empty())
//            m_blocks.emplace_back(std::make_unique<CCustomerBlock>(m_defaultFont, m_boldFont, customerFields));
//
//        if (!propertyFieldsOrdered.empty())
//            m_blocks.emplace_back(std::make_unique<CPropertyBlock>(m_defaultFont, m_boldFont, propertyFieldsOrdered));
//
//        m_blocks.emplace_back(std::make_unique<CMetricBlock>(m_defaultFont, m_boldFont));
//
//        if (!otherNotes.IsEmpty())
//            m_blocks.emplace_back(std::make_unique<CNotesBlock>(m_defaultFont, otherNotes));
//
//        // 5. Blokları Yürüt
//        for (const auto& block : m_blocks)
//        {
//            currentY = block->Render(painter, currentY - 15.0f);
//        }
//
//        // 6. Footer Çiz
//        DrawFooterBlock(painter);
//
//        painter.FinishPage();
//
//        // 7. Kaydetme
//        m_doc.Write(W2A(filePath));
//        return true;
//    }
//    catch (const PdfError& e)
//    {
//        TRACE(L"[PODOFO KRİTİK HATA] %S: %S\n", e.what(), e.GetErrorName());
//        return false;
//    }
//}
//
//// ----------------------------------------------------
//// Modül 1: CCustomerBlock - CONSTRUCTOR & RENDER
//// ----------------------------------------------------
//
//CCustomerBlock::CCustomerBlock(PdfFont* dFont, PdfFont* bFont, const std::vector<std::pair<CString, CString>>& data)
//    : m_data(data), m_defaultFont(dFont), m_boldFont(bFont) {
//}
//
//float CCustomerBlock::DrawGridRow4(PdfPainter& painter, const std::pair<CString, CString>* kvs, float y, bool zebra)
//{
//    const float X = RenderConfig::MARGIN;
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float H = RenderConfig::ROW_HEIGHT;
//    const float Pad = RenderConfig::PADDING;
//    const float ColW = W / RenderConfig::COL_COUNT;
//    float drawY = RenderConfig::P_A4H - y;
//
//    // 1. Zemin (Zebra Satır)
//    if (zebra)
//    {
//        SetRGBFill(painter, RenderConfig::COL_ZEBRA);
//        painter.DrawRect(X, drawY - H, W, H);
//        painter.Fill();
//    }
//
//    for (int i = 0; i < RenderConfig::COL_COUNT; ++i)
//    {
//        const CString& key = kvs[i].first;
//        const CString& val = kvs[i].second;
//
//        if (key.IsEmpty()) continue;
//
//        float cx1 = X + i * ColW;
//        float cx2 = cx1 + ColW;
//
//        // 2. Label (Etiket) - Üst Yarı
//        painter.SetFont(m_boldFont);
//        painter.SetFontSize(9.0f);
//        SetRGBFill(painter, RenderConfig::COL_META);
//        painter.DrawText(cx1 + Pad, drawY - 12, CStringtoUTF8(key));
//
//        // 3. Value (Değer) - Alt Yarı
//        painter.SetFont(m_defaultFont);
//        painter.SetFontSize(11.0f);
//        SetRGBFill(painter, RenderConfig::COL_TEXT);
//        painter.DrawText(cx1 + Pad, drawY - 24, CStringtoUTF8(val));
//
//        // 4. Dikey Ayırıcı
//        if (i < RenderConfig::COL_COUNT - 1)
//        {
//            SetRGBStroke(painter, RenderConfig::COL_LINE);
//            painter.SetLineWidth(0.5f);
//            painter.DrawLine(cx2, drawY, cx2, drawY - H);
//            painter.Stroke();
//        }
//    }
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.0f);
//    painter.DrawLine(X, drawY - H, X + W, drawY - H);
//    painter.Stroke();
//
//    return y + H;
//}
//
//float CCustomerBlock::Render(PdfPainter& painter, float currentY)
//{
//    if (m_data.empty()) return currentY;
//
//    // 1. Başlık Çizimi
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(14.0f);
//    SetRGBFill(painter, RenderConfig::COL_HEADER);
//    painter.DrawText(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY + 5, CStringtoUTF8(L"MÜŞTERİ BİLGİLERİ"));
//
//    currentY += 20.0f;
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.5f);
//    painter.DrawLine(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY, RenderConfig::P_A4W - RenderConfig::MARGIN, RenderConfig::P_A4H - currentY);
//    painter.Stroke();
//
//    currentY += 10.0f;
//
//    // 2. Veri Grid'i
//    bool zebra = false;
//    const size_t n = m_data.size();
//
//    for (size_t i = 0; i < n; i += RenderConfig::COL_COUNT)
//    {
//        zebra = !zebra;
//
//        std::pair<CString, CString> kvs[RenderConfig::COL_COUNT];
//        for (int j = 0; j < RenderConfig::COL_COUNT; ++j)
//        {
//            if (i + j < n)
//                kvs[j] = m_data[i + j];
//            else
//                kvs[j] = { L"", L"" };
//        }
//
//        currentY = DrawGridRow4(painter, kvs, currentY, zebra);
//    }
//
//    return currentY;
//}
//// ----------------------------------------------------
//// Modül 2: CPropertyBlock - CONSTRUCTOR & RENDER
//// ----------------------------------------------------
//CPropertyBlock::CPropertyBlock(PdfFont* dFont, PdfFont* bFont, const std::vector<std::pair<CString, CString>>& data)
//    : m_data(data), m_defaultFont(dFont), m_boldFont(bFont) {
//}
//
//float CPropertyBlock::DrawGridRow4(PdfPainter& painter, const std::pair<CString, CString>* kvs, float y, bool zebra)
//{
//    // CCustomerBlock ile tamamen aynı implementasyon (Tekrar Kodu)
//    const float X = RenderConfig::MARGIN;
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float H = RenderConfig::ROW_HEIGHT;
//    const float Pad = RenderConfig::PADDING;
//    const float ColW = W / RenderConfig::COL_COUNT;
//    float drawY = RenderConfig::P_A4H - y;
//
//    if (zebra)
//    {
//        SetRGBFill(painter, RenderConfig::COL_ZEBRA);
//        painter.DrawRect(X, drawY - H, W, H);
//        painter.Fill();
//    }
//
//    for (int i = 0; i < RenderConfig::COL_COUNT; ++i)
//    {
//        const CString& key = kvs[i].first;
//        const CString& val = kvs[i].second;
//
//        if (key.IsEmpty()) continue;
//
//        float cx1 = X + i * ColW;
//        float cx2 = cx1 + ColW;
//
//        painter.SetFont(m_boldFont);
//        painter.SetFontSize(9.0f);
//        SetRGBFill(painter, RenderConfig::COL_META);
//        painter.DrawText(cx1 + Pad, drawY - 12, CStringtoUTF8(key));
//
//        painter.SetFont(m_defaultFont);
//        painter.SetFontSize(11.0f);
//        SetRGBFill(painter, RenderConfig::COL_TEXT);
//        painter.DrawText(cx1 + Pad, drawY - 24, CStringtoUTF8(val));
//
//        if (i < RenderConfig::COL_COUNT - 1)
//        {
//            SetRGBStroke(painter, RenderConfig::COL_LINE);
//            painter.SetLineWidth(0.5f);
//            painter.DrawLine(cx2, drawY, cx2, drawY - H);
//            painter.Stroke();
//        }
//    }
//
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.0f);
//    painter.DrawLine(X, drawY - H, X + W, drawY - H);
//    painter.Stroke();
//
//    return y + H;
//}
//
//float CPropertyBlock::Render(PdfPainter& painter, float currentY)
//{
//    if (m_data.empty()) return currentY;
//
//    // 1. Başlık Çizimi
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(14.0f);
//    SetRGBFill(painter, RenderConfig::COL_HEADER);
//    painter.DrawText(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY + 5, CStringtoUTF8(L"MÜLK DETAYLARI"));
//
//    currentY += 20.0f;
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.5f);
//    painter.DrawLine(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY, RenderConfig::P_A4W - RenderConfig::MARGIN, RenderConfig::P_A4H - currentY);
//    painter.Stroke();
//
//    currentY += 10.0f;
//
//    // 2. Veri Grid'i
//    bool zebra = false;
//    const size_t n = m_data.size();
//
//    for (size_t i = 0; i < n; i += RenderConfig::COL_COUNT)
//    {
//        zebra = !zebra;
//
//        std::pair<CString, CString> kvs[RenderConfig::COL_COUNT];
//        for (int j = 0; j < RenderConfig::COL_COUNT; ++j)
//        {
//            if (i + j < n)
//                kvs[j] = m_data[i + j];
//            else
//                kvs[j] = { L"", L"" };
//        }
//
//        currentY = DrawGridRow4(painter, kvs, currentY, zebra);
//    }
//
//    return currentY;
//}
//// ----------------------------------------------------
//// Modül 3: CMetricBlock - CONSTRUCTOR & RENDER
//// ----------------------------------------------------
//CMetricBlock::CMetricBlock(PdfFont* dFont, PdfFont* bFont)
//    : m_defaultFont(dFont), m_boldFont(bFont) {
//}
//
//void CMetricBlock::DrawCheckBox(PdfPainter& painter, float x, float y, const CString& label, float size)
//{
//    float boxSize = size * 0.8f;
//    float drawY = RenderConfig::P_A4H - y;
//
//    // 1. Kutu
//    SetRGBStroke(painter, RenderConfig::COL_META);
//    painter.SetLineWidth(0.5f);
//    painter.DrawRect(x, drawY - boxSize, boxSize, boxSize);
//    painter.Stroke();
//
//    // 2. Etiket
//    painter.SetFont(m_defaultFont);
//    painter.SetFontSize(size);
//    SetRGBFill(painter, RenderConfig::COL_TEXT);
//    painter.DrawText(x + boxSize + 3, drawY - size, CStringtoUTF8(label));
//}
//
//float CMetricBlock::Render(PdfPainter& painter, float currentY)
//{
//    // 1. Başlık Çizimi
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(14.0f);
//    SetRGBFill(painter, RenderConfig::COL_HEADER);
//    painter.DrawText(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY + 5, CStringtoUTF8(L"ODA DAĞILIMI VE METREKARE ÇİZELGESİ"));
//    currentY += 20.0f;
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.5f);
//    painter.DrawLine(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY, RenderConfig::P_A4W - RenderConfig::MARGIN, RenderConfig::P_A4H - currentY);
//    painter.Stroke();
//    currentY += 10.0f;
//
//    const float X = RenderConfig::MARGIN;
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float RowH = RenderConfig::ROW_HEIGHT;
//
//    // Kolon Ayırıcıları
//    float cRoom = X + W * 0.35f;
//    float cM2 = X + W * 0.55f;
//
//    // 2. Başlık Satırı
//    float yHeader = currentY;
//    SetRGBFill(painter, RenderConfig::COL_ZEBRA);
//    painter.DrawRect(X, RenderConfig::P_A4H - yHeader, W, RowH);
//    painter.Fill();
//
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(11.0f);
//    SetRGBFill(painter, RenderConfig::COL_HEADER);
//
//    painter.DrawText(X + RenderConfig::PADDING, RenderConfig::P_A4H - yHeader + 5, CStringtoUTF8(L"Oda / Mekan"));
//    painter.DrawText(cRoom + RenderConfig::PADDING, RenderConfig::P_A4H - yHeader + 5, CStringtoUTF8(L"m²"));
//    painter.DrawText(cM2 + RenderConfig::PADDING, RenderConfig::P_A4H - yHeader + 5, CStringtoUTF8(L"Not / İşaretler"));
//
//    currentY += RowH; // Başlık bitti
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.0f);
//    painter.DrawLine(X, RenderConfig::P_A4H - currentY, X + W, RenderConfig::P_A4H - currentY);
//    painter.Stroke();
//
//    // 3. Oda Listesi
//    std::vector<CString> rooms = { L"Yatak Odası", L"Salon", L"Mutfak", L"Banyo", L"Giyinme Odası", L"Depo" };
//    bool zebra = false;
//
//    for (const auto& roomName : rooms)
//    {
//        zebra = !zebra;
//        float yRow = currentY;
//        float drawY = RenderConfig::P_A4H - yRow;
//
//        if (zebra) {
//            SetRGBFill(painter, RenderConfig::COL_ZEBRA);
//            painter.DrawRect(X, drawY - RowH, W, RowH);
//            painter.Fill();
//        }
//
//        // Oda Adı
//        painter.SetFont(m_defaultFont);
//        painter.SetFontSize(10.0f);
//        SetRGBFill(painter, RenderConfig::COL_TEXT);
//        painter.DrawText(X + RenderConfig::PADDING, drawY - 12, CStringtoUTF8(roomName));
//
//        // m² Çizgisi
//        SetRGBStroke(painter, RenderConfig::COL_META);
//        painter.SetLineWidth(0.5f);
//        painter.DrawLine(cRoom + RenderConfig::PADDING, drawY - 8, cM2 - RenderConfig::PADDING, drawY - 8);
//        painter.Stroke();
//
//        // Checkboxlar
//        float cbX = cM2 + RenderConfig::PADDING;
//        DrawCheckBox(painter, cbX, yRow + 10, L"Banyo", 9.0f);
//        DrawCheckBox(painter, cbX + 70, yRow + 10, L"Balkon", 9.0f);
//
//        currentY += RowH;
//
//        // Alt Çizgi
//        SetRGBStroke(painter, RenderConfig::COL_LINE);
//        painter.SetLineWidth(0.5f);
//        painter.DrawLine(X, RenderConfig::P_A4H - currentY, X + W, RenderConfig::P_A4H - currentY);
//        painter.Stroke();
//    }
//    return currentY;
//}
//// ----------------------------------------------------
//// Modül 4: CNotesBlock - CONSTRUCTOR & RENDER
//// ----------------------------------------------------
//CNotesBlock::CNotesBlock(PdfFont* dFont, const CString& notes)
//    : m_defaultFont(dFont), m_notes(notes) {
//}
//
//float CNotesBlock::Render(PdfPainter& painter, float currentY)
//{
//    if (m_notes.IsEmpty()) return currentY;
//
//    // 1. Başlık Çizimi
//    painter.SetFont(m_boldFont);
//    painter.SetFontSize(14.0f);
//    SetRGBFill(painter, RenderConfig::COL_HEADER);
//    painter.DrawText(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY + 5, CStringtoUTF8(L"DİĞER NOTLAR"));
//    currentY += 20.0f;
//
//    // Alt Çizgi
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.5f);
//    painter.DrawLine(RenderConfig::MARGIN, RenderConfig::P_A4H - currentY, RenderConfig::P_A4W - RenderConfig::MARGIN, RenderConfig::P_A4H - currentY);
//    painter.Stroke();
//    currentY += 10.0f;
//
//    // 2. Metin Kutusu ve Word Wrap
//    const float X = RenderConfig::MARGIN;
//    const float W = RenderConfig::P_A4W - 2 * RenderConfig::MARGIN;
//    const float H = 80.0f;
//
//    float drawY = RenderConfig::P_A4H - currentY;
//
//    // Kutuyu Çiz
//    SetRGBStroke(painter, RenderConfig::COL_LINE);
//    painter.SetLineWidth(1.0f);
//    painter.DrawRect(X, drawY - H, W, H);
//    painter.Stroke();
//
//    // Not Metnini Çiz
//    painter.SetFont(m_defaultFont);
//    painter.SetFontSize(10.0f);
//    SetRGBFill(painter, RenderConfig::COL_TEXT);
//
//    // DrawText, metin kutusu içinde otomatik hizalama (Word Wrap) yapar.
//    painter.DrawText(X + RenderConfig::PADDING, drawY - H + 5, W - 2 * RenderConfig::PADDING, H - 10, CStringtoUTF8(m_notes), PdfPainter::Left | PdfPainter::Top);
//
//    currentY += H;
//    return currentY;
//}