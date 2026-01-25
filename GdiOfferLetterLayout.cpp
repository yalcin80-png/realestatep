#include "stdafx.h"
#include "GdiOfferLetterLayout.h"

static std::wstring ToStdW(const CString& str) {
#ifdef UNICODE
    return std::wstring(str.GetString());
#else
    // ANSI Fallback
    int len = MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, NULL, 0);
    if (len <= 0) return L"";
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, buf.data(), len);
    return std::wstring(buf.data());
#endif
}

void GdiOfferLetterLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_data.clear();
    for (const auto& kv : fields) {
        m_data[ToStdW(kv.first)] = ToStdW(kv.second);
    }
}

std::wstring GdiOfferLetterLayout::GetVal(const std::wstring& key)
{
    auto it = m_data.find(key);
    return (it != m_data.end()) ? it->second : L"....................";
}

FontDesc GdiOfferLetterLayout::MakeFont(const std::wstring& family, float size, bool bold) {
    return FontDesc(family, size, bold, false);
}

void GdiOfferLetterLayout::Render(IDrawContext& ctx, int pageNo)
{
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255));
    
    // Çerçeve
    ctx.SetPen(RGB(0, 0, 0), 2.0f);
    ctx.DrawRect(50, 50, 2000, 2870, true);

    int y = 100;
    int x = 100;
    int w = 1900;

    // BAŞLIK
    ctx.SetFont(MakeFont(L"Arial", 24, true));
    ctx.SetBrush(RGB(200, 0, 0));
    ctx.DrawTextW(L"GAYRİMENKUL ALIM / SATIM TEKLİF FORMU", x, y, w, 60, DT_CENTER);
    y += 80;

    ctx.SetFont(MakeFont(L"Arial", 14, true));
    ctx.SetBrush(RGB(0, 0, 0));
    ctx.DrawTextW(L"REALTY WORLD GAZİŞEHİR", x, y, w, 40, DT_CENTER);
    y += 60;

    ctx.SetPen(RGB(150, 150, 150), 1.0f);
    ctx.DrawLine(50, y, 2050, y);
    y += 50;

    auto DrawHeader = [&](const std::wstring& text) {
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(0, 0, 150));
        ctx.DrawTextW(text, x, y, w, 30, DT_LEFT);
        y += 40;
    };

    auto DrawField = [&](const std::wstring& label, const std::wstring& val) {
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(100, 100, 100));
        ctx.DrawTextW(label, x, y, 300, 30, DT_LEFT);

        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(val, x + 310, y, 1000, 30, DT_LEFT);
        
        ctx.SetPen(RGB(220, 220, 220), 1.0f);
        ctx.DrawLine(x, y + 35, x + w, y + 35);
        y += 50;
    };

    // 1. Gayrimenkul
    DrawHeader(L"1. GAYRİMENKUL BİLGİLERİ");
    DrawField(L"Adres:", GetVal(L"FullAddress"));
    
    // Yan Yana
    std::wstring line1 = GetVal(L"City") + L" / " + GetVal(L"Neighborhood");
    DrawField(L"İl / Mahalle:", line1);
    
    std::wstring line2 = L"Ada: " + GetVal(L"Ada") + L" Parsel: " + GetVal(L"Parsel");
    DrawField(L"Ada / Parsel:", line2);
    
    DrawField(L"Liste Fiyatı:", GetVal(L"Price"));
    
    y += 30;

    // 2. Alıcı
    DrawHeader(L"2. TEKLİF SAHİBİ (ALICI)");
    DrawField(L"Adı Soyadı:", GetVal(L"BuyerName"));
    DrawField(L"TC / Tel:", GetVal(L"BuyerTC") + L" / " + GetVal(L"BuyerPhone"));

    y += 30;

    // 3. Teklif
    DrawHeader(L"3. TEKLİF DETAYLARI");
    DrawField(L"Teklif Fiyatı:", GetVal(L"OfferPrice"));
    DrawField(L"Ödeme Şekli:", GetVal(L"PaymentMethod"));
    DrawField(L"Geçerlilik:", GetVal(L"Validity"));

    y += 50;
    
    // Metin
    ctx.SetFont(MakeFont(L"Arial", 10, false));
    ctx.SetBrush(RGB(0, 0, 0));
    std::wstring legal = L"Yukarıdaki teklifimin satıcı tarafından kabul edilmesi durumunda, belirtilen şartlarda satınalma işlemini gerçekleştireceğimi beyan ederim.";
    ctx.DrawTextW(legal, x, y, w, 100, DT_LEFT | DT_WORDBREAK);
    y += 150;

    // İmzalar
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"TEKLİF VEREN (ALICI)", 200, y, 500, 30, DT_CENTER);
    ctx.DrawRect(200, y+40, 500, 100, true);

    ctx.DrawTextW(L"TEKLİFİ ALAN (DANIŞMAN)", 1300, y, 500, 30, DT_CENTER);
    ctx.DrawRect(1300, y+40, 500, 100, true);
}