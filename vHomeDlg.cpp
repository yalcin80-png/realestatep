#include "stdafx.h"
#include "vHomeDlg.h"
#include "dataIsMe.h"
#include "resource.h"
#include <vector>
#include <map>
#include <sstream>

#include <commctrl.h>
#include "HomeFeaturesPage.h"

namespace {
    constexpr int kHomeTabId = 5001;
    constexpr int kTabHeaderReservePx = 24; // mevcut UI'yi aþaðý itmek için
    constexpr int kBottomSectionTop = 375;  // Resource.rc'deki çizgi üst sýnýrý
}

// Küçük Yardýmcýlar
namespace {
    // Güvenli metin okuma
    CString GetTextSafe(HWND hDlg, int id) {
        HWND hCtrl = ::GetDlgItem(hDlg, id);
        if (!hCtrl) return _T("");
        int len = ::GetWindowTextLength(hCtrl);
        if (len <= 0) return _T("");
        std::vector<TCHAR> buffer(len + 1);
        ::GetWindowText(hCtrl, buffer.data(), len + 1);
        return CString(buffer.data());
    }
    // Güvenli metin yazma
    void SetTextSafe(HWND hDlg, int id, const CString& text) {
        ::SetDlgItemText(hDlg, id, text);
    }
}

CHomeDialog::CHomeDialog(DatabaseManager& dbManagerRef, DialogMode mode, const CString& cariKod, const CString& homeCodeToEdit)
    : CDialog(IDD_HOME_DIALOG), m_dbManager(dbManagerRef), m_dialogMode(mode), m_cariKod(cariKod), m_homeCodeToEdit(homeCodeToEdit) {
}

BOOL CHomeDialog::OnInitDialog() {
    CDialog::OnInitDialog();

    // Seviye-2: tab kontrolünü kur (Genel + Özellikler + Çevre)
    InitTabs();

    OnSetCtrl();
    ::SendMessage(GetDlgItem(IDOK), BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE);
    ::SendMessage(GetDlgItem(ID_EDIT_HOME_CODE), EM_SETREADONLY, TRUE, 0);
    ::SendMessage(GetDlgItem(IDC_EDIT_CARI_KOD), EM_SETREADONLY, TRUE, 0);

    if (m_dialogMode == INEWUSER) {
        SetWindowText(_T("Yeni Daire / Ev Ekle"));
        SetTextSafe(*this, ID_EDIT_HOME_CODE, m_dbManager.GenerateNextHomeCode());
        if (!m_cariKod.IsEmpty()) SetTextSafe(*this, IDC_EDIT_CARI_KOD, m_cariKod);
        SetTextSafe(*this, IDC_EDIT_UPDATED_AT, m_dbManager.GetCurrentIsoUtc());
        ::SendMessage(GetDlgItem(IDC_COMBO_CURRENCY), CB_SELECTSTRING, -1, (LPARAM)_T("TL"));
        ::SendMessage(GetDlgItem(IDC_COMBO_STATUS), CB_SELECTSTRING, -1, (LPARAM)_T("Aktif"));
    }
    else {
        SetWindowText(_T("Daire / Ev Düzenle"));
        if (!m_homeCodeToEdit.IsEmpty()) {
            auto dataMap = m_dbManager.FetchRecordMap(TABLE_NAME_HOME, _T("Home_Code"), m_homeCodeToEdit);
            if (!dataMap.empty()) {
                m_dbManager.Bind_Data_To_UI(this, TABLE_NAME_HOME, dataMap);
                if (dataMap.find(_T("Cari_Kod")) != dataMap.end()) m_cariKod = dataMap[_T("Cari_Kod")];

                // Seviye-2: özellik tablarýný DB verisiyle doldur
                m_featuresPage1.LoadFromMap(dataMap);
                m_featuresPage2.LoadFromMap(dataMap);
            }
            else {
                MessageBox(_T("Kayýt bulunamadý!"), _T("Hata"), MB_ICONERROR);
            }
        }
    }
    LayoutTabAndPages();
    FixTabFonts();
    // Yeni kayýtta default olarak boþ özellikler
    if (m_dialogMode == INEWUSER)
    {
        std::map<CString, CString> tmp;
        m_featuresPage1.LoadFromMap(tmp);
        m_featuresPage2.LoadFromMap(tmp);
    }

    SwitchTab(0);
    return TRUE;
}

void CHomeDialog::OnOK() {
    std::map<CString, CString> uiData;

    // 1. Ana Dialog (Tab-0: Genel) verilerini oku
    m_dbManager.Bind_UI_To_Data(this, TABLE_NAME_HOME, uiData);

    // 2. Özellik Sekmeleri (Tab-1 ve Tab-2) için AYRI map'ler kullan
    std::map<CString, CString> mapPage1, mapPage2;

    // Sayfalarýn Handle'ý varsa (Window oluþmuþsa) verileri çek
    if (m_featuresPage1.GetHwnd() && ::IsWindow(m_featuresPage1.GetHwnd())) {
        m_featuresPage1.SaveToMap(mapPage1);
    }
    if (m_featuresPage2.GetHwnd() && ::IsWindow(m_featuresPage2.GetHwnd())) {
        m_featuresPage2.SaveToMap(mapPage2);
    }

    // 3. Map verilerini ana uiData map'ine güvenli aktar (Çakýþma Önleme)
    // Page 1: Cephe ve Ýç Özellikler
    for (auto const& [key, val] : mapPage1) {
        if (!val.IsEmpty()) uiData[key] = val;
    }
    // Page 2: Dýþ, Muhit ve Diðer
    for (auto const& [key, val] : mapPage2) {
        if (!val.IsEmpty()) uiData[key] = val;
    }

    // 4. Veri temizleme ve Kimlik tamamlama
    SanitizeDataMap(uiData);

    if (m_dialogMode == IUPDATEUSER)
        uiData[_T("Home_Code")] = m_homeCodeToEdit;
    else if (uiData[_T("Home_Code")].IsEmpty())
        uiData[_T("Home_Code")] = m_dbManager.GenerateNextHomeCode();

    if (uiData[_T("Cari_Kod")].IsEmpty())
        uiData[_T("Cari_Kod")] = m_cariKod;

    uiData[_T("Updated_At")] = m_dbManager.GetCurrentIsoUtc();

    // 5. Struct'a dönüþtür ve Kaydet
    Home_cstr h;
    for (const auto& [key, val] : uiData) {
        DatabaseManager::SetFieldByStringName(h, key, val);
    }

    bool success = (m_dialogMode == INEWUSER) ? m_dbManager.InsertGlobal(h) : m_dbManager.UpdateGlobal(h);

    if (success) {
        MessageBox(_T("Kayýt Baþarýlý!"), _T("Bilgi"), MB_ICONINFORMATION);
        CDialog::OnOK();
    }
    else {
        MessageBox(_T("Veritabaný kayýt hatasý!"), _T("Hata"), MB_ICONERROR);
    }
}

// ============================================================================
// Seviye-2: Tab Control + Dinamik Özellik Sayfalarý
// ============================================================================

void CHomeDialog::InitTabs()
{
    if (m_hTab) return;

    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icc);

    RECT rcClient{};
    ::GetClientRect(*this, &rcClient);

    // Tab, tüm üst alaný kapsasýn; alt butonlar sabit kalsýn.
    RECT rcTab{ 5, 5, rcClient.right - 5, kBottomSectionTop - 5 };

    m_hTab = ::CreateWindowEx(0, WC_TABCONTROL, _T(""),
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
        *this, (HMENU)(INT_PTR)kHomeTabId, GetApp()->GetInstanceHandle(), nullptr);

    if (!m_hTab) return;

    // Sekmeler
    TCITEM tci{};
    tci.mask = TCIF_TEXT;

    tci.pszText = (LPTSTR)_T("Genel");
    TabCtrl_InsertItem(m_hTab, 0, &tci);

    tci.pszText = (LPTSTR)_T("Özellikler");
    TabCtrl_InsertItem(m_hTab, 1, &tci);

    tci.pszText = (LPTSTR)_T("Çevre/Detay");
    TabCtrl_InsertItem(m_hTab, 2, &tci);

    // Genel UI'nin tab baþlýðýný kapatmamasý için bir defalýk aþaðý itme
    ShiftGeneralControlsForTabHeader();
    CollectGeneralControls();

    // Tab body alaný
    LayoutTabAndPages();
}

void CHomeDialog::CollectGeneralControls()
{
    m_generalControls.clear();
    for (HWND h = ::GetWindow(*this, GW_CHILD); h; h = ::GetWindow(h, GW_HWNDNEXT))
    {
        if (h == m_hTab) continue;

        int id = (int)::GetDlgCtrlID(h);
        if (id == IDOK || id == IDCANCEL || id == IDC_BTN_LOAD_CLIPBOARD) continue;

        RECT rc{};
        ::GetWindowRect(h, &rc);
        ::MapWindowPoints(nullptr, *this, (LPPOINT)&rc, 2);

        // Üst ana alan: sekmelerle yönetilecek
        if (rc.top < kBottomSectionTop)
            m_generalControls.push_back(h);
    }
}

void CHomeDialog::ShiftGeneralControlsForTabHeader()
{
    if (m_layoutShifted) return;
    m_layoutShifted = true;

    // Tab baþlýðý için üstte ~24px yer aç
    for (HWND h = ::GetWindow(*this, GW_CHILD); h; h = ::GetWindow(h, GW_HWNDNEXT))
    {
        if (h == m_hTab) continue;
        int id = (int)::GetDlgCtrlID(h);
        if (id == IDOK || id == IDCANCEL) continue;

        RECT rc{};
        ::GetWindowRect(h, &rc);
        ::MapWindowPoints(nullptr, *this, (LPPOINT)&rc, 2);

        // Görünmez/0x0 kontrolleri elleme
        if ((rc.right - rc.left) <= 0 || (rc.bottom - rc.top) <= 0) continue;

        if (rc.top >= kBottomSectionTop) continue; // alt bar sabit

        ::SetWindowPos(h, nullptr, rc.left, rc.top + kTabHeaderReservePx,
            rc.right - rc.left, rc.bottom - rc.top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CHomeDialog::LayoutTabAndPages()
{
    if (!m_hTab) return;

    RECT rcClient{};
    ::GetClientRect(*this, &rcClient);

    RECT rcTab{ 5, 5, rcClient.right - 5, kBottomSectionTop - 5 };
    ::SetWindowPos(m_hTab, nullptr,
        rcTab.left, rcTab.top,
        rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    // ? DOÐRU: Tab'ýn kendi client rect’i üzerinden body hesapla
    RECT rcBody{};
    ::GetClientRect(m_hTab, &rcBody);
    TabCtrl_AdjustRect(m_hTab, FALSE, &rcBody); // artýk doðru koordinat sisteminde

    // ? DOÐRU: Sayfalarýn parent'ý TAB olmalý
    if (!m_featuresPage1.GetHwnd())
        m_featuresPage1.Create(m_hTab, rcBody, 6001, CHomeFeaturesPage::PageKind::Features1);

    if (!m_featuresPage2.GetHwnd())
        m_featuresPage2.Create(m_hTab, rcBody, 6002, CHomeFeaturesPage::PageKind::Features2);

    // ? Sayfalarý tab içinde konumlandýr
    ::SetWindowPos(m_featuresPage1, nullptr,
        rcBody.left, rcBody.top,
        rcBody.right - rcBody.left, rcBody.bottom - rcBody.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    ::SetWindowPos(m_featuresPage2, nullptr,
        rcBody.left, rcBody.top,
        rcBody.right - rcBody.left, rcBody.bottom - rcBody.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    // Ýstersen burada mevcut seçili taba göre görünürlük de güncellenebilir:
    // int sel = (m_hTab ? TabCtrl_GetCurSel(m_hTab) : 0);
    // if (sel < 0) sel = 0;
    // SwitchTab(sel);
}

void CHomeDialog::SwitchTab(int index)
{
    // Tab-0: mevcut UI
    bool showGeneral = (index == 0);
    for (HWND h : m_generalControls)
        ::ShowWindow(h, showGeneral ? SW_SHOW : SW_HIDE);

    // Tab-1/2: özellik sayfalarý
    ::ShowWindow(m_featuresPage1, (index == 1) ? SW_SHOW : SW_HIDE);
    ::ShowWindow(m_featuresPage2, (index == 2) ? SW_SHOW : SW_HIDE);

    if (m_hTab)
        TabCtrl_SetCurSel(m_hTab, index);
}

void CHomeDialog::OnLoadFromClipboard() {
    CString rawText = GetClipboardText();
    if (rawText.IsEmpty()) {
        MessageBox(_T("Panoda metin yok!"), _T("Uyarý"), MB_ICONWARNING);
        return;
    }

    std::map<CString, CString> parsedData = ParseSahibindenText(rawText);
    if (parsedData.empty()) {
        MessageBox(_T("Metinden veri çýkarýlamadý."), _T("Hata"), MB_ICONERROR);
        return;
    }

    std::map<CString, CString> schemaData = NormalizeToSchemaMap(parsedData);
    SanitizeDataMap(schemaData);

    std::map<CString, CString> currentData;
    m_dbManager.Bind_UI_To_Data(this, TABLE_NAME_HOME, currentData);
    for (const auto& [key, val] : schemaData) if (!val.IsEmpty()) currentData[key] = val;
    m_dbManager.Bind_Data_To_UI(this, TABLE_NAME_HOME, currentData);

    MessageBox(_T("Veriler aktarýldý."), _T("Tamam"), MB_ICONINFORMATION);
}

// ============================================================================
// PARSE FONKSÝYONU (Düzeltilmiþ ve Güçlendirilmiþ)
// ============================================================================
std::map<CString, CString> CHomeDialog::ParseSahibindenText(const CString& rawText)
{
    std::map<CString, CString> out;
    CString text = rawText;

    // Satýr sonlarýný normalize et
    text.Replace(_T("\r\n"), _T("\n"));
    text.Replace(_T("\r"), _T("\n"));
    text.Trim();

    std::wistringstream iss((LPCWSTR)text);
    std::vector<CString> lines;
    for (std::wstring line; std::getline(iss, line); ) {
        CString cLine(line.c_str());
        cLine.Trim();
        if (!cLine.IsEmpty()) lines.push_back(cLine);
    }

    // Alias haritasý (Eþanlamlýlar)
    static const std::map<CString, CString> alias = {
        {_T("Isýtma"), _T("Isýtma Tipi")},
        {_T("Eþyalý"), _T("Eþyalý mý")},
        {_T("Site Ýçerisinde"), _T("Site Ýçinde")},
        {_T("Aidat (TL)"), _T("Aidat")},
        // Emlak Tipi ve Ýlan Tarihi'ni aþaðýda özel olarak ele alýyoruz
    };

    for (size_t i = 0; i < lines.size(); ++i)
    {
        CString cLine = lines[i];

        // 1?? ADRES (Þehir / Ýlçe / Mahalle)
        if (cLine.Find(_T('/')) != -1 && cLine.Find(_T("Ýlan No")) == -1 && cLine.Find(_T("TL")) == -1)
        {
            CString temp = cLine;
            temp.Replace(_T(" / "), _T("/"));
            int p1 = temp.Find(_T('/'));
            int p2 = temp.Find(_T('/'), p1 + 1);
            if (p1 != -1) {
                out[_T("Þehir")] = temp.Left(p1);
                if (p2 != -1) {
                    out[_T("Ýlçe")] = temp.Mid(p1 + 1, p2 - p1 - 1);
                    out[_T("Mahalle")] = temp.Mid(p2 + 1);
                }
                else {
                    out[_T("Ýlçe")] = temp.Mid(p1 + 1);
                }
                CString addr = temp; addr.Replace(_T("/"), _T(", "));
                out[_T("Adres")] = addr;
            }
            continue;
        }

        // 2?? FÝYAT
        if (cLine.Find(_T("TL")) != -1 || cLine.Find(_T("USD")) != -1 || cLine.Find(_T("EUR")) != -1)
        {
            if (out.find(_T("Fiyat")) == out.end()) { // Sadece ilk fiyatý al
                CString digits;
                for (int j = 0; j < cLine.GetLength(); ++j) if (_istdigit(cLine[j])) digits += cLine[j];

                // Formatlama: 3500000 -> 3.500.000
                if (!digits.IsEmpty()) {
                    CString formatted = digits;
                    int len = formatted.GetLength();
                    if (len > 3) for (int k = len - 3; k > 0; k -= 3) formatted.Insert(k, _T("."));
                    out[_T("Fiyat")] = formatted;
                }

                if (cLine.Find(_T("USD")) != -1) out[_T("Para Birimi")] = _T("USD");
                else if (cLine.Find(_T("EUR")) != -1) out[_T("Para Birimi")] = _T("EUR");
                else out[_T("Para Birimi")] = _T("TL");
            }
            continue;
        }

        // 3?? ÝLAN NO (Özel Blok)
        if (cLine.Left(7).CompareNoCase(_T("Ýlan No")) == 0)
        {
            CString val = cLine.Mid(7); val.Trim();
            if (val == _T(":") || val.IsEmpty()) { // Deðer alt satýrda
                if (i + 1 < lines.size()) {
                    out[_T("Ýlan No")] = lines[i + 1];
                    i++;
                }
            }
            else { // Deðer ayný satýrda (Ýlan No: 12345)
                if (val.Left(1) == _T(":")) val = val.Mid(1);
                out[_T("Ýlan No")] = val;
            }
            continue;
        }

        // 4?? ÝLAN TARÝHÝ (Özel Blok - Düzeltildi)
        if (cLine.Left(11).CompareNoCase(_T("Ýlan Tarihi")) == 0)
            // 4?? ÝLAN TARÝHÝ (Özel Blok - Süper Esnek Versiyon)
        {
            CString lower = cLine;
            lower.MakeLower();

            // satýrda hem "ilan" hem de "tarihi" geçiyorsa, bunu ilan tarihi satýrý say
            if (lower.Find(_T("ilan")) != -1 && lower.Find(_T("tarihi")) != -1)
            {
                CString val;

                // Eðer ayný satýrda ":" sonrasý bir þey varsa onu al
                int colonPos = cLine.Find(_T(':'));
                if (colonPos != -1)
                {
                    val = cLine.Mid(colonPos + 1);
                    val.Trim();
                }

                // Deðer hâlâ boþsa alt satýrdan al (sahibinden genelde böyle yapýyor)
                if (val.IsEmpty() && i + 1 < (int)lines.size())
                {
                    val = lines[i + 1];
                    val.Trim();
                    i++;    // alt satýrý da tükettik
                }

                if (!val.IsEmpty())
                {
                    out[_T("Ýlan Tarihi")] = val;
                }

                continue;
            }
        }


        // 5?? EMLAK TÝPÝ (Özel Blok - Düzeltildi)
        if (cLine.Left(10).CompareNoCase(_T("Emlak Tipi")) == 0)
        {
            CString val = cLine.Mid(10); val.Trim();
            if (val == _T(":") || val.IsEmpty()) { // Deðer alt satýrda
                if (i + 1 < lines.size()) {
                    out[_T("PropertyType")] = lines[i + 1];
                    i++;
                }
            }
            else { // Deðer ayný satýrda (Emlak Tipi Satýlýk Daire)
                if (val.Left(1) == _T(":")) val = val.Mid(1);
                val.Trim();
                out[_T("PropertyType")] = val;
            }
            continue;
        }

        // 6?? DÝÐER ALANLAR (Genel Tarama)
        CString key = cLine;
        bool handled = false;

        // Alias kontrolü
        if (alias.find(key) != alias.end()) key = alias.at(key);

        // Bilinen anahtarlar listesi (Eðer alias deðilse)
        CString knownKeys[] = {
            _T("Oda Sayýsý"), _T("Bina Yaþý"), _T("Bulunduðu Kat"), _T("Kat Sayýsý"),
            _T("Isýtma"), _T("Banyo Sayýsý"), _T("Mutfak"), _T("Balkon"), _T("Asansör"),
            _T("Otopark"), _T("Eþyalý"), _T("Kullaným Durumu"), _T("Site Ýçerisinde"),
            _T("Site Adý"), _T("Aidat"), _T("Krediye Uygun"), _T("Tapu Durumu"),
            _T("Kimden"), _T("Takas"), _T("m² (Brüt)"), _T("m² (Net)")
        };

        // Satýrýn kendisi bir anahtar mý? (Alt alta durumu)
        for (const auto& k : knownKeys) {
            if (key.CompareNoCase(k) == 0) {
                if (i + 1 < lines.size()) {
                    // Alias uygulayarak kaydet
                    CString finalKey = k;
                    if (alias.find(k) != alias.end()) finalKey = alias.at(k);

                    out[finalKey] = lines[i + 1];
                    i++;
                    handled = true;
                }
                break;
            }
        }
        if (handled) continue;

        // Satýr "Anahtar: Deðer" veya "Anahtar Deðer" formatýnda mý? (Yan yana durumu)
        // Örn: "Oda Sayýsý 3+1"
        for (const auto& k : knownKeys) {
            if (cLine.Left(k.GetLength()).CompareNoCase(k) == 0) {
                CString val = cLine.Mid(k.GetLength());
                val.Trim();
                if (!val.IsEmpty() && val != _T(":")) {
                    if (val.Left(1) == _T(":")) val = val.Mid(1);
                    val.Trim();

                    CString finalKey = k;
                    if (alias.find(k) != alias.end()) finalKey = alias.at(k);

                    out[finalKey] = val;
                    handled = true;
                }
                break;
            }
        }
    }

    // 7?? Fiyat / m² hesapla (Güçlendirilmiþ)
    auto ToNumber = [](CString s) -> double
        {
            CString digits;
            for (int i = 0; i < s.GetLength(); ++i)
            {
                if (_istdigit(s[i]))
                    digits += s[i];
            }

            if (digits.IsEmpty())
                return 0.0;

            return _tstof(digits); // "3095000" -> 3095000.0
        };

    // Hem Fiyat hem de Net m² varsa hesapla
    auto itFiyat = out.find(_T("Fiyat"));
    auto itNet = out.find(_T("m² (Net)"));

    if (itFiyat != out.end() && itNet != out.end())
    {
        double fiyat = ToNumber(itFiyat->second);
        double net = ToNumber(itNet->second);

        if (fiyat > 0.0 && net > 0.0)
        {
            double per = fiyat / net;   // m² fiyatý
            CString perStr;
            perStr.Format(_T("%.0f"), per);   // tam sayý olarak

            // Binlik ayraç ekle: 30950 -> 30.950
            int len = perStr.GetLength();
            for (int k = len - 3; k > 0; k -= 3)
                perStr.Insert(k, _T("."));

            out[_T("Fiyat/m²")] = perStr;
        }
    }

    out[_T("Website")] = _T("sahibinden.com");
    return out;
}

std::map<CString, CString> CHomeDialog::NormalizeToSchemaMap(const std::map<CString, CString>& rawFields)
{
    std::map<CString, CString> schema;
    auto get = [&](const CString& k) { auto it = rawFields.find(k); return (it != rawFields.end()) ? it->second : CString(); };

    schema[_T("ListingNo")] = get(_T("Ýlan No"));
    schema[_T("ListingDate")] = get(_T("Ýlan Tarihi"));
    schema[_T("Price")] = get(_T("Fiyat"));
    schema[_T("Currency")] = get(_T("Para Birimi"));
    schema[_T("PricePerM2")] = get(_T("Fiyat/m²"));

    schema[_T("City")] = get(_T("Þehir"));
    schema[_T("District")] = get(_T("Ýlçe"));
    schema[_T("Neighborhood")] = get(_T("Mahalle"));
    schema[_T("Address")] = get(_T("Adres"));

    // PropertyType doðrudan geliyor
    schema[_T("PropertyType")] = get(_T("PropertyType"));

    schema[_T("RoomCount")] = get(_T("Oda Sayýsý"));
    schema[_T("NetArea")] = get(_T("m² (Net)"));
    schema[_T("GrossArea")] = get(_T("m² (Brüt)"));
    schema[_T("BuildingAge")] = get(_T("Bina Yaþý"));
    schema[_T("Floor")] = get(_T("Bulunduðu Kat"));
    schema[_T("TotalFloor")] = get(_T("Kat Sayýsý"));

    schema[_T("HeatingType")] = get(_T("Isýtma Tipi"));
    schema[_T("BathroomCount")] = get(_T("Banyo Sayýsý"));
    schema[_T("KitchenType")] = get(_T("Mutfak"));

    schema[_T("Balcony")] = get(_T("Balkon"));
    schema[_T("Elevator")] = get(_T("Asansör"));
    schema[_T("Parking")] = get(_T("Otopark"));
    schema[_T("Furnished")] = get(_T("Eþyalý mý"));
    schema[_T("InSite")] = get(_T("Site Ýçinde"));
    schema[_T("SiteName")] = get(_T("Site Adý"));
    schema[_T("Dues")] = get(_T("Aidat"));

    schema[_T("CreditEligible")] = get(_T("Krediye Uygun"));
    schema[_T("DeedStatus")] = get(_T("Tapu Durumu"));
    schema[_T("SellerType")] = get(_T("Kimden"));
    schema[_T("Swap")] = get(_T("Takas"));
    schema[_T("UsageStatus")] = get(_T("Kullaným Durumu"));

    schema[_T("WebsiteName")] = _T("sahibinden.com");
    if (schema[_T("Status")].IsEmpty()) schema[_T("Status")] = _T("Aktif");

    return schema;
}

void CHomeDialog::SanitizeDataMap(std::map<CString, CString>& dataMap)
{
    // Veritabaný sayýsal olduðu için kayýt sýrasýnda boþ metin gitmemeli
    auto CleanNum = [&](const CString& key) {
        if (dataMap.find(key) == dataMap.end()) return;
        CString v = dataMap[key];
        CString n;
        for (int i = 0; i < v.GetLength(); i++) if (_istdigit(v[i])) n += v[i];

        if (n.IsEmpty()) dataMap[key] = _T("0");
        else dataMap[key] = n;
        };

    CleanNum(_T("Price"));
    CleanNum(_T("NetArea"));
    CleanNum(_T("GrossArea"));
    CleanNum(_T("Dues"));
    CleanNum(_T("PricePerM2"));
    CleanNum(_T("PurchasePrice"));
    CleanNum(_T("SalePrice"));
    CleanNum(_T("PhotoCount"));

    auto FixVarYok = [&](const CString& key) {
        if (dataMap.find(key) == dataMap.end()) return;
        CString v = dataMap[key]; v.Trim();
        if (v.CompareNoCase(_T("Evet")) == 0 || v.CompareNoCase(_T("Var")) == 0 || v == _T("1") || v.CompareNoCase(_T("True")) == 0)
            dataMap[key] = _T("Var");
        else if (v.CompareNoCase(_T("Hayýr")) == 0 || v.CompareNoCase(_T("Yok")) == 0 || v == _T("0") || v.CompareNoCase(_T("False")) == 0)
            dataMap[key] = _T("Yok");
        };

    FixVarYok(_T("Balcony"));
    FixVarYok(_T("Elevator"));
    FixVarYok(_T("Furnished"));
    FixVarYok(_T("InSite"));
    FixVarYok(_T("Swap"));
}

void CHomeDialog::OnSetCtrl() {
    std::vector<CString> varyok = { _T("Var"), _T("Yok") };
    std::vector<CString> yesno = { _T("Evet"), _T("Hayýr") };
    std::vector<CString> types = { _T("Satýlýk Daire"), _T("Kiralýk Daire"), _T("Satýlýk Ýþyeri"), _T("Kiralýk Ýþyeri"), _T("Satýlýk Arsa"), _T("Kiralýk Arsa"), _T("Satýlýk Villa"), _T("Kiralýk Villa"), _T("Satýlýk Yazlýk"), _T("Kiralýk Yazlýk"), _T("Satýlýk Bina"), _T("Devren Satýlýk") };
    std::vector<CString> isitma = { _T("Kombi (Doðalgaz)"), _T("Kombi (Elektrik)"), _T("Merkezi"), _T("Merkezi (Pay Ölçer)"), _T("Soba"), _T("Doðalgaz Sobasý"), _T("Kat Kaloriferi"), _T("Yerden Isýtma"), _T("Klima"), _T("Yok") };
    std::vector<CString> mutfak = { _T("Kapalý"), _T("Açýk (Amerikan)"), _T("Ankastre") };
    std::vector<CString> tapu = { _T("Kat Mülkiyetli"), _T("Kat Ýrtifaklý"), _T("Hisseli"), _T("Müstakil"), _T("Arsa"), _T("Bilinmiyor") };
    std::vector<CString> kimden = { _T("Sahibinden"), _T("Emlak Ofisinden"), _T("Bankadan"), _T("Ýnþaat Firmasýndan") };
    std::vector<CString> currency = { _T("TL"), _T("USD"), _T("EUR"), _T("GBP") };
    std::vector<CString> usage = { _T("Boþ"), _T("Kiracýlý"), _T("Mülk Sahibi") };
    std::vector<CString> status = { _T("Aktif"), _T("Opsiyonlu"), _T("Satýldý"), _T("Kiralandý"), _T("Pasif") };
    std::vector<CString> otopark = { _T("Açýk Otopark"), _T("Kapalý"), _T("Açýk & Kapalý Otopark"), _T("Yok") };
    std::vector<CString> website = { _T("sahibinden.com"), _T("hepsiemlak"), _T("emlakjet"), _T("zingat"), _T("Diðer") };

    FillCombo(IDC_COMBO_BALCONY, varyok);
    FillCombo(IDC_COMBO_ELEVATOR, varyok);
    FillCombo(IDC_COMBO_ESYALI, varyok);
    FillCombo(IDC_COMBO_INSITE, varyok);
    FillCombo(IDC_COMBO_SWAP, varyok);
    FillCombo(IDC_COMBO_CREDIT_ELIGIBLE, yesno);
    FillCombo(IDC_COMBO_PROPERTY_TYPE, types);
    FillCombo(ID_COMBO_ISITMA, isitma);
    FillCombo(IDC_COMBO_MUTFAK, mutfak);
    FillCombo(IDC_COMBO_DEED_STATUS, tapu);
    FillCombo(IDC_COMBO_SELLER_TYPE, kimden);
    FillCombo(IDC_COMBO_CURRENCY, currency);
    FillCombo(IDC_COMBO_USAGE_STATUS, usage);
    FillCombo(IDC_COMBO_STATUS, status);
    FillCombo(IDC_COMBO_PARKING, otopark);
    FillCombo(IDC_COMBO_WEBSITE, website);
}

void CHomeDialog::FillCombo(int id, const std::vector<CString>& items) {
    HWND hCtrl = GetDlgItem(id);
    if (!hCtrl) return;
    ::SendMessage(hCtrl, CB_RESETCONTENT, 0, 0);
    for (const auto& s : items) ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s);
}

CString CHomeDialog::GetClipboardText() {
    if (!::OpenClipboard(*this)) return _T("");
    HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
    if (!hData) { ::CloseClipboard(); return _T(""); }
    LPCWSTR ptr = (LPCWSTR)::GlobalLock(hData);
    CString txt(ptr);
    ::GlobalUnlock(hData);
    ::CloseClipboard();
    return txt;
}

INT_PTR CHomeDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_BTN_LOAD_CLIPBOARD) {
        OnLoadFromClipboard();
        return TRUE;
    }

    if (uMsg == WM_NOTIFY)
    {
        NMHDR* hdr = (NMHDR*)lParam;
        if (hdr && hdr->idFrom == kHomeTabId && hdr->code == TCN_SELCHANGE)
        {
            int sel = TabCtrl_GetCurSel(m_hTab);
            if (sel < 0) sel = 0;
            SwitchTab(sel);
            return TRUE;
        }
    }
    switch (uMsg)
    {
    case WM_VSCROLL:
    {
        int nDelta = 0;
        int nMaxPos = m_nVscrollMax;

        switch (LOWORD(wParam))
        {
        case SB_LINEDOWN:      nDelta = 10; break;
        case SB_LINEUP:        nDelta = -10; break;
        case SB_PAGEDOWN:      nDelta = 50; break;
        case SB_PAGEUP:        nDelta = -50; break;
        case SB_THUMBTRACK:    nDelta = (int)HIWORD(wParam) - m_nVscrollPos; break;
        }

        int nNewPos = m_nVscrollPos + nDelta;
        nNewPos = std::max(0, std::min(nNewPos, nMaxPos));

        if (nNewPos != m_nVscrollPos)
        {
            ScrollWindow(0, m_nVscrollPos - nNewPos);
            m_nVscrollPos = nNewPos;
            SetScrollPos(SB_VERT, m_nVscrollPos, TRUE);
        }
        return 0;
    }

    case WM_SIZE:
        UpdateScrollInfo();
        break;
    }
    if (uMsg == WM_SIZE)
    {
        LayoutTabAndPages();
        return FALSE; // default iþlemler devam edebilir
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}

void CHomeDialog::UpdateScrollInfo()
{
    CRect rc;
    rc =  GetClientRect();

    // Ýçerik yüksekliðini hesapla (Örn: Kontrollerin bittiði yer)
    // Eðer kontrolleriniz dinamikse burayý 600-800 gibi bir deðer yapabilirsiniz.
    int nContentHeight = 650;
    int nContentWidth = 550;

    m_nVscrollMax = std::max(0, nContentHeight - rc.Height());
    m_nHscrollMax = std::max(0, nContentWidth - rc.Width());

    SCROLLINFO si{ sizeof(si) };
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;

    // Dikey Scroll
    si.nMax = nContentHeight;
    si.nPage = rc.Height();
    si.nPos = m_nVscrollPos;
    SetScrollInfo(SB_VERT, si, TRUE);

    // Yatay Scroll (Gerekliyse)
    si.nMax = nContentWidth;
    si.nPage = rc.Width();
    si.nPos = m_nHscrollPos;
    SetScrollInfo(SB_HORZ, si, TRUE);
}

void CHomeDialog::ApplyFontRecursive(HWND hWnd, HFONT hFont)
{
    if (!hWnd || !hFont) return;

    ::SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

    for (HWND child = ::GetWindow(hWnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
        ApplyFontRecursive(child, hFont);
}

void CHomeDialog::FixTabFonts()
{
    // 1) Dialog'un aktif fontunu al
    if (!m_hUiFont)
        m_hUiFont = (HFONT)::SendMessage(*this, WM_GETFONT, 0, 0);

    if (!m_hUiFont)
        m_hUiFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

    // 2) Tab kontrolüne uygula
    if (m_hTab)
        ::SendMessage(m_hTab, WM_SETFONT, (WPARAM)m_hUiFont, TRUE);

    // 3) Tab sayfalarýna ve içindeki dinamik kontrollere uygula
    if (m_featuresPage1.GetHwnd())
        ApplyFontRecursive(m_featuresPage1, m_hUiFont);

    if (m_featuresPage2.GetHwnd())
        ApplyFontRecursive(m_featuresPage2, m_hUiFont);
}











// -----------------------------------------------------------------------------
//  IMPORTANT BUILD NOTE (Linker unresolved externals fix)
//  ---------------------------------------------------------------------------
//  Some project setups compile only files already listed in the IDE/build system.
//  If HomeFeaturesPage.cpp / FeaturesCodec.cpp are not added to the build, the
//  linker will report unresolved external symbols for CHomeFeaturesPage methods.
//
//  To make the integration "drop-in" and avoid missing compilation units, we
//  include the implementation .cpp files here by default.
//
//  If you *do* add HomeFeaturesPage.cpp and FeaturesCodec.cpp as separate
//  compilation units, define HOME_FEATURES_BUILD_SEPARATE in your project
//  preprocessor definitions to prevent duplicate symbols.
// -----------------------------------------------------------------------------
#ifndef HOME_FEATURES_BUILD_SEPARATE
    #include "FeaturesCodec.cpp"
    #include "HomeFeaturesPage.cpp"
#endif
