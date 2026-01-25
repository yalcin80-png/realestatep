#include "stdafx.h"
#include "GdiRentalAuthLayout.h"

// (Yardımcı dönüşüm fonksiyonu ToStdW, önceki dosyalarda vardı, aynısı)
static std::wstring ToStdW(const CString& str) {
#ifdef UNICODE
    return std::wstring(str.GetString());
#else
    // Basit ANSI dönüşümü
    int len = MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, NULL, 0);
    if(len<=0) return L"";
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, buf.data(), len);
    return std::wstring(buf.data());
#endif
}

GdiRentalAuthLayout::GdiRentalAuthLayout() {}
GdiRentalAuthLayout::~GdiRentalAuthLayout() {}

void GdiRentalAuthLayout::SetData(const std::vector<std::pair<CString, CString>>& fields) {
    m_data.clear();
    for(const auto& kv : fields) m_data[ToStdW(kv.first)] = ToStdW(kv.second);
}

std::wstring GdiRentalAuthLayout::GetVal(const std::wstring& key) {
    auto it = m_data.find(key);
    return (it != m_data.end()) ? it->second : L"....................";
}

FontDesc GdiRentalAuthLayout::MakeFont(const std::wstring& family, float size, bool bold) {
    return FontDesc(family, size, bold, false);
}

void GdiRentalAuthLayout::Render(IDrawContext& ctx, int pageNo) {
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255));
    ctx.SetPen(RGB(0,0,0), 2.0f);
    ctx.DrawRect(50, 50, 2000, 2870, true); // Çerçeve

    int x = 100, y = 100, w = 1900;

    if (pageNo == 1) {
        // --- SAYFA 1: BİLGİLER ---
        ctx.SetFont(MakeFont(L"Arial", 22, true));
        ctx.SetBrush(RGB(0,0,0));
        ctx.DrawTextW(L"KİRALIK YETKİLENDİRME SÖZLEŞMESİ", x, y, w, 60, DT_CENTER);
        y += 100;

        auto DrawSection = [&](const std::wstring& title) {
            ctx.SetFont(MakeFont(L"Arial", 14, true));
            ctx.SetBrush(RGB(200,0,0));
            ctx.DrawTextW(title, x, y, w, 40, DT_LEFT);
            y += 50;
        };

        auto DrawField = [&](const std::wstring& lbl, const std::wstring& val) {
            ctx.SetFont(MakeFont(L"Arial", 10, true));
            ctx.SetBrush(RGB(100,100,100));
            ctx.DrawTextW(lbl, x, y, 300, 30, DT_LEFT);
            
            ctx.SetFont(MakeFont(L"Arial", 11, true));
            ctx.SetBrush(RGB(0,0,150));
            ctx.DrawTextW(val, x+310, y, 1500, 30, DT_LEFT);
            
            ctx.SetPen(RGB(200,200,200), 1.0f);
            ctx.DrawLine(x, y+35, x+w, y+35);
            y += 50;
        };

        DrawSection(L"1. MÜLK BİLGİLERİ");
        DrawField(L"Adres:", GetVal(L"FullAddress"));
        DrawField(L"İl / İlçe:", GetVal(L"City") + L" / " + GetVal(L"District"));
        DrawField(L"Talep Edilen Kira:", GetVal(L"Price"));

        y += 50;
        DrawSection(L"2. MÜLK SAHİBİ BİLGİLERİ");
        DrawField(L"Adı Soyadı:", GetVal(L"OwnerName"));
        DrawField(L"Telefon:", GetVal(L"OwnerPhone"));
        DrawField(L"TC Kimlik:", GetVal(L"OwnerTC"));
    }
    else if (pageNo == 2) {
        // --- SAYFA 2: MADDELER ---
        ctx.SetFont(MakeFont(L"Arial", 14, true));
        ctx.DrawTextW(L"SÖZLEŞME MADDELERİ", x, y, w, 40, DT_CENTER);
        y += 60;
        
        ctx.SetFont(MakeFont(L"Times New Roman", 11));
        std::wstring text = L"MADDE 1: Taraflar...\nMADDE 2: Yetki...\n(Buraya uzun metinler gelecek)";
        ctx.DrawTextW(text, x, y, w, 2000, DT_LEFT | DT_WORDBREAK);
    }
    else {
        // --- SAYFA 3: İMZALAR ---
        ctx.SetFont(MakeFont(L"Arial", 14, true));
        ctx.DrawTextW(L"ÖZEL ŞARTLAR", x, y, w, 40, DT_LEFT);
        y += 50;
        ctx.SetPen(RGB(0,0,0), 1.0f);
        ctx.DrawRect(x, y, w, 1000, true);
        
        y += 1100;
        
        // İmzalar
        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.DrawTextW(L"MÜLK SAHİBİ", 200, y, 500, 30, DT_CENTER);
        ctx.DrawTextW(GetVal(L"OwnerName"), 200, y+40, 500, 30, DT_CENTER);
        ctx.DrawRect(200, y+80, 500, 150, true);

        ctx.DrawTextW(L"YETKİLİ DANIŞMAN", 1300, y, 500, 30, DT_CENTER);
        ctx.DrawRect(1300, y+80, 500, 150, true);
    }
}