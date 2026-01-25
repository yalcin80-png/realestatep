#include "stdafx.h"
#include "DepositPageLayout.h"
#include "ThemeConfig.h" // Merkezi Tema Ayarlarý

// Font Yardýmcýsý
static FontDesc MakeFont(float sizePt, bool bold = false) {
    return FontDesc(L"Arial", sizePt, bold, false);
}

// =========================================================
// CTOR & DATA
// =========================================================
DepositPageLayout::DepositPageLayout()
{
}

void DepositPageLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_dataMap.clear();
    for (const auto& p : fields) {
        m_dataMap[ToW(p.first)] = ToW(p.second);
    }
}

std::wstring DepositPageLayout::ToW(const CString& s) {
    return std::wstring(s.GetString());
}

std::wstring DepositPageLayout::GetW(const wchar_t* key, const wchar_t* def) const {
    auto it = m_dataMap.find(key);
    return (it != m_dataMap.end() && !it->second.empty()) ? it->second : (def ? def : L"");
}

// =========================================================
// RENDER
// =========================================================
void DepositPageLayout::Render(IDrawContext& ctx, int pageNo)
{
    // 1. Sayfa Ayarý ve Beyaz Zemin
    ctx.SetLogicalPageSize(Theme::Page_Width, Theme::Page_Height);
    ctx.FillRect(0, 0, Theme::Page_Width, Theme::Page_Height, Theme::Color_PageBg);

    // 2. Çerçeve
    RenderFrame(ctx);

    // 3. Ýçerik
    if (pageNo == 1) RenderPage1(ctx);
    else if (pageNo == 2) RenderPage2(ctx);
}

void DepositPageLayout::RenderFrame(IDrawContext& ctx)
{
    ctx.SetPen(Theme::Color_BrandRed, 2.0f);
    ctx.DrawRect(Theme::Margin, Theme::Margin, Theme::Content_Width, Theme::Page_Height - 2 * Theme::Margin, true);
}

// =========================================================
// SAYFA 1
// =========================================================
void DepositPageLayout::RenderPage1(IDrawContext& ctx)
{
    int y = Theme::Margin + 40;

    RenderHeader(ctx, y);
    RenderSellerInfo(ctx, y);
    RenderBuyerInfo(ctx, y);
    RenderPropertyShort(ctx, y);
    RenderPaymentPlan(ctx, y);

    RenderFooter(ctx);
}

void DepositPageLayout::RenderHeader(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin;
    int w = Theme::Content_Width;
    int h = Theme::H_Header;

    // Logo (Sol)
    int logoW = 280;
    ctx.FillRect(x, y, logoW, h, Theme::Color_BrandRed);
    ctx.SetFont(MakeFont(28.0f, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(L"REALTY\nWORLD", x, y, logoW, h, DT_CENTER | DT_VCENTER);

    // Baþlýk (Sað)
    ctx.FillRect(x + logoW, y, w - logoW, h, Theme::Color_HeaderBg);
    ctx.SetFont(MakeFont(Theme::Font_H1, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(L"ALIM SATIM VE KOMÝSYON SÖZLEÞMESÝ",
        x + logoW + 40, y, w - logoW - 80, h,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    y += h + 40;
}

// SATICI BÝLGÝLERÝ (GÜNCELLENDÝ)
void DepositPageLayout::RenderSellerInfo(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 20;
    int w = Theme::Content_Width - 40;

    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"SATICI (MÜLK SAHÝBÝ) BÝLGÝLERÝ");
    y += Theme::H_Section;

    int colW = w / 2;
    int rowH = Theme::H_RowStd; // <-- ARTIK 85px (Geniþ)

    DrawFieldBox(ctx, x, y, colW, rowH, L"ADI SOYADI", GetW(L"OwnerName"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"TC KÝMLÝK NO", GetW(L"OwnerTC"));
    y += rowH;

    DrawFieldBox(ctx, x, y, colW, rowH, L"TELEFON", GetW(L"OwnerPhone"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ADRES", GetW(L"OwnerAddress"));
    y += rowH + 20;
}
// ALICI BÝLGÝLERÝ (GÜNCELLENDÝ)
void DepositPageLayout::RenderBuyerInfo(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 20;
    int w = Theme::Content_Width - 40;

    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"ALICI BÝLGÝLERÝ");
    y += Theme::H_Section;

    int colW = w / 2;
    int rowH = Theme::H_RowStd; // <-- ARTIK 85px

    DrawFieldBox(ctx, x, y, colW, rowH, L"ADI SOYADI", L"");
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"TC KÝMLÝK NO", L"");
    y += rowH;

    DrawFieldBox(ctx, x, y, colW, rowH, L"TELEFON", L"");
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ADRES", L"");
    y += rowH + 20;
}
// MÜLK ÖZETÝ (GÜNCELLENDÝ)
void DepositPageLayout::RenderPropertyShort(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 20;
    int w = Theme::Content_Width - 40;

    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"SÖZLEÞMEYE KONU GAYRÝMENKUL");
    y += Theme::H_Section;

    int col3 = w / 3;
    int rowH = Theme::H_RowStd; // <-- ARTIK 85px

    std::wstring cityInfo = GetW(L"City") + L" / " + GetW(L"District");
    std::wstring adaParsel = GetW(L"Ada") + L" / " + GetW(L"Parsel");

    DrawFieldBox(ctx, x, y, col3, rowH, L"ÝL / ÝLÇE", cityInfo);
    DrawFieldBox(ctx, x + col3, y, col3, rowH, L"MAHALLE", GetW(L"Neighborhood"));
    DrawFieldBox(ctx, x + 2 * col3, y, col3, rowH, L"ADA / PARSEL", adaParsel);
    y += rowH + 20;
}
// ÖDEME PLANI (GÜNCELLENDÝ)
void DepositPageLayout::RenderPaymentPlan(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 20;
    int w = Theme::Content_Width - 40;

    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"SATIÞ BEDELÝ VE KAPORA");
    y += Theme::H_Section;

    int rowH = 90; // Fiyat için biraz daha geniþ

    std::wstring price = GetW(L"Price") + L" " + GetW(L"Currency");

    // Fiyat Kutusu
    ctx.FillRect(x, y, w, rowH, Theme::Color_BoxBgAlt);
    ctx.SetPen(Theme::Color_BrandRed, 2.0f);
    ctx.DrawRect(x, y, w, rowH, true);

    ctx.SetFont(MakeFont(Theme::Font_H2, true)); // Label Büyüdü
    ctx.SetBrush(Theme::Color_TextMain);
    ctx.DrawTextW(L"TOPLAM SATIÞ BEDELÝ:", x + 20, y, 400, rowH, DT_LEFT | DT_VCENTER);

    ctx.SetFont(MakeFont(Theme::Font_ValBig, true));
    ctx.SetBrush(Theme::Color_TextAccent);
    ctx.DrawTextW(price, x + 420, y, w - 440, rowH, DT_LEFT | DT_VCENTER);

    y += rowH; // Aradaki boþluðu kaldýrdýk, bitiþik olsun

    // Kapora Satýrý
    int rowK = Theme::H_RowStd;
    DrawFieldBox(ctx, x, y, w / 2, rowK, L"ALINAN KAPORA (CAYMA AKÇESÝ)", L"");
    DrawFieldBox(ctx, x + w / 2, y, w / 2, rowK, L"TARÝH", L".... / .... / 20....");
    y += rowK + 30;
}
void DepositPageLayout::RenderFooter(IDrawContext& ctx)
{
    int h = 90;
    int w = (Theme::Content_Width - 120) / 3;
    int y = Theme::Page_Height - Theme::Margin - h - 30;
    int x = Theme::Margin + 60;

    auto DrawSig = [&](int bx, int by, const wchar_t* title, const std::wstring& name) {
        ctx.FillRect(bx, by, w, h, Theme::Color_BoxBg);
        ctx.SetPen(Theme::Color_BorderLight, 1.0f);
        ctx.DrawRect(bx, by, w, h, true);

        ctx.SetFont(MakeFont(Theme::Font_Label, true));
        ctx.SetBrush(Theme::Color_TextLabel);
        ctx.DrawTextW(title, bx, by + 5, w, 20, DT_CENTER);

        ctx.SetFont(MakeFont(Theme::Font_Val, true));
        ctx.SetBrush(Theme::Color_TextMain);
        ctx.DrawTextW(name, bx, by + 30, w, 30, DT_CENTER);
        };

    DrawSig(x, y, L"SATICI", GetW(L"OwnerName"));
    DrawSig(x + w + 20, y, L"ALICI", L"");
    DrawSig(x + 2 * w + 40, y, L"EMLAK ÝÞLETMESÝ", GetW(L"AgentName"));
}

// =========================================================
// SAYFA 2
// =========================================================
void DepositPageLayout::RenderPage2(IDrawContext& ctx)
{
    int y = Theme::Margin + 60;
    int w = Theme::Content_Width - 40;
    int x = Theme::Margin + 20;

    ctx.SetFont(MakeFont(Theme::Font_H1, true));
    ctx.SetBrush(Theme::Color_BrandRed);
    ctx.DrawTextW(L"SÖZLEÞME ÞARTLARI", x, y, w, 50, DT_CENTER);
    y += 80;

    DrawClause(ctx, x, y, w, L"1. KONU",
        L"Ýþbu sözleþme, yukarýda bilgileri verilen gayrimenkulün satýþý konusunda taraflarýn anlaþtýðýný ve kapora (cayma akçesi) ödemesinin yapýldýðýný belgeler.");

    DrawClause(ctx, x, y, w, L"2. CAYMA TAZMÝNATI",
        L"ALICI, bu sözleþmeyi imzaladýktan sonra alýmdan vazgeçerse, verdiði kaporayý geri talep edemez ve kapora SATICI'ya irad kaydedilir.\n"
        L"SATICI, satýþtan vazgeçerse, aldýðý kaporayý iade etmekle birlikte, kapora miktarý kadar ek tazminatý ALICI'ya ödemeyi kabul eder.");

    DrawClause(ctx, x, y, w, L"3. HÝZMET BEDELÝ (KOMÝSYON)",
        L"Satýþ gerçekleþtiðinde, ALICI %2 + KDV, SATICI %2 + KDV oranýnda hizmet bedelini Emlak Ýþletmesi'ne ödeyecektir.\n"
        L"Taraflardan biri vazgeçerse, vazgeçen taraf hem kendi komisyonunu hem de karþý tarafýn komisyonunu ödemekle yükümlüdür.");

    // Alt Ýmza (Tekrar)
    RenderFooter(ctx);
}

// =========================================================
// YARDIMCILAR
// =========================================================

void DepositPageLayout::DrawSectionHeader(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& title)
{
    ctx.FillRect(x, y, w, h, Theme::Color_HeaderBg);
    ctx.SetFont(MakeFont(Theme::Font_H2, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(title, x + 10, y, w - 20, h, DT_LEFT | DT_VCENTER);
}

void DepositPageLayout::DrawFieldBox(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value)
{
    ctx.FillRect(x, y, w, h, Theme::Color_BoxBg);
    ctx.SetPen(Theme::Color_BorderLight, 1.0f);
    ctx.DrawRect(x, y, w, h, true);

    // --- DÝNAMÝK YÜKSEKLÝK HESABI ---
    int labelH = (int)(Theme::Font_Label * 1.8f);
    if (labelH < 25) labelH = 25;

    if (!label.empty()) {
        ctx.SetFont(MakeFont(Theme::Font_Label, true));
        ctx.SetBrush(Theme::Color_TextLabel);
        ctx.DrawTextW(label, x + 8, y + 4, w - 16, labelH, DT_LEFT | DT_TOP);
    }

    ctx.SetFont(MakeFont(Theme::Font_Val, false));
    ctx.SetBrush(Theme::Color_TextMain);

    int valY = label.empty() ? y : (y + labelH);
    int valH = label.empty() ? h : (h - labelH - 4);

    if (!value.empty()) {
        ctx.DrawTextW(value, x + 8, valY, w - 16, valH, DT_LEFT | DT_VCENTER);
    }
}
void DepositPageLayout::DrawClause(IDrawContext& ctx, int x, int& y, int w, const std::wstring& title, const std::wstring& content)
{
    // Madde Baþlýðý
    ctx.SetFont(MakeFont(Theme::Font_H2, true));
    ctx.SetBrush(Theme::Color_BrandRed);
    ctx.DrawTextW(title, x, y, w, 30, DT_LEFT);
    y += 35;

    // Ýçerik
    ctx.SetFont(MakeFont(Theme::Font_Val, false));
    ctx.SetBrush(Theme::Color_TextMain);

    // Basit yükseklik tahmini (Karakter sayýsý / Ortalama satýr uzunluðu)
    int estH = (content.length() / 90 + 2) * 25;

    ctx.DrawTextW(content, x, y, w, estH, DT_LEFT | DT_WORDBREAK);
    y += estH + 20;
}