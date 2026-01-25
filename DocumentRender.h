#ifndef DOC_DOCUMEN_TRENDER_H
#define DOC_DOCUMEN_TRENDER_H

#pragma once

#include "stdafx.h"
#include "IDrawContext.h"
#include "PreviewItem.h"
#include <map>
#include <vector>

// =========================================================
// PROFESYONEL RENK TANIMLARI (SABÝT)
// =========================================================
// Not: RGB() makrosunun projedeki tanýmýna uyumlu kullanýyoruz.
#define CLR_WHITE       Color4f(RGB(255, 255, 255))
#define CLR_OFF_WHITE   Color4f(RGB(250, 250, 250)) // Çok çok açýk gri
#define CLR_NAVY        Color4f(RGB(30, 40, 60))    // Kurumsal Lacivert
#define CLR_RED         Color4f(RGB(190, 0, 0))     // Realty World Kýrmýzýsý
#define CLR_BORDER      Color4f(RGB(200, 200, 200)) // Ýnce Gri Çizgi
#define CLR_TXT_MAIN    Color4f(RGB(40, 40, 40))    // Koyu Gri Yazý
#define CLR_TXT_LABEL   Color4f(RGB(120, 120, 120)) // Açýk Gri Etiket
#define CLR_TXT_BLUE    Color4f(RGB(0, 50, 140))    // Vurgu Mavisi

class DocumentRender
{
public:
    DocumentRender();

    void SetData(const PreviewItem& item);
    void Render(IDrawContext& ctx);

private:
    PreviewItem m_item;
    std::map<CString, CString> m_dataMap;

    CString GetVal(const CString& key);
    std::wstring ToW(const CString& s);
    FontDesc MakeFont(float size, bool bold = false, bool italic = false);

    // Render Parçalarý
    void RenderCommonHeader(IDrawContext& ctx, int& y);
    void RenderFooter(IDrawContext& ctx, int& y);

    void RenderOwnerInfo(IDrawContext& ctx, int& y);
    void RenderPropertyInfo(IDrawContext& ctx, int& y);
    void RenderContractTerms(IDrawContext& ctx, int& y);

    // Çizim Araçlarý
    void DrawSectionTitle(IDrawContext& ctx, int x, int y, int w, const std::wstring& title);
    void DrawLabelValue(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value);
    void DrawCheckBox(IDrawContext& ctx, int x, int y, const std::wstring& label, bool checked);

    static const int PAGE_W = 2100;
    static const int PAGE_H = 2970;
    static const int MARGIN = 80; // Kenar boþluðunu biraz daralttým (daha geniþ kullaným)
};

// =========================================================
// IMPLEMENTASYON
// =========================================================

DocumentRender::DocumentRender() {}

void DocumentRender::SetData(const PreviewItem& item)
{
    m_item = item;
    m_dataMap.clear();
    for (const auto& pair : item.fields) {
        m_dataMap[pair.first] = pair.second;
    }
}

CString DocumentRender::GetVal(const CString& key) {
    auto it = m_dataMap.find(key);
    return (it != m_dataMap.end()) ? it->second : CString(_T(""));
}

std::wstring DocumentRender::ToW(const CString& s) {
#ifdef UNICODE
    return std::wstring(s.GetString());
#else
    if (s.IsEmpty()) return L"";
    int len = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, s, -1, &buf[0], len);
    return std::wstring(&buf[0]);
#endif
}

FontDesc DocumentRender::MakeFont(float size, bool bold, bool italic) {
    // Yazý tipi boyutu çok büyük gelirse buradan düþürebilirsin
    return FontDesc(L"Arial", size, bold, italic);
}

// ---------------------------------------------------------
// ANA RENDER
// ---------------------------------------------------------
void DocumentRender::Render(IDrawContext& ctx)
{
    // 1. ZEMÝNÝ BEYAZA BOYA (Siyah ekraný önler)
    ctx.SetLogicalPageSize(PAGE_W, PAGE_H);
    ctx.FillRect(0, 0, PAGE_W, PAGE_H, CLR_WHITE);

    // 2. DIÞ ÇERÇEVE (Ýnce Kýrmýzý)
    ctx.SetPen(CLR_RED, 2.0f);
    ctx.DrawRect(MARGIN, MARGIN, PAGE_W - 2 * MARGIN, PAGE_H - 2 * MARGIN, true);

    int y = MARGIN + 40;

    RenderCommonHeader(ctx, y);

    // Form içeriði
    RenderOwnerInfo(ctx, y);
    RenderPropertyInfo(ctx, y);
    RenderContractTerms(ctx, y);

    RenderFooter(ctx, y);
}

// ---------------------------------------------------------
// HEADER (KURUMSAL TASARIM)
// ---------------------------------------------------------
void DocumentRender::RenderCommonHeader(IDrawContext& ctx, int& y)
{
    int x = MARGIN;
    int w = PAGE_W - 2 * MARGIN;
    int h = 140;

    // LOGO KUTUSU (SOL - KIRMIZI)
    int logoW = 300;
    ctx.FillRect(x, y, logoW, h, CLR_RED);

    ctx.SetFont(MakeFont(32, true));
    ctx.SetBrush(CLR_WHITE);
    ctx.DrawTextW(L"REALTY\nWORLD", x, y, logoW, h, DT_CENTER | DT_VCENTER);

    // BAÞLIK KUTUSU (SAÐ - LACÝVERT)
    ctx.FillRect(x + logoW, y, w - logoW, h, CLR_NAVY);

    // Dinamik Baþlýk
    CString title = _T("BELGE");
    if (m_item.docType == DOC_CONTRACT_SALES_AUTH) title = _T("SATILIK YETKÝLENDÝRME SÖZLEÞMESÝ");
    else if (m_item.docType == DOC_CONTRACT_RENTAL_AUTH) title = _T("KÝRALIK YETKÝLENDÝRME SÖZLEÞMESÝ");
    else if (!m_item.title.IsEmpty()) title = m_item.title;

    ctx.SetFont(MakeFont(36, true)); // Büyük Baþlýk
    ctx.SetBrush(CLR_WHITE);
    ctx.DrawTextW(ToW(title), x + logoW + 40, y, w - logoW - 80, h, DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    y += h + 20;

    // ALT BÝLGÝ SATIRI (Ofis Adý - Tarih)
    int rowH = 30;

    // Ofis Adý (Siyah Kalýn)
    ctx.SetFont(MakeFont(14, true));
    ctx.SetBrush(CLR_TXT_MAIN);
    ctx.DrawTextW(ToW(GetVal(_T("AgentName"))), x + 20, y, 600, rowH, DT_LEFT | DT_VCENTER);

    // Tarih ve No (Saða Yaslý)
    ctx.SetFont(MakeFont(12, true));
    ctx.SetBrush(CLR_TXT_LABEL); // Gri Etiket
    ctx.DrawTextW(L"BELGE NO:", x + w - 400, y, 100, rowH, DT_RIGHT | DT_VCENTER);
    ctx.SetBrush(CLR_TXT_MAIN); // Siyah Deðer
    ctx.DrawTextW(L"..................", x + w - 290, y, 150, rowH, DT_LEFT | DT_VCENTER);

    y += rowH + 20;
}

// ---------------------------------------------------------
// ÝÞ SAHÝBÝ
// ---------------------------------------------------------
void DocumentRender::RenderOwnerInfo(IDrawContext& ctx, int& y)
{
    int w = PAGE_W - 2 * MARGIN;

    DrawSectionTitle(ctx, MARGIN, y, w, L"ÝÞ SAHÝBÝ BÝLGÝLERÝ");
    y += 50; // Baþlýk ile kutular arasý boþluk

    // Tüzel / Gerçek Kiþi Checkbox
    bool isCorp = (GetVal(_T("IsCorporate")) == _T("1"));
    DrawCheckBox(ctx, MARGIN + 10, y, L"GERÇEK KÝÞÝ", !isCorp);
    DrawCheckBox(ctx, MARGIN + 300, y, L"TÜZEL KÝÞÝ", isCorp);
    y += 40;

    int hw = w / 2;
    int h = 60; // Kutu yüksekliði

    DrawLabelValue(ctx, MARGIN, y, hw, h, L"ADI SOYADI", ToW(GetVal(_T("OwnerName"))));
    DrawLabelValue(ctx, MARGIN + hw, y, hw, h, L"TC / VERGÝ NO", ToW(GetVal(_T("OwnerTC"))));
    y += h;

    DrawLabelValue(ctx, MARGIN, y, hw, h, L"TELEFON", ToW(GetVal(_T("OwnerPhone"))));
    DrawLabelValue(ctx, MARGIN + hw, y, hw, h, L"E-POSTA", ToW(GetVal(_T("OwnerEmail"))));
    y += h;

    DrawLabelValue(ctx, MARGIN, y, w, h, L"ADRES", ToW(GetVal(_T("OwnerAddress"))));
    y += h + 30; // Bölüm sonu boþluðu
}

// ---------------------------------------------------------
// MÜLK BÝLGÝLERÝ
// ---------------------------------------------------------
void DocumentRender::RenderPropertyInfo(IDrawContext& ctx, int& y)
{
    int w = PAGE_W - 2 * MARGIN;

    DrawSectionTitle(ctx, MARGIN, y, w, L"GAYRÝMENKUL BÝLGÝLERÝ");
    y += 50;

    int c3 = w / 3;
    int h = 60;

    DrawLabelValue(ctx, MARGIN, y, c3, h, L"ÝLÝ", ToW(GetVal(_T("City"))));
    DrawLabelValue(ctx, MARGIN + c3, y, c3, h, L"ÝLÇESÝ", ToW(GetVal(_T("District"))));
    DrawLabelValue(ctx, MARGIN + 2 * c3, y, c3, h, L"MAHALLE", ToW(GetVal(_T("Neighborhood"))));
    y += h;

    DrawLabelValue(ctx, MARGIN, y, c3, h, L"ADA", ToW(GetVal(_T("Ada"))));
    DrawLabelValue(ctx, MARGIN + c3, y, c3, h, L"PARSEL", ToW(GetVal(_T("Parsel"))));
    DrawLabelValue(ctx, MARGIN + 2 * c3, y, c3, h, L"ALAN (m2)", ToW(GetVal(_T("MapArea"))));
    y += h;

    DrawLabelValue(ctx, MARGIN, y, w, h, L"AÇIK ADRES", ToW(GetVal(_T("FullAddress"))));
    y += h + 30;
}

// ---------------------------------------------------------
// ÞARTLAR VE FÝYAT
// ---------------------------------------------------------
void DocumentRender::RenderContractTerms(IDrawContext& ctx, int& y)
{
    int w = PAGE_W - 2 * MARGIN;

    DrawSectionTitle(ctx, MARGIN, y, w, L"HÝZMET BEDELÝ VE KOÞULLAR");
    y += 50;

    // FÝYAT KUTUSU (Göz Alýcý Yapýyoruz)
    std::wstring priceLabel = (m_item.docType == DOC_CONTRACT_RENTAL_AUTH) ? L"KÝRA BEDELÝ (LÝSTE)" : L"SATIÞ BEDELÝ (LÝSTE)";

    // Hafif gri zemin, Lacivert çerçeve
    ctx.FillRect(MARGIN, y, w, 80, CLR_OFF_WHITE);
    ctx.SetPen(CLR_NAVY, 1.5f);
    ctx.DrawRect(MARGIN, y, w, 80, false);

    // Etiket
    ctx.SetFont(MakeFont(12, true));
    ctx.SetBrush(CLR_TXT_LABEL);
    ctx.DrawTextW(priceLabel, MARGIN + 20, y + 15, 300, 30, DT_LEFT);

    // Deðer (Mavi ve Büyük)
    ctx.SetFont(MakeFont(28, true));
    ctx.SetBrush(CLR_TXT_BLUE);
    ctx.DrawTextW(ToW(GetVal(_T("Price"))) + L" " + ToW(GetVal(_T("Currency"))),
        MARGIN + 20, y + 35, w - 40, 45, DT_LEFT | DT_VCENTER);

    y += 100;

    // Metinler
    std::wstring terms;
    if (m_item.docType == DOC_CONTRACT_SALES_AUTH) {
        terms = L"1. Ýþ sahibi, yukarýda belirtilen gayrimenkulün pazarlanmasý ve satýþý konusunda Emlak Ýþletmesini yetkilendirmiþtir.\n\n"
            L"2. Hizmet Bedeli: Satýþ gerçekleþmesi durumunda, satýþ bedelinin %2 + KDV tutarý hizmet bedeli olarak ödenir.\n\n"
            L"3. Yetki Süresi: Bu sözleþme imza tarihinden itibaren 90 gün süreyle geçerlidir.";
    }
    else {
        terms = L"Standart yetkilendirme þartlarý geçerlidir.\nMadde 1...\nMadde 2...";
    }

    ctx.SetFont(MakeFont(12, false));
    ctx.SetBrush(CLR_TXT_MAIN);
    ctx.DrawTextW(terms, MARGIN, y, w, 400, DT_LEFT | DT_TOP | DT_WORDBREAK);
}

// ---------------------------------------------------------
// ÝMZALAR
// ---------------------------------------------------------
void DocumentRender::RenderFooter(IDrawContext& ctx, int& y)
{
    // Sayfanýn en altýna sabitleyelim
    y = PAGE_H - MARGIN - 150;

    int w = PAGE_W - 2 * MARGIN;
    int boxW = w / 2 - 40;
    int boxH = 100;

    auto DrawSigBox = [&](int bx, int by, CString role, CString name) {
        // Ýmza Kutusu Çerçevesi
        ctx.SetPen(CLR_BORDER, 1.0f);
        ctx.FillRect(bx, by, boxW, boxH, CLR_WHITE);
        ctx.DrawRect(bx, by, boxW, boxH, false);

        // Baþlýk
        ctx.SetFont(MakeFont(10, true));
        ctx.SetBrush(CLR_TXT_LABEL);
        ctx.DrawTextW(ToW(role), bx, by + 5, boxW, 20, DT_CENTER);

        // Ýsim
        ctx.SetFont(MakeFont(11, false));
        ctx.SetBrush(CLR_TXT_MAIN);
        ctx.DrawTextW(ToW(name), bx, by + 30, boxW, 30, DT_CENTER);

        // Ýmza Yeri
        ctx.SetFont(MakeFont(8, false));
        ctx.SetBrush(CLR_TXT_LABEL);
        ctx.DrawTextW(L"(Ýmza / Kaþe)", bx, by + boxH - 20, boxW, 20, DT_CENTER);
        };

    DrawSigBox(MARGIN, y, _T("EMLAK ÝÞLETMESÝ"), GetVal(_T("AgentName")));
    DrawSigBox(PAGE_W - MARGIN - boxW, y, _T("ÝÞ SAHÝBÝ"), GetVal(_T("OwnerName")));
}

// ---------------------------------------------------------
// YARDIMCI FONKSÝYONLAR (DÜZELTÝLDÝ)
// ---------------------------------------------------------

// Baþlýk Çizici - Siyah yerine Lacivert
void DocumentRender::DrawSectionTitle(IDrawContext& ctx, int x, int y, int w, const std::wstring& title) {
    ctx.FillRect(x, y, w, 40, CLR_NAVY); // Lacivert Zemin

    ctx.SetFont(MakeFont(14, true));
    ctx.SetBrush(CLR_WHITE); // Beyaz Yazý
    ctx.DrawTextW(title, x + 15, y, w - 30, 40, DT_LEFT | DT_VCENTER);
}

// Veri Kutusu - Siyah Þerit Yerine Temiz Kutu
void DocumentRender::DrawLabelValue(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value) {
    // Kutu Zemini BEYAZ
    ctx.FillRect(x, y, w, h, CLR_WHITE);
    // Ýnce Gri Çerçeve
    ctx.SetPen(CLR_BORDER, 1.0f);
    ctx.DrawRect(x, y, w, h, false); // false = fill yapma, sadece çizgi çek

    // Etiket (Küçük, Gri, Üstte)
    int labelH = 20;
    ctx.SetFont(MakeFont(9, true));
    ctx.SetBrush(CLR_TXT_LABEL);
    ctx.DrawTextW(label, x + 8, y + 4, w - 16, labelH, DT_LEFT);

    // Deðer (Normal, Siyah, Altta)
    ctx.SetFont(MakeFont(12, false));
    ctx.SetBrush(CLR_TXT_MAIN);

    // Eðer deðer boþsa bile kutu çizilsin
    if (!value.empty()) {
        ctx.DrawTextW(value, x + 8, y + 18, w - 16, h - 20, DT_LEFT | DT_VCENTER | DT_WORDBREAK);
    }
}

void DocumentRender::DrawCheckBox(IDrawContext& ctx, int x, int y, const std::wstring& label, bool checked) {
    // Kutu
    ctx.SetPen(CLR_TXT_MAIN, 1.0f);
    ctx.FillRect(x, y, 20, 20, CLR_WHITE);
    ctx.DrawRect(x, y, 20, 20, false);

    // Tik Ýþareti
    if (checked) {
        ctx.SetFont(MakeFont(14, true));
        ctx.SetBrush(CLR_TXT_MAIN);
        ctx.DrawTextW(L"X", x, y, 20, 20, DT_CENTER | DT_VCENTER);
    }

    // Etiket
    ctx.SetFont(MakeFont(12, false));
    ctx.SetBrush(CLR_TXT_MAIN);
    ctx.DrawTextW(label, x + 25, y, 200, 20, DT_LEFT | DT_VCENTER);
}

#endif