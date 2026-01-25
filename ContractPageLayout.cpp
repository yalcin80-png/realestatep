#include "stdafx.h"
#include "ContractPageLayout.h"
#include "ThemeConfig.h" 

// Font Helper
static FontDesc MakeFont(float sizePt, bool bold = false) {
    return FontDesc(L"Arial", sizePt, bold, false);
}

// =========================================================
// CTOR & DATA
// =========================================================
ContractPageLayout::ContractPageLayout()
{
}
#include <algorithm> // transform için

// ... (Diðer kodlar ayný kalacak) ...

// 1. DÜZELTME: ToUpperW Hatasý Ýçin Kesin Çözüm (WinAPI)
static std::wstring ToUpperW(const std::wstring& s) {
    if (s.empty()) return std::wstring();
    std::wstring up = s;
    if (up.length() > 0) {
        ::CharUpperBuffW(&up[0], (DWORD)up.length());
    }
    return up;
}





// ... (Diðer fonksiyonlar ayný kalsýn) ...

void ContractPageLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_dataMap.clear();

    for (const auto& kv : fields)
    {
        std::wstring rawKey = ToW(kv.first); // SQL'den gelen orjinal isim (Örn: OwnerName)
        std::wstring val = ToW(kv.second);   // Deðer (Örn: Ahmet Yýlmaz)

        // Anahtarý BÜYÜK HARFE çevir (Eþleþme garantisi için)
        std::wstring key = ToUpperW(rawKey);

        // 1. ORJÝNAL VERÝYÝ SAKLA (Yedek)
        m_dataMap[rawKey] = val;

        // 2. AKILLI EÞLEÞTÝRME (MAPPING)
        // SQL'den ne gelirse gelsin, Formun beklediði anahtara yönlendiriyoruz.

        // --- MÜÞTERÝ BÝLGÝLERÝ ---
        if (key == L"OWNERNAME" || key == L"ADSOYAD" || key == L"MUSTERI") {
            m_dataMap[L"OwnerName"] = val;
            // "Malikler" kutusu boþ kalmasýn diye Sahip Adýný oraya da kopyalýyoruz
            if (m_dataMap[L"Owners"].empty()) m_dataMap[L"Owners"] = val;
        }

        if (key == L"OWNERTC" || key == L"TC" || key == L"TCKN")           m_dataMap[L"OwnerTC"] = val;
        if (key == L"OWNERPHONE" || key == L"TELEFON" || key == L"CEP")    m_dataMap[L"OwnerPhone"] = val;
        if (key == L"OWNEREMAIL" || key == L"EMAIL")                       m_dataMap[L"OwnerEmail"] = val;
        if (key == L"OWNERADDRESS" || key == L"SAHIP_ADRES")               m_dataMap[L"OwnerAddress"] = val;
        if (key == L"OWNERCOMPANY" || key == L"FIRMA" || key == L"SIRKET") m_dataMap[L"OwnerCompany"] = val;

        // --- MÜLK ADRESÝ ---
        if (key == L"CITY" || key == L"IL" || key == L"SEHIR")             m_dataMap[L"City"] = val;
        if (key == L"DISTRICT" || key == L"ILCE")                          m_dataMap[L"District"] = val;
        if (key == L"NEIGHBORHOOD" || key == L"MAHALLE")                   m_dataMap[L"Neighborhood"] = val;
        if (key == L"VILLAGE" || key == L"KOY")                            m_dataMap[L"Village"] = val;

        if (key == L"FULLADDRESS" || key == L"ADRES" || key == L"ACIKADRES") m_dataMap[L"FullAddress"] = val;

        // --- TAPU BÝLGÝLERÝ ---
        if (key == L"PAFTA")        m_dataMap[L"Pafta"] = val;
        if (key == L"ADA")          m_dataMap[L"Ada"] = val;
        if (key == L"PARSEL")       m_dataMap[L"Parsel"] = val;

        if (key == L"MAPAREA" || key == L"ALAN" || key == L"M2" || key == L"ARSAALANI")
            m_dataMap[L"MapArea"] = val;

        if (key == L"ANANITELIK" || key == L"NITELIK" || key == L"TAPUCINSI")
            m_dataMap[L"AnaNitelik"] = val;

        if (key == L"BBNO" || key == L"KAPINO" || key == L"DAIRENO")
            m_dataMap[L"BBNo"] = val;

        // DÜZELTME: SQL'den BOLUMNITELIK geliyor, Form BBNITELIK bekliyorsa:
        if (key == L"BOLUMNITELIK" || key == L"BBNITELIK" || key == L"KAT")
            m_dataMap[L"BolumNitelik"] = val;

        // --- FÝYAT ---
        if (key == L"PRICE" || key == L"FIYAT")                            m_dataMap[L"Price"] = val;
        if (key == L"CURRENCY" || key == L"PARABIRIMI" || key == L"PB")    m_dataMap[L"Currency"] = val;

        // --- EMLAK TÝPÝ (DÖNÜÞÜM) ---
        if (key == L"PROPERTYTYPE" || key == L"EMLAKTIPI" || key == L"TUR")
        {
            std::wstring uVal = ToUpperW(val);
            if (uVal.find(L"DAIRE") != std::string::npos ||
                uVal.find(L"KONUT") != std::string::npos ||
                uVal.find(L"VILLA") != std::string::npos)
            {
                m_dataMap[L"PropertyType"] = L"KONUT";
            }
            else if (uVal.find(L"ISYERI") != std::string::npos ||
                uVal.find(L"DUKKAN") != std::string::npos ||
                uVal.find(L"OFIS") != std::string::npos)
            {
                m_dataMap[L"PropertyType"] = L"TICARI";
            }
            else if (uVal.find(L"ARSA") != std::string::npos ||
                uVal.find(L"TARLA") != std::string::npos ||
                uVal.find(L"ARAZI") != std::string::npos)
            {
                m_dataMap[L"PropertyType"] = L"ARSA";
            }
            else {
                // Eþleþme yoksa gelen veriyi sakla
                m_dataMap[L"PropertyType"] = uVal;
            }
        }

        // --- DÝÐER DETAYLAR ---
        if (key == L"NETAREA" || key == L"NETM2")          m_dataMap[L"NetArea"] = val;
        if (key == L"GROSSAREA" || key == L"BRUTM2")       m_dataMap[L"GrossArea"] = val;
        if (key == L"BUILDINGAGE" || key == L"BINAYASI")   m_dataMap[L"BuildingAge"] = val;

        if (key == L"USAGESTATUS" || key == L"KULLANIM" || key == L"KULLANIMDURUMU")
            m_dataMap[L"UsageStatus"] = val;

        // ÝSKAN (Evet/Hayýr -> 1/0)
        if (key == L"ISISKANVAR" || key == L"ISKAN" || key == L"KREDIYEUYGUN")
        {
            std::wstring uVal = ToUpperW(val);
            m_dataMap[L"IsIskanVar"] = (uVal == L"1" || uVal == L"EVET" || uVal == L"VAR" || uVal == L"UYGUN") ? L"1" : L"0";
        }
    }
}
// ... (Render fonksiyonlarý ayný kalacak) ...
// =========================================================
// RENDER (ANA DÖNGÜ)
// =========================================================
void ContractPageLayout::Render(IDrawContext& ctx, int pageNo)
{
    ctx.SetLogicalPageSize(Theme::Page_Width, Theme::Page_Height);
    ctx.FillRect(0, 0, Theme::Page_Width, Theme::Page_Height, Theme::Color_PageBg);
    RenderFrame(ctx);

    if (pageNo <= 1)      RenderPage1(ctx);
    else if (pageNo == 2) RenderPage2(ctx);
    else                  RenderPage3(ctx);
}

void ContractPageLayout::RenderFrame(IDrawContext& ctx)
{
    ctx.SetPen(Theme::Color_BrandRed, 2.0f);
    ctx.DrawRect(Theme::Margin, Theme::Margin, Theme::Content_Width, Theme::Page_Height - 2 * Theme::Margin, true);
}

// =========================================================
// SAYFA 1
// =========================================================
void ContractPageLayout::RenderPage1(IDrawContext& ctx)
{
    int y = Theme::Margin + 40;

    RenderHeader(ctx, y);
    RenderOwnerSection(ctx, y); y += 20;
    RenderOfficeSection(ctx, y); y += 20;
    RenderDeedSection(ctx, y); y += 20;
    RenderAddressSection(ctx, y); y += 20;
    RenderPropertyTypeSection(ctx, y); y += 20;
    RenderFooterPage1(ctx, y);
}

// BAÞLIK
void ContractPageLayout::RenderHeader(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin;
    int w = Theme::Content_Width;
    int h = Theme::H_Header;

    // Logo
    int logoW = 280;
    ctx.FillRect(x, y, logoW, h, Theme::Color_BrandRed);
    ctx.SetFont(MakeFont(28.0f, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(L"REALTY\nWORLD", x, y, logoW, h, DT_CENTER | DT_VCENTER);

    // Baþlýk
    ctx.FillRect(x + logoW, y, w - logoW, h, Theme::Color_HeaderBg);
    ctx.SetFont(MakeFont(Theme::Font_H1, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(L"SATILIK YETKÝLENDÝRME SÖZLEÞMESÝ",
        x + logoW + 40, y, w - logoW - 80, h,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK);

    y += h + 20;

    // Alt Bilgi
    std::wstring officeName = GetW(L"AgentName", L"GAZÝÞEHÝR GAYRÝMENKUL");
    std::wstring recordCode = GetW(L"RecordCode", L".................");
    std::wstring dateStr = GetW(L"ContractDate", L"..../..../20....");

    int rowH = 30;
    ctx.SetFont(MakeFont(Theme::Font_H2, true));
    ctx.SetBrush(Theme::Color_TextMain);
    ctx.DrawTextW(officeName, x + 20, y, 600, rowH, DT_LEFT | DT_VCENTER);

    int rightX = x + w - 450;
    ctx.SetFont(MakeFont(Theme::Font_Label, true));
    ctx.SetBrush(Theme::Color_TextLabel);
    ctx.DrawTextW(L"SÖZLEÞME NO:", rightX, y, 140, rowH, DT_RIGHT | DT_VCENTER);

    ctx.SetFont(MakeFont(Theme::Font_Val, true));
    ctx.SetBrush(Theme::Color_TextMain);
    ctx.DrawTextW(recordCode, rightX + 150, y, 100, rowH, DT_LEFT | DT_VCENTER);
    ctx.DrawTextW(dateStr, rightX + 260, y, 180, rowH, DT_RIGHT | DT_VCENTER);

    y += rowH + 10;
    ctx.SetPen(Theme::Color_BrandRed, 2.0f);
    ctx.DrawLine(x, y, x + w, y);
    y += 30;
}

// BÖLÜMLER
void ContractPageLayout::RenderOwnerSection(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"ÝÞ SAHÝBÝ BÝLGÝLERÝ");
    y += Theme::H_Section;

    int colW = w / 2;
    int rowH = Theme::H_RowStd;

    DrawFieldBox(ctx, x, y, colW, rowH, L"ADI SOYADI", GetW(L"OwnerName"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"T.C. / KÝMLÝK NO", GetW(L"OwnerTC"));
    y += rowH;
    DrawFieldBox(ctx, x, y, colW, rowH, L"FÝRMA ADI", GetW(L"OwnerCompany"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"TELEFON", GetW(L"OwnerPhone"));
    y += rowH;
    DrawFieldBox(ctx, x, y, colW, rowH, L"E-POSTA", GetW(L"OwnerEmail"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ADRES", GetW(L"OwnerAddress"));
    y += rowH;
}

void ContractPageLayout::RenderOfficeSection(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"EMLAK ÝÞLETMESÝ BÝLGÝLERÝ");
    y += Theme::H_Section;

    int colW = w / 2;
    int rowH = Theme::H_RowStd;
    DrawFieldBox(ctx, x, y, colW, rowH, L"YETKÝLÝ DANIÞMAN", GetW(L"AgentAuthName"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"TELEFON", GetW(L"AgentPhone"));
    y += rowH;
    DrawFieldBox(ctx, x, y, colW, rowH, L"ÜNVANI", GetW(L"AgentTitle"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ADRES", GetW(L"AgentAddress"));
    y += rowH;
}

void ContractPageLayout::RenderDeedSection(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"GAYRÝMENKULÜN TAPU KAYDI");
    y += Theme::H_Section;

    int colW = w / 4;
    int rowH = Theme::H_RowStd;

    DrawFieldBox(ctx, x, y, colW, rowH, L"ÝLÝ", GetW(L"City"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ÝLÇESÝ", GetW(L"District"));
    DrawFieldBox(ctx, x + colW * 2, y, colW, rowH, L"MAHALLESÝ", GetW(L"Neighborhood"));
    DrawFieldBox(ctx, x + colW * 3, y, colW, rowH, L"KÖYÜ", GetW(L"Village"));
    y += rowH;

    DrawFieldBox(ctx, x, y, colW, rowH, L"PAFTA", GetW(L"Pafta"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"ADA", GetW(L"Ada"));
    DrawFieldBox(ctx, x + colW * 2, y, colW, rowH, L"PARSEL", GetW(L"Parsel"));
    DrawFieldBox(ctx, x + colW * 3, y, colW, rowH, L"ALAN (m2)", GetW(L"MapArea"));
    y += rowH;

    DrawFieldBox(ctx, x, y, colW, rowH, L"ANA GAYRÝMENKUL NÝT.", GetW(L"AnaNitelik"));
    DrawFieldBox(ctx, x + colW, y, colW, rowH, L"B.B. NO", GetW(L"BBNo"));
    DrawFieldBox(ctx, x + colW * 2, y, colW * 2, rowH, L"BÖLÜM NÝTELÝÐÝ", GetW(L"BolumNitelik"));
    y += rowH;
}

void ContractPageLayout::RenderAddressSection(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"GAYRÝMENKUL ADRESÝ VE MALÝKLER");
    y += Theme::H_Section;
    DrawFieldBox(ctx, x, y, w, Theme::H_RowStd, L"AÇIK ADRES", GetW(L"FullAddress"));
    y += Theme::H_RowStd;
    DrawFieldBox(ctx, x, y, w, Theme::H_RowStd, L"MALÝKLER", GetW(L"Owners"));
    y += Theme::H_RowStd;
}

// -------------------------------------------------------------------------
// CHECKBOX ALANI (GÜNCELLENDÝ: HÝZALAMA DÜZELTÝLDÝ)
// -------------------------------------------------------------------------
void ContractPageLayout::RenderPropertyTypeSection(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"GAYRÝMENKULÜN CÝNSÝ VE DURUMU");
    y += Theme::H_Section + 10;

    int rowH = Theme::H_RowStd;
    std::wstring propType = GetW(L"PropertyType");

    // Satýr 1: TÜR
    DrawLabel(ctx, x, y, 100, rowH, L"TÜRÜ:", true);
    // X Koordinatlarýný biraz daha açtýk (Çakýþmayý önlemek için)
    DrawCheckbox(ctx, x + 120, y, L"KONUT", propType.find(L"KONUT") != std::wstring::npos);
    DrawCheckbox(ctx, x + 380, y, L"TÝCARÝ", propType.find(L"TÝCAR") != std::wstring::npos);
    DrawCheckbox(ctx, x + 640, y, L"ARSA/ARAZÝ", propType.find(L"ARSA") != std::wstring::npos);
    y += rowH + 10;

    // Satýr 2: ÝSKAN & MÜLKÝYET
    DrawLabel(ctx, x, y, 150, rowH, L"ÝSKAN:", true);
    DrawCheckbox(ctx, x + 150, y, L"VAR", IsTrue(L"IsIskanVar", L"1"));
    DrawCheckbox(ctx, x + 300, y, L"YOK", IsTrue(L"IsIskanVar", L"0"));

    DrawLabel(ctx, x + 480, y, 150, rowH, L"MÜLKÝYET:", true);
    DrawCheckbox(ctx, x + 630, y, L"HÝSSELÝ", IsTrue(L"IsShared", L"1"));
    DrawCheckbox(ctx, x + 800, y, L"MÜSTAKÝL", IsTrue(L"IsDetached", L"1"));
    y += rowH + 30; // Boþluðu artýrdýk

    // --- FÝYAT KUTUSU (DÜZELTÝLDÝ) ---
    // Yüksekliði 70 -> 90 yaptýk (Büyük fontlar için)
    int priceH = 90;
    ctx.FillRect(x, y, w, priceH, Theme::Color_BoxBgAlt);
    ctx.SetPen(Theme::Color_BrandRed, 2.0f); // Kalýn çerçeve
    ctx.DrawRect(x, y, w, priceH, true);

    // Etiketi Sola Yasla (Geniþlik: 500px verdik ki sýðsýn)
    ctx.SetFont(MakeFont(Theme::Font_H2, true));
    ctx.SetBrush(Theme::Color_TextMain);
    ctx.DrawTextW(L"SATIÞ HÝZMET BEDELÝ (LÝSTE FÝYATI):", x + 20, y, 550, priceH, DT_LEFT | DT_VCENTER);

    // Deðeri Saða Yasla (Geri kalan alan)
    ctx.SetFont(MakeFont(Theme::Font_ValBig, true));
    ctx.SetBrush(Theme::Color_TextAccent);
    std::wstring p = GetW(L"Price") + L" " + GetW(L"Currency", L"TL");

    // x + 580'den baþlayýp sonuna kadar
    ctx.DrawTextW(p, x + 580, y, w - 600, priceH, DT_LEFT | DT_VCENTER);

    y += priceH + 10;
}
// =========================================================
// SAYFA 2
// =========================================================
void ContractPageLayout::RenderPage2(IDrawContext& ctx)
{
    int y = Theme::Margin + 40;
    RenderBuildingInfoPage2(ctx, y); y += 30;
    RenderTransportInfoPage2(ctx, y); y += 30;
    RenderInteriorFeaturesPage2(ctx, y); y += 30;
    RenderMainPropertyFeaturesPage2(ctx, y); y += 30;
    RenderFooterPage1(ctx, y); // Ýmza
}

void ContractPageLayout::RenderBuildingInfoPage2(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"YAPIYA AÝT BÝLGÝLER");
    y += Theme::H_Section;
    int colW = w / 4; int h = Theme::H_RowStd;
    DrawFieldBox(ctx, x, y, colW, h, L"ALAN (NET m2)", GetW(L"NetArea"));
    DrawFieldBox(ctx, x + colW, y, colW, h, L"ALAN (BRÜT m2)", GetW(L"GrossArea"));
    DrawFieldBox(ctx, x + colW * 2, y, colW, h, L"BÝNA YAÞI", GetW(L"BuildingAge"));
    DrawFieldBox(ctx, x + colW * 3, y, colW, h, L"KULLANIM", GetW(L"UsageStatus"));
    y += h;
}

void ContractPageLayout::RenderTransportInfoPage2(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"ULAÞIM OLANAKLARI");
    y += Theme::H_Section + 10;

    // Checkbox hizalamasý
    int rowH = 40;
    DrawCheckbox(ctx, x + 20, y, L"TOPLU TAÞIMA", IsTrue(L"HasPublicTransport"));
    DrawCheckbox(ctx, x + 300, y, L"RAYLI SÝSTEM", IsTrue(L"HasRailSystem"));
    y += rowH + 10;

    DrawFieldBox(ctx, x, y, w, 60, L"ULAÞIM NOTLARI", GetW(L"TransportNotes"));
    y += 60;
}

void ContractPageLayout::RenderInteriorFeaturesPage2(IDrawContext& ctx, int& y) {
    int x = Theme::Margin + 15; int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"ÝÇ ÖZELLÝKLER"); y += Theme::H_Section;
    DrawFieldBox(ctx, x, y, w, 100, L"", GetW(L"InteriorFeatures")); y += 100;
}

void ContractPageLayout::RenderMainPropertyFeaturesPage2(IDrawContext& ctx, int& y) {
    int x = Theme::Margin + 15; int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"DIÞ ÖZELLÝKLER / ISINMA"); y += Theme::H_Section;
    DrawFieldBox(ctx, x, y, w, 100, L"", GetW(L"MainPropertyFeatures")); y += 100;
}

// =========================================================
// SAYFA 3
// =========================================================
void ContractPageLayout::RenderPage3(IDrawContext& ctx)
{
    int y = Theme::Margin + 40;
    RenderTextClausesPage3(ctx, y);
    RenderFooterPage1(ctx, y);
}

void ContractPageLayout::RenderTextClausesPage3(IDrawContext& ctx, int& y)
{
    int x = Theme::Margin + 15;
    int w = Theme::Content_Width - 30;
    DrawSectionHeader(ctx, x, y, w, Theme::H_Section, L"SÖZLEÞME ÞARTLARI");
    y += Theme::H_Section + 20;
    ctx.SetFont(MakeFont(Theme::Font_Val, false));
    ctx.SetBrush(Theme::Color_TextMain);
    std::wstring clauses = GetW(L"ContractClauses");
    if (clauses.empty()) clauses = L"1. Madde...\n2. Madde...";
    ctx.DrawTextW(clauses, x, y, w, 600, DT_LEFT | DT_TOP | DT_WORDBREAK);
}

// -------------------------------------------------------------------------
// ÝMZA ALANI (GÜNCELLENDÝ: METÝN KESÝLMESÝ GÝDERÝLDÝ)
// -------------------------------------------------------------------------
void ContractPageLayout::RenderFooterPage1(IDrawContext& ctx, int&)
{
    int h = 120; // Yüksekliði artýrdýk (100 -> 120) - Sýkýþmayý önler
    int w = (Theme::Content_Width - 120) / 3;
    int y = Theme::Page_Height - Theme::Margin - h - 30;
    int x = Theme::Margin + 60;

    auto DrawSig = [&](int bx, int by, const wchar_t* txt) {
        ctx.FillRect(bx, by, w, h, Theme::Color_BoxBg);
        ctx.SetPen(Theme::Color_BorderLight, 1.0f);
        ctx.DrawRect(bx, by, w, h, true);

        // Baþlýk (Üstten biraz boþluk býrak)
        ctx.SetFont(MakeFont(Theme::Font_Label, true));
        ctx.SetBrush(Theme::Color_TextLabel);
        ctx.DrawTextW(txt, bx, by + 10, w, 25, DT_CENTER | DT_TOP);

        // Ýmza/Kaþe (En alta yapýþtýr)
        ctx.SetFont(MakeFont(10.0f, false)); // Biraz küçülttük (9-10 ideal)
        ctx.DrawTextW(L"(Ýmza / Kaþe)", bx, by + h - 35, w, 30, DT_CENTER | DT_BOTTOM);
        };

    DrawSig(x, y, L"EMLAK ÝÞLETMESÝ");
    DrawSig(x + w + 20, y, L"SÖZLEÞMELÝ ÝÞLETME");
    DrawSig(x + 2 * w + 40, y, L"ÝÞ SAHÝBÝ");
}
// =========================================================
// HELPER FONKSÝYONLAR
// =========================================================

void ContractPageLayout::DrawSectionHeader(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& title)
{
    ctx.FillRect(x, y, w, h, Theme::Color_HeaderBg);
    ctx.SetFont(MakeFont(Theme::Font_H2, true));
    ctx.SetBrush(Theme::Color_TextWhite);
    ctx.DrawTextW(title, x + 10, y, w - 20, h, DT_LEFT | DT_VCENTER);
}

void ContractPageLayout::DrawFieldBox(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& label, const std::wstring& value)
{
    ctx.FillRect(x, y, w, h, Theme::Color_BoxBg);
    ctx.SetPen(Theme::Color_BorderLight, 1.0f);
    ctx.DrawRect(x, y, w, h, true);

    // --- DÝNAMÝK YÜKSEKLÝK ---
    int labelH = (int)(Theme::Font_Label * 1.8f);
    if (labelH < 25) labelH = 25;

    if (!label.empty()) {
        ctx.SetFont(MakeFont(Theme::Font_Label, true));
        ctx.SetBrush(Theme::Color_TextLabel);
        ctx.DrawTextW(label, x + 8, y + 4, w - 16, labelH, DT_LEFT | DT_TOP);
    }

    int valY = label.empty() ? y : (y + labelH);
    int valH = label.empty() ? h : (h - labelH - 4);

    if (!value.empty()) {
        ctx.SetFont(MakeFont(Theme::Font_Val, false));
        ctx.SetBrush(Theme::Color_TextMain);
        ctx.DrawTextW(value, x + 8, valY, w - 16, valH, DT_LEFT | DT_VCENTER | DT_WORDBREAK);
    }
}

// -------------------------------------------------------------------------
// CHECKBOX ÇÝZÝMÝ (GÜNCELLENDÝ: DÝKEY ORTALAMA)
// -------------------------------------------------------------------------
void ContractPageLayout::DrawCheckbox(IDrawContext& ctx, int x, int y, const std::wstring& label, bool isChecked)
{
    // DÝKEY ORTALAMA HESABI
    // Checkbox'ýn bulunduðu satýrýn yüksekliði Theme::H_RowStd (85px)
    // Bu yüzden kutuyu satýrýn ortasýna indiriyoruz.
    // Ancak fonksiyon sadece 'y' (satýr baþý) alýyor. 
    // Basit çözüm: y deðerine manuel bir ofset eklemek veya kutuyu ortalamak.

    int rowHeight = Theme::H_RowStd;

    // Fonta göre kutu boyutu
    int sz = (int)(Theme::Font_Val * 1.6f);
    if (sz < 20) sz = 20;

    // Ortalamak için Y ofseti
    int offsetY = (rowHeight - sz) / 2;
    int boxY = y + offsetY; // Kutunun yeni Y konumu

    // 1. Kutu
    ctx.FillRect(x, boxY, sz, sz, Theme::Color_BoxBg);
    ctx.SetPen(Theme::Color_BorderDark, 1.0f);
    ctx.DrawRect(x, boxY, sz, sz, true);

    // 2. Ýþaret
    if (isChecked) {
        ctx.SetFont(MakeFont(Theme::Font_Val, true));
        ctx.SetBrush(Theme::Color_TextMain);
        ctx.DrawTextW(L"X", x, boxY, sz, sz, DT_CENTER | DT_VCENTER);
    }

    // 3. Etiket
    if (!label.empty()) {
        ctx.SetFont(MakeFont(Theme::Font_Val, false));
        ctx.SetBrush(Theme::Color_TextMain);
        // Metni de kutuyla ayný hizaya getiriyoruz
        ctx.DrawTextW(label, x + sz + 10, boxY, 300, sz, DT_LEFT | DT_VCENTER);
    }
}

void ContractPageLayout::DrawLabel(IDrawContext& ctx, int x, int y, int w, int h, const std::wstring& text, bool bold)
{
    ctx.SetFont(MakeFont(Theme::Font_Label, bold));
    ctx.SetBrush(Theme::Color_TextLabel);
    ctx.DrawTextW(text, x, y, w, h, DT_LEFT | DT_VCENTER);
}

// Veri Eriþim
std::wstring ContractPageLayout::GetW(const wchar_t* key, const wchar_t* def) const {
    auto it = m_dataMap.find(key);
    return (it != m_dataMap.end() && !it->second.empty()) ? it->second : (def ? def : L"");
}
bool ContractPageLayout::IsTrue(const wchar_t* key, const wchar_t* expected) const { return GetW(key) == expected; }
std::wstring ContractPageLayout::ToW(const CString& s) { return std::wstring(s.GetString()); }