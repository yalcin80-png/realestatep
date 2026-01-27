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

    // Ortak çizim yardımcısı
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

    if (pageNo == 1) {
        // ============================================================
        // SAYFA 1: GENEL BİLGİLER VE TARAF BİLGİLERİ
        // ============================================================
        
        // BAŞLIK
        ctx.SetFont(MakeFont(L"Arial", 18, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"KİRA SÖZLEŞMESİ", x, y, w, 50, DT_CENTER);
        y += 80;

        // Alt başlık
        ctx.SetFont(MakeFont(L"Arial", 12, false));
        ctx.DrawTextW(L"(Özel Şartlar)", x, y, w, 30, DT_CENTER);
        y += 60;

        // Sözleşme numarası ve tarihi
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.DrawTextW(L"Sözleşme No:", x, y, 300, 30, DT_LEFT);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"ContractNo"), x + 320, y, 400, 30, DT_LEFT);
        
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.DrawTextW(L"Tarih:", x + 950, y, 200, 30, DT_LEFT);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"ContractDate"), x + 1160, y, 700, 30, DT_LEFT);
        y += 60;

        // TAŞINMAZİN TANIMI
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"TAŞINMAZIN TANIMI", x, y, w, 40, DT_LEFT);
        y += 50;

        // TABLO ÇİZİMİ
        DrawLineItem(L"DAİRESİ / APT NO", GetVal(L"Address"));
        
        // Yan Yana (Mahalle ve Sokak)
        DrawLineItem(L"MAHALLE", GetVal(L"Neighborhood"), false);
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"SOKAK", x + 950, y, 200, 30, DT_LEFT);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150));
        ctx.DrawTextW(GetVal(L"Street"), x + 1160, y, 700, 30, DT_LEFT);
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawLine(x, y + 40, x + w, y + 40);
        y += lineH;

        // İl/İlçe
        DrawLineItem(L"İL / İLÇE", GetVal(L"City") + L" / " + GetVal(L"District"));
        DrawLineItem(L"BRÜT M²", GetVal(L"SquareMeter"));
        DrawLineItem(L"ODA SAYISI", GetVal(L"Rooms"));
        DrawLineItem(L"KAT NO", GetVal(L"FloorNo"));
        
        y += 20;

        // KİRALAYAN (MÜLK SAHİBİ)
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"KİRAYA VEREN (MÜLK SAHİBİ)", x, y, w, 40, DT_LEFT);
        y += 50;

        DrawLineItem(L"ADI SOYADI", GetVal(L"OwnerName"), false);
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"TC KİMLİK NO", x + 950, y, 200, 30, DT_LEFT);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.SetBrush(RGB(0, 0, 150));
        ctx.DrawTextW(GetVal(L"OwnerTC"), x + 1160, y, 700, 30, DT_LEFT);
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawLine(x, y + 40, x + w, y + 40);
        y += lineH;

        DrawLineItem(L"ADRES", GetVal(L"OwnerAddress"));
        DrawLineItem(L"TELEFON", GetVal(L"OwnerPhone"));

        y += 20;

        // KİRACI BİLGİLERİ (Boş - doldurulacak)
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"KİRACI BİLGİLERİ", x, y, w, 40, DT_LEFT);
        y += 50;

        DrawLineItem(L"ADI SOYADI", L"", false);
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"TC KİMLİK NO", x + 950, y, 200, 30, DT_LEFT);
        ctx.DrawLine(x, y + 40, x + w, y + 40);
        y += lineH;

        DrawLineItem(L"ADRES", L"");
        DrawLineItem(L"TELEFON", L"");

    }
    else if (pageNo == 2) {
        // ============================================================
        // SAYFA 2: MALİ ŞARTLAR VE SÖZLEŞME MADDELERİ
        // ============================================================
        
        // BAŞLIK
        ctx.SetFont(MakeFont(L"Arial", 14, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"KİRA SÖZLEŞMESİ (Sayfa 2)", x, y, w, 40, DT_CENTER);
        y += 70;

        // MALİ ŞARTLAR
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"MALİ ŞARTLAR", x, y, w, 40, DT_LEFT);
        y += 50;

        DrawLineItem(L"AYLIK KİRA BEDELİ", GetVal(L"Price") + L" " + GetVal(L"Currency"));
        DrawLineItem(L"YILLIK KİRA BEDELİ", GetVal(L"AnnualRent") + L" " + GetVal(L"Currency"));
        DrawLineItem(L"DEPOZITO / TEMINAT", GetVal(L"Deposit") + L" " + GetVal(L"Currency"));
        DrawLineItem(L"KİRA ÖDEME GÜNÜ", GetVal(L"PaymentDay"));
        DrawLineItem(L"KİRA BAŞLANGIÇ TARİHİ", GetVal(L"StartDate"));
        DrawLineItem(L"KİRA BİTİŞ TARİHİ", GetVal(L"EndDate"));
        DrawLineItem(L"SÜRE", L"1 (BİR) YIL");

        y += 30;

        // SÖZLEŞME MADDELERİ
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"SÖZLEŞME MADDELERİ", x, y, w, 40, DT_LEFT);
        y += 50;

        ctx.SetFont(MakeFont(L"Times New Roman", 10, false));
        ctx.SetBrush(RGB(0, 0, 0));

        std::wstring articles = 
            L"MADDE 1 - TARAFLAR: İşbu sözleşme yukarıda kimlik bilgileri belirtilen kiralayan ve kiracı arasında düzenlenmiştir.\n\n"
            L"MADDE 2 - KONU: Kiralayan, yukarıda tanımı yapılan taşınmazı kiracıya kiraya vermeyi ve kiracı da kiralamayı kabul ve taahhüt etmiştir.\n\n"
            L"MADDE 3 - SÜRE: Kira süresi yukarıda belirtilen tarihler arasındadır. Süre sonunda otomatik uzama söz konusu değildir.\n\n"
            L"MADDE 4 - KİRA BEDELİ: Aylık kira bedeli yukarıda belirtilmiştir. Her ayın belirlenen gününde peşin olarak ödenecektir.\n\n"
            L"MADDE 5 - DEPOZITO: Kiracı, yukarıda belirtilen depozito tutarını kiralayana teslim etmiştir.\n\n"
            L"MADDE 6 - KULLANIM: Taşınmaz sadece konut amaçlı kullanılacaktır. İzinsiz tadilat yapılamaz.\n\n"
            L"MADDE 7 - GİDERLER: Elektrik, su, doğalgaz, aidat gibi giderler kiracıya aittir.\n\n"
            L"MADDE 8 - TESLİM ŞARTI: Taşınmaz temiz, kullanılabilir durumda teslim edilmiştir. Demirbaşlar: Kombi, Petek, Mutfak Dolabı.";

        ctx.DrawTextW(articles, x, y, w, 1600, DT_LEFT | DT_WORDBREAK);
    }
    else {
        // ============================================================
        // SAYFA 3: ÖZEL ŞARTLAR VE İMZALAR
        // ============================================================
        
        // BAŞLIK
        ctx.SetFont(MakeFont(L"Arial", 14, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"KİRA SÖZLEŞMESİ (Sayfa 3)", x, y, w, 40, DT_CENTER);
        y += 70;

        // ÖZEL ŞARTLAR
        ctx.SetFont(MakeFont(L"Arial", 12, true));
        ctx.SetBrush(RGB(150, 0, 0));
        ctx.DrawTextW(L"ÖZEL ŞARTLAR VE EK HÜKÜMLER", x, y, w, 40, DT_LEFT);
        y += 50;

        // Özel şartlar için boş alan (elle doldurulacak)
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawRect(x, y, w, 800, true);
        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.SetBrush(RGB(100, 100, 100));
        ctx.DrawTextW(L"(Taraflar arasında mutabık kalınan özel şartlar buraya yazılacaktır)", 
                     x + 10, y + 10, w - 20, 30, DT_LEFT);
        y += 850;

        // GENEL HÜKÜMLER
        ctx.SetFont(MakeFont(L"Times New Roman", 9, false));
        ctx.SetBrush(RGB(0, 0, 0));
        std::wstring general = 
            L"İşbu sözleşme 3 (üç) sayfa ve toplam 8 (sekiz) maddeden oluşmaktadır. "
            L"Sözleşmede hüküm bulunmayan hallerde 6098 sayılı Türk Borçlar Kanunu "
            L"ve ilgili diğer mevzuat hükümleri uygulanır. İşbu sözleşme taraflarca "
            L"okunmuş, anlaşılmış ve karşılıklı olarak imzalanmıştır.";
        ctx.DrawTextW(general, x, y, w, 150, DT_LEFT | DT_WORDBREAK);
        y += 180;

        // İMZA ALANI
        y += 100;
        
        // Sol taraf: Kiralayan (Mülk Sahibi)
        int leftX = 200;
        int rightX = 1300;
        int signW = 500;
        
        ctx.SetFont(MakeFont(L"Arial", 11, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"KİRAYA VEREN (MÜLK SAHİBİ)", leftX, y, signW, 30, DT_CENTER);
        ctx.DrawTextW(L"KİRACI", rightX, y, signW, 30, DT_CENTER);
        y += 40;

        // İsimler
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"OwnerName"), leftX, y, signW, 30, DT_CENTER);
        ctx.DrawTextW(L"................................", rightX, y, signW, 30, DT_CENTER);
        y += 80;

        // İmza çizgileri
        ctx.SetPen(RGB(0, 0, 0), 1.0f);
        ctx.DrawLine(leftX, y, leftX + signW, y);
        ctx.DrawLine(rightX, y, rightX + signW, y);
        y += 10;

        ctx.SetFont(MakeFont(L"Arial", 9, false));
        ctx.SetBrush(RGB(100, 100, 100));
        ctx.DrawTextW(L"(İmza)", leftX, y, signW, 30, DT_CENTER);
        ctx.DrawTextW(L"(İmza)", rightX, y, signW, 30, DT_CENTER);
        y += 100;

        // Kutular (elle imza için)
        ctx.SetPen(RGB(0, 0, 0), 1.5f);
        ctx.DrawRect(leftX, y, signW, 150, true);
        ctx.DrawRect(rightX, y, signW, 150, true);
        
        // Tarih
        y += 180;
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(L"Tarih: ", x, y, 100, 30, DT_LEFT);
        ctx.SetFont(MakeFont(L"Arial", 10, false));
        ctx.DrawTextW(GetVal(L"ContractDate"), x + 110, y, 400, 30, DT_LEFT);
    }
}