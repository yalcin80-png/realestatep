#include "stdafx.h"
#include "GdiWaiverLayout.h"
#include <ctime>
#include <vector> // std::vector için

// ---------------------------------------------------------
// YARDIMCI: CString -> std::wstring Dönüştürücü
// (Bunu sınıfın içine değil, sadece bu dosyaya özel yapıyoruz)
// ---------------------------------------------------------
static std::wstring ToStdW(const CString& str)
{
    // Win32++ CString, GetString() ile const TCHAR* döner.
    // Unicode projesinde TCHAR = wchar_t olduğundan doğrudan cast edilebilir.
#ifdef UNICODE
    return std::wstring(str.GetString());
#else
    // ANSI projesi ise (pek sanmam ama önlem)
    int len = MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, NULL, 0);
    if (len <= 0) return L"";
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_ACP, 0, str.GetString(), -1, buf.data(), len);
    return std::wstring(buf.data());
#endif
}

// ---------------------------------------------------------
// SINIF IMPLEMENTASYONU
// ---------------------------------------------------------

GdiWaiverLayout::GdiWaiverLayout()
{
}

GdiWaiverLayout::~GdiWaiverLayout()
{
}

void GdiWaiverLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_data.clear();

    // Gelen CString verilerini std::wstring'e çevirip sakla
    for (const auto& kv : fields) {
        m_data[ToStdW(kv.first)] = ToStdW(kv.second);
    }

    // Tarih yoksa bugünü ekle (Standart C++ yöntemiyle)
    if (m_data.find(L"Date") == m_data.end()) {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now); // Güvenli zaman fonksiyonu

        wchar_t buffer[32];
        swprintf_s(buffer, 32, L"%02d.%02d.%04d", ltm.tm_mday, 1 + ltm.tm_mon, 1900 + ltm.tm_year);

        m_data[L"Date"] = buffer;
    }
}

std::wstring GdiWaiverLayout::GetVal(const std::wstring& key)
{
    auto it = m_data.find(key);
    if (it != m_data.end()) {
        return it->second;
    }
    // Veri yoksa boşluk döndür ki çizim bozulmasın
    return L"................";
}

FontDesc GdiWaiverLayout::MakeFont(const std::wstring& family, float size, bool bold, bool italic)
{
    return FontDesc(family, size, bold, italic);
}

void GdiWaiverLayout::Render(IDrawContext& ctx, int pageNo)
{
    // Sayfa Ayarları (A4)
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255));

    // Koordinatlar
    int y = 150;
    int leftMargin = 150;
    int width = 1800; // İçerik genişliği

    // --- BAŞLIK ---
    ctx.SetFont(MakeFont(L"Arial", 20, true));
    ctx.SetBrush(RGB(0, 0, 0));
    // DrawTextW artık std::wstring alıyor (IDrawContext yapına göre .c_str() gerekebilir, ama çoğu overload destekler)
    ctx.DrawTextW(L"ÖNALIM HAKKINDAN FERAGATNAME", leftMargin, y, width, 50, DT_CENTER);

    // Alt Çizgi
    ctx.SetPen(RGB(0, 0, 0), 2.0f);
    ctx.DrawLine(leftMargin, y + 60, leftMargin + width, y + 60);
    y += 150;

    // --- GİRİŞ METNİ ---
    ctx.SetFont(MakeFont(L"Times New Roman", 12));
    std::wstring introText = L"Aşağıda bilgileri verilen taşınmazın diğer paydaşı/paydaşları tarafından üçüncü şahsa satılması halinde, Türk Medeni Kanunu'nun 732. ve devamı maddelerinden kaynaklanan yasal önalım (şüf'a) hakkımı kullanmayacağımı, bu hakkımdan gayrikabili rücu olarak feragat ettiğimi beyan ve kabul ederim.";

    ctx.DrawTextW(introText, leftMargin, y, width, 200, DT_LEFT | DT_WORDBREAK);
    y += 150;

    // --- TAŞINMAZ BİLGİLERİ BAŞLIK ---
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"TAŞINMAZ BİLGİLERİ:", leftMargin, y, width, 30, DT_LEFT);

    ctx.SetPen(RGB(150, 150, 150), 1.0f);
    ctx.DrawLine(leftMargin, y + 35, leftMargin + width, y + 35);
    y += 60;

    // --- BİLGİ SATIRLARI (Lambda ile Pratik Çizim) ---
    auto DrawRow = [&](const std::wstring& label, const std::wstring& val) {
        // Etiket
        ctx.SetFont(MakeFont(L"Arial", 10, true));
        ctx.DrawTextW(label, leftMargin, y, 300, 30, DT_LEFT);

        // Değer
        ctx.SetFont(MakeFont(L"Arial", 11, false));
        ctx.DrawTextW(val, leftMargin + 310, y, width - 310, 30, DT_LEFT);

        y += 50;
        };

    // Verileri Birleştir (std::wstring + operatörü ile)
    std::wstring cityDistrict = GetVal(L"City") + L" / " + GetVal(L"District");
    std::wstring adaParsel = GetVal(L"AdaNo") + L" / " + GetVal(L"ParselNo");

    DrawRow(L"İl / İlçe:", cityDistrict);
    DrawRow(L"Mahalle:", GetVal(L"Neighborhood"));
    DrawRow(L"Ada / Parsel:", adaParsel);

    y += 30;

    // --- SONUÇ METNİ ---
    ctx.SetPen(RGB(150, 150, 150), 1.0f);
    ctx.DrawLine(leftMargin, y, leftMargin + width, y);
    y += 50;

    ctx.SetFont(MakeFont(L"Times New Roman", 12));

    // Alıcı ismini metnin içine gömüyoruz
    std::wstring finalText = L"İşbu feragatname, taşınmazın " + GetVal(L"BuyerName") + L" isimli alıcıya satışı işlemi için geçerlidir.";

    ctx.DrawTextW(finalText, leftMargin, y, width, 100, DT_LEFT | DT_WORDBREAK);
    y += 150;

    // --- İMZA BÖLÜMÜ ---
    int signX = 1300; // Sağ tarafa yasla

    // Tarih
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"TARİH:", signX, y, 200, 30, DT_RIGHT);

    ctx.SetFont(MakeFont(L"Arial", 11));
    ctx.DrawTextW(GetVal(L"Date"), signX + 210, y, 300, 30, DT_LEFT);
    y += 100;

    // Feragat Eden
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"FERAGAT EDEN (MÜLK SAHİBİ)", signX, y, 500, 30, DT_CENTER);
    y += 40;

    // İsim
    ctx.SetFont(MakeFont(L"Arial", 11));
    ctx.DrawTextW(GetVal(L"OwnerName"), signX, y, 500, 30, DT_CENTER);
    y += 100;

    // İmza Yeri
    ctx.SetFont(MakeFont(L"Arial", 10, true));
    ctx.DrawTextW(L"İMZA", signX, y, 500, 30, DT_CENTER);
}