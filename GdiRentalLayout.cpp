#include "stdafx.h"
#include "GdiRentalLayout.h"
#include <ctime>

// Yardımcı: CString -> std::wstring
static std::wstring ToStdW(const CString& str) {
#ifdef UNICODE
    return std::wstring(str.GetString());
#else
    int len = MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, NULL, 0);
    if (len <= 0) return L"";
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, buf.data(), len);
    return std::wstring(buf.data());
#endif
}

GdiRentalLayout::GdiRentalLayout() {}
GdiRentalLayout::~GdiRentalLayout() {}

void GdiRentalLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_data.clear();
    for (const auto& kv : fields) {
        m_data[ToStdW(kv.first)] = ToStdW(kv.second);
    }
}

std::wstring GdiRentalLayout::GetVal(const std::wstring& key)
{
    auto it = m_data.find(key);
    return (it != m_data.end()) ? it->second : L"....................";
}

FontDesc GdiRentalLayout::MakeFont(const std::wstring& family, float size, bool bold) {
    return FontDesc(family, size, bold, false);
}

void GdiRentalLayout::Render(IDrawContext& ctx, int pageNo)
{
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255));

    int x = 100;
    int y = 100;
    int w = 1900;
    int lineH = 50; // Satır yüksekliği

    // BAŞLIK
    ctx.SetFont(MakeFont(L"Arial", 16, true));
    ctx.SetBrush(RGB(0, 0, 0));
    ctx.DrawTextW(L"KIRA SOZLESMESI (OZEL SARTLAR)", x, y, w, 50, DT_CENTER);
    y += 100;

    auto DrawLineItem = [&](const std::wstring& label, const std::wstring& val, bool fullWidth = true) {
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(label, x, y, 300, 30, DT_LEFT);

        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150)); // Mavi
        
        int valW = fullWidth ? 1500 : 600;
        ctx.DrawTextW(val, x + 320, y, valW, 30, DT_LEFT);

        if (fullWidth) {
            ctx.SetPen(RGB(0, 0, 0), 1.0f);
            ctx.DrawLine(x, y + 40, x + w, y + 40);
            y += lineH;
        }
    };

    // TABLO ÇİZİMİ
    DrawLineItem(L"DAIRESI", GetVal(L"Address"));
    
    // Yan Yana
    DrawLineItem(L"MAHALLESI", GetVal(L"Neighborhood"), false);
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.SetBrush(RGB(0, 0, 0));
    ctx.DrawTextW(L"SOKAGI", x + 950, y, 200, 30, DT_LEFT);
    ctx.SetFont(MakeFont(L"Arial", 10, false));
    ctx.SetBrush(RGB(0, 0, 150));
    ctx.DrawTextW(GetVal(L"Street"), x + 1160, y, 700, 30, DT_LEFT);
    ctx.SetPen(RGB(0, 0, 0), 1.0f);
    ctx.DrawLine(x, y + 40, x + w, y + 40);
    y += lineH;

    // Kiralayan
    DrawLineItem(L"KIRAYA VEREN", GetVal(L"OwnerName"), false);
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.SetBrush(RGB(0, 0, 0));
    ctx.DrawTextW(L"TC KIMLIK", x + 950, y, 200, 30, DT_LEFT);
    ctx.SetFont(MakeFont(L"Arial", 10, false));
    ctx.SetBrush(RGB(0, 0, 150));
    ctx.DrawTextW(GetVal(L"OwnerTC"), x + 1160, y, 700, 30, DT_LEFT);
    ctx.SetPen(RGB(0, 0, 0), 1.0f);
    ctx.DrawLine(x, y + 40, x + w, y + 40);
    y += lineH;

    DrawLineItem(L"ADRESI", GetVal(L"OwnerAddress"));

    // Kiracı (Boş)
    DrawLineItem(L"KIRACI", L"", false);
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"TC KIMLIK", x + 950, y, 200, 30, DT_LEFT);
    ctx.DrawLine(x, y + 40, x + w, y + 40);
    y += lineH;

    DrawLineItem(L"1 YILLIK KIRA", GetVal(L"AnnualRent"));
    DrawLineItem(L"1 AYLIK KIRA", GetVal(L"Price"));
    
    DrawLineItem(L"KIRA MUDDETI", L"1 (BIR) YIL");
    DrawLineItem(L"SIMDIKI DURUMU", L"BOS, TEMIZ");
    DrawLineItem(L"DEMIRBASLAR", L"Kombi, Petekler, Mutfak Dolabi");

}