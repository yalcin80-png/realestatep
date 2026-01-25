#include "stdafx.h"
#include "ListingPageLayout.h"
#include "ThemeConfig.h" 
#include "JsonFormRenderer.h" // Motorumuz
#include <map>
#include <set>

// ============================================================================
// YARDIMCILAR
// ============================================================================

// Statik yaparak diğer dosyalardaki GetExePath ile çakışmasını önlüyoruz.
static CString GetExePathLocal() {
    TCHAR buffer[MAX_PATH];
    ::GetModuleFileName(NULL, buffer, MAX_PATH);
    CString path(buffer);
    int pos = path.ReverseFind('\\');
    if (pos != -1) path = path.Left(pos);
    return path;
}

std::wstring ListingPageLayout::ToW(const CString& s)
{
    if (s.IsEmpty()) return std::wstring();
#ifdef UNICODE
    return std::wstring(s.GetString());
#else
    int len = ::MultiByteToWideChar(1254, 0, s, -1, nullptr, 0);
    if (len <= 0) return std::wstring();
    std::wstring ws(static_cast<size_t>(len), L'\0');
    ::MultiByteToWideChar(1254, 0, s, -1, &ws[0], len);
    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
#endif
}

// ============================================================================
// KURUCU & SETDATA
// ============================================================================
ListingPageLayout::ListingPageLayout()
{
}

void ListingPageLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_customerFields.clear();
    m_propertyFields.clear();
    m_notes.clear();

    // Tekrar eden verileri engellemek için set kullanıyoruz
    std::set<CString> addedKeys;

    for (const auto& p : fields)
    {
        CString key = p.first;
        CString val = p.second;

        // 1. Temizlik
        if (val.IsEmpty() || val == _T("0")) continue;

        // 2. Tekrar Kontrolü
        if (addedKeys.find(key) != addedKeys.end()) continue;
        addedKeys.insert(key);

        // 3. Başlık ve Sistem Verileri
        if (key == _T("TableName")) { m_tableName = val; continue; }
        if (key == _T("DocTitle")) { m_title = val; continue; }

        // Kodları gizle (İsteğe bağlı)
        if (key == _T("Cari_Kod") || key == _T("Home_Code")) continue;

        // 4. Akıllı Ayrıştırma (Müşteri vs Mülk)
        CString keyLower = key;
        keyLower.MakeLower();

        bool isCustomer = false;
        // Müşteri kelimeleri
        if (keyLower.Find(_T("owner")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("sahibi")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("müşteri")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("telefon")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("e-posta")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("email")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("tc kimlik")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("firma")) != -1) isCustomer = true;
        else if (keyLower.Find(_T("yetkili")) != -1) isCustomer = true;

        if (isCustomer)
        {
            m_customerFields.emplace_back(ToW(key), ToW(val));
        }
        else
        {
            // Mülk Listesi (Gereksiz sistem flaglerini temizle)
            if (key != _T("IsIskanVar") && key != _T("Swap") && key != _T("Deleted"))
            {
                m_propertyFields.emplace_back(ToW(key), ToW(val));
            }
        }
    }
}

// ============================================================================
// RENDER (JSON MOTORU ENTEGRASYONU)
// ============================================================================
void ListingPageLayout::Render(IDrawContext& ctx, int pageNo)
{
    // 1. Verileri JSON Motoruna Uygun Hale Getir (Map'e Çevir)
    std::map<CString, CString> renderData;

    // Başlık Bilgileri
    renderData[_T("Title")] = m_title;
    renderData[_T("TableName")] = m_tableName;

    // Müşteri Verilerini Aktar
    for (const auto& item : m_customerFields) {
        renderData[item.first.c_str()] = item.second.c_str();
    }

    // Mülk Verilerini Aktar
    for (const auto& item : m_propertyFields) {
        renderData[item.first.c_str()] = item.second.c_str();
    }

    // 2. JSON Motorunu Başlat
    JsonFormRenderer engine;

    // Şablon Dosyasının Yolu
    // Exe'nin yanındaki "templates" klasörüne bakar.
    //CString exePath = GetExePathLocal();
    //CString templatePath = exePath + _T("\\templates\\emlak_bilgi_formu.json");
    CString templatePath = _T("templates\\emlak_bilgi_formu.json"); // Sadece bu!

   // bool success = engine.Render(ctx, templateName, renderData);
    // 3. ÇİZİMİ BAŞLAT
    bool success = engine.Render(ctx, templatePath, renderData);

    // Hata Durumu (Dosya yoksa kullanıcıyı uyar)
    if (!success) {
        // Fallback: Basit bir hata mesajı çiz
        ctx.SetLogicalPageSize(2100, 2970);
        ctx.SetFont(FontDesc(L"Arial", 14, true));
        ctx.SetBrush(RGB(255, 0, 0)); // Kırmızı Yazı

        CString errorMsg;
        // Düzeltme: AllocSysString yerine GetString kullandık (Leak Önleme)
        errorMsg.Format(_T("HATA: Şablon dosyası bulunamadı veya bozuk!\nYol: %s"), templatePath.GetString());

        ctx.DrawTextW(errorMsg.GetString(), 100, 100, 1000, 200, DT_LEFT);
    }
}