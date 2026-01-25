#include "stdafx.h"
#include "GdiLocationFormLayout.h" // Yeni başlık dosyası

void GdiLocationFormLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_data.clear();
    for (const auto& kv : fields) {
        m_data[kv.first] = kv.second;
    }
}

void GdiLocationFormLayout::Render(IDrawContext& ctx, int pageNo)
{
    // A4 Boyutu
    ctx.SetLogicalPageSize(2100, 2970);
    ctx.FillRect(0, 0, 2100, 2970, RGB(255, 255, 255)); // Beyaz Zemin

    // Çerçeve
    ctx.SetPen(RGB(0, 0, 0), 2.0f);
    ctx.DrawRect(50, 50, 2000, 2870, true);

    int y = 100;
    int x = 100;
    int w = 1900;

    // --- BAŞLIK ---
    ctx.SetFont(MakeFont(24, true));
    ctx.SetBrush(RGB(0, 0, 0));
    ctx.DrawTextW(L"YER GÖSTERME BELGESİ", 50, y, 2000, 60, DT_CENTER);
    y += 70;

    ctx.SetFont(MakeFont(16, true));
    ctx.SetBrush(RGB(200, 0, 0)); // Kırmızı
    ctx.DrawTextW(L"REALTY WORLD GAZİŞEHİR", 50, y, 2000, 40, DT_CENTER);
    y += 60;

    // Çizgi
    ctx.SetPen(RGB(150, 150, 150), 1.0f);
    ctx.DrawLine(50, y, 2050, y);
    y += 50;

    // --- MÜŞTERİ BÖLÜMÜ ---
    ctx.SetFont(MakeFont(14, true));
    ctx.SetBrush(RGB(0, 0, 150)); // Mavi
    ctx.DrawTextW(L"MÜŞTERİ BİLGİLERİ", x, y, w, 40, DT_LEFT);
    y += 50;

    auto DrawField = [&](const CString& label, const CString& valKey) {
        ctx.SetFont(MakeFont(10, false));
        ctx.SetBrush(RGB(100, 100, 100));
        ctx.DrawTextW(label.GetString(), x, y, 200, 30, DT_LEFT);

        ctx.SetFont(MakeFont(11, true));
        ctx.SetBrush(RGB(0, 0, 0));
        ctx.DrawTextW(GetVal(valKey).GetString(), x + 200, y, w - 200, 30, DT_LEFT);
        
        ctx.SetPen(RGB(220, 220, 220), 1.0f);
        ctx.DrawLine(x, y + 35, x + w, y + 35);
        y += 50;
    };

    DrawField(_T("Adı Soyadı:"), _T("OwnerName"));
    DrawField(_T("TC Kimlik:"),  _T("OwnerTC"));
    DrawField(_T("Telefon:"),    _T("OwnerPhone"));
    DrawField(_T("Adres:"),      _T("OwnerAddress"));

    y += 30;

    // --- MÜLK BÖLÜMÜ ---
    ctx.SetFont(MakeFont(14, true));
    ctx.SetBrush(RGB(0, 0, 150));
    ctx.DrawTextW(L"GÖRÜLEN GAYRİMENKUL", x, y, w, 40, DT_LEFT);
    y += 50;

    DrawField(_T("Adres:"), _T("FullAddress"));
    
    // Yan Yana (İl / Fiyat)
    ctx.SetFont(MakeFont(10, false));
    ctx.SetBrush(RGB(100, 100, 100));
    ctx.DrawTextW(L"İl / İlçe:", x, y, 200, 30, DT_LEFT);
    
    ctx.SetFont(MakeFont(11, true));
    ctx.SetBrush(RGB(0, 0, 0));
    CString cityInfo = GetVal(_T("City")) + L" / " + GetVal(_T("District"));
    ctx.DrawTextW(cityInfo.GetString(), x + 200, y, 700, 30, DT_LEFT);

    // Fiyat (Sağ Taraf)
    ctx.SetFont(MakeFont(10, false));
    ctx.SetBrush(RGB(100, 100, 100));
    ctx.DrawTextW(L"Fiyat:", x + 1000, y, 200, 30, DT_LEFT);

    ctx.SetFont(MakeFont(11, true));
    ctx.SetBrush(RGB(200, 0, 0));
    CString priceInfo = GetVal(_T("Price")) + L" " + GetVal(_T("Currency"));
    ctx.DrawTextW(priceInfo.GetString(), x + 1200, y, 700, 30, DT_LEFT);
    
    y += 50;

    // --- YASAL METİN ---
    y += 30;
    CString legalText = L"Yukarıda özellikleri belirtilen gayrimenkulü, firmanız yetkilileri vasıtası ile gördüm. "
                        L"Bu gayrimenkulü şahsım, eşim, kan veya sıhri hısımlarım, ortağım veya temsil ettiğim kurum adına "
                        L"satın aldığım veya kiraladığım takdirde, T.S. 11816 standartları gereği KDV hariç satış bedelinin "
                        L"%2'sini veya bir kira bedelini hizmet bedeli olarak ödemeyi kabul ve taahhüt ediyorum.";

    ctx.SetFont(MakeFont(9, false, true));
    ctx.SetBrush(RGB(50, 50, 50));
    ctx.DrawTextW(legalText.GetString(), x, y, w, 200, DT_LEFT | DT_WORDBREAK);
    
    y += 200;

    // --- İMZALAR ---
    int sigY = y + 50;
    
    ctx.SetFont(MakeFont(10, true));
    ctx.SetBrush(RGB(0, 0, 0));

    // Müşteri İmza
    ctx.DrawTextW(L"MÜŞTERİ", 200, sigY, 500, 30, DT_CENTER);
    ctx.SetPen(RGB(0, 0, 0), 1.0f);
    ctx.DrawRect(200, sigY + 40, 500, 150, true);

    // Danışman İmza
    ctx.DrawTextW(L"YETKİLİ DANIŞMAN", 1300, sigY, 500, 30, DT_CENTER);
    ctx.DrawRect(1300, sigY + 40, 500, 150, true);

    // Alt Bilgi
    ctx.SetFont(MakeFont(8, false));
    ctx.SetBrush(RGB(150, 150, 150));
    ctx.DrawTextW(L"© Realty World Gazişehir - Sistem Çıktısı", 50, 2900, 2000, 50, DT_CENTER);
}