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
    constexpr int kTabHeaderReservePx = 24; // mevcut UI'yi a�a�� itmek i�in
    constexpr int kBottomSectionTop = 375;  // Resource.rc'deki �izgi �st s�n�r�
}

// K���k Yard�mc�lar
namespace {
    // G�venli metin okuma
    CString GetTextSafe(HWND hDlg, int id) {
        HWND hCtrl = ::GetDlgItem(hDlg, id);
        if (!hCtrl) return _T("");
        int len = ::GetWindowTextLength(hCtrl);
        if (len <= 0) return _T("");
        std::vector<TCHAR> buffer(len + 1);
        ::GetWindowText(hCtrl, buffer.data(), len + 1);
        return CString(buffer.data());
    }
    // G�venli metin yazma
    void SetTextSafe(HWND hDlg, int id, const CString& text) {
        ::SetDlgItemText(hDlg, id, text);
    }
}

CHomeDialog::CHomeDialog(DatabaseManager& dbManagerRef, DialogMode mode, const CString& cariKod, const CString& homeCodeToEdit)
    : CDialog(IDD_HOME_DIALOG), m_dbManager(dbManagerRef), m_dialogMode(mode), m_cariKod(cariKod), m_homeCodeToEdit(homeCodeToEdit) {
}

BOOL CHomeDialog::OnInitDialog() {
    CDialog::OnInitDialog();

    // Seviye-2: tab kontrol�n� kur (Genel + �zellikler + �evre)
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
        SetWindowText(_T("Daire / Ev D�zenle"));
        if (!m_homeCodeToEdit.IsEmpty()) {
            auto dataMap = m_dbManager.FetchRecordMap(TABLE_NAME_HOME, _T("Home_Code"), m_homeCodeToEdit);
            if (!dataMap.empty()) {
                m_dbManager.Bind_Data_To_UI(this, TABLE_NAME_HOME, dataMap);
                if (dataMap.find(_T("Cari_Kod")) != dataMap.end()) m_cariKod = dataMap[_T("Cari_Kod")];

                // Seviye-2: �zellik tablar�n� DB verisiyle doldur
                m_featuresPage1.LoadFromMap(dataMap);
                m_featuresPage2.LoadFromMap(dataMap);
            }
            else {
                MessageBox(_T("Kay�t bulunamad�!"), _T("Hata"), MB_ICONERROR);
            }
        }
    }
    LayoutTabAndPages();
    FixTabFonts();
    // Yeni kay�tta default olarak bo� �zellikler
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

    // 2. �zellik Sekmeleri (Tab-1 ve Tab-2) i�in AYRI map'ler kullan
    std::map<CString, CString> mapPage1, mapPage2;

    // Sayfalar�n Handle'� varsa (Window olu�mu�sa) verileri �ek
    if (m_featuresPage1.GetHwnd() && ::IsWindow(m_featuresPage1.GetHwnd())) {
        m_featuresPage1.SaveToMap(mapPage1);
    }
    if (m_featuresPage2.GetHwnd() && ::IsWindow(m_featuresPage2.GetHwnd())) {
        m_featuresPage2.SaveToMap(mapPage2);
    }

    // 3. Map verilerini ana uiData map'ine g�venli aktar (�ak��ma �nleme)
    // Page 1: Cephe ve �� �zellikler
    for (auto const& [key, val] : mapPage1) {
        if (!val.IsEmpty()) uiData[key] = val;
    }
    // Page 2: D��, Muhit ve Di�er
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

    // 5. Struct'a d�n��t�r ve Kaydet
    Home_cstr h;
    for (const auto& [key, val] : uiData) {
        DatabaseManager::SetFieldByStringName(h, key, val);
    }

    bool success = (m_dialogMode == INEWUSER) ? m_dbManager.InsertGlobal(h) : m_dbManager.UpdateGlobal(h);

    if (success) {
        MessageBox(_T("Kay�t Ba�ar�l�!"), _T("Bilgi"), MB_ICONINFORMATION);
        CDialog::OnOK();
    }
    else {
        MessageBox(_T("Veritaban� kay�t hatas�!"), _T("Hata"), MB_ICONERROR);
    }
}

// ============================================================================
// Seviye-2: Tab Control + Dinamik �zellik Sayfalar�
// ============================================================================

void CHomeDialog::InitTabs()
{
    if (m_hTab) return;

    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icc);

    RECT rcClient{};
    ::GetClientRect(*this, &rcClient);

    // Tab, t�m �st alan� kapsas�n; alt butonlar sabit kals�n.
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

    tci.pszText = (LPTSTR)_T("�zellikler");
    TabCtrl_InsertItem(m_hTab, 1, &tci);

    tci.pszText = (LPTSTR)_T("�evre/Detay");
    TabCtrl_InsertItem(m_hTab, 2, &tci);

    // Genel UI'nin tab ba�l���n� kapatmamas� i�in bir defal�k a�a�� itme
    ShiftGeneralControlsForTabHeader();
    CollectGeneralControls();

    // Tab body alan�
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

        // �st ana alan: sekmelerle y�netilecek
        if (rc.top < kBottomSectionTop)
            m_generalControls.push_back(h);
    }
}

void CHomeDialog::ShiftGeneralControlsForTabHeader()
{
    if (m_layoutShifted) return;
    m_layoutShifted = true;

    // Tab ba�l��� i�in �stte ~24px yer a�
    for (HWND h = ::GetWindow(*this, GW_CHILD); h; h = ::GetWindow(h, GW_HWNDNEXT))
    {
        if (h == m_hTab) continue;
        int id = (int)::GetDlgCtrlID(h);
        if (id == IDOK || id == IDCANCEL) continue;

        RECT rc{};
        ::GetWindowRect(h, &rc);
        ::MapWindowPoints(nullptr, *this, (LPPOINT)&rc, 2);

        // G�r�nmez/0x0 kontrolleri elleme
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

    // ? DO�RU: Tab'�n kendi client rect�i �zerinden body hesapla
    RECT rcBody{};
    ::GetClientRect(m_hTab, &rcBody);
    TabCtrl_AdjustRect(m_hTab, FALSE, &rcBody); // art�k do�ru koordinat sisteminde

    // ? DO�RU: Sayfalar�n parent'� TAB olmal�
    if (!m_featuresPage1.GetHwnd())
        m_featuresPage1.Create(m_hTab, rcBody, 6001, CHomeFeaturesPage::PageKind::Features1);

    if (!m_featuresPage2.GetHwnd())
        m_featuresPage2.Create(m_hTab, rcBody, 6002, CHomeFeaturesPage::PageKind::Features2);

    // ? Sayfalar� tab i�inde konumland�r
    ::SetWindowPos(m_featuresPage1, nullptr,
        rcBody.left, rcBody.top,
        rcBody.right - rcBody.left, rcBody.bottom - rcBody.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    ::SetWindowPos(m_featuresPage2, nullptr,
        rcBody.left, rcBody.top,
        rcBody.right - rcBody.left, rcBody.bottom - rcBody.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    // �stersen burada mevcut se�ili taba g�re g�r�n�rl�k de g�ncellenebilir:
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

    // Tab-1/2: �zellik sayfalar�
    ::ShowWindow(m_featuresPage1, (index == 1) ? SW_SHOW : SW_HIDE);
    ::ShowWindow(m_featuresPage2, (index == 2) ? SW_SHOW : SW_HIDE);

    if (m_hTab)
        TabCtrl_SetCurSel(m_hTab, index);
}

void CHomeDialog::OnLoadFromClipboard() {
    CString rawText = GetClipboardText();
    if (rawText.IsEmpty()) {
        MessageBox(_T("Panoda metin yok!"), _T("Uyar�"), MB_ICONWARNING);
        return;
    }

    std::map<CString, CString> parsedData = ParseSahibindenText(rawText);
    if (parsedData.empty()) {
        MessageBox(_T("Metinden veri ��kar�lamad�."), _T("Hata"), MB_ICONERROR);
        return;
    }

    std::map<CString, CString> schemaData = NormalizeToSchemaMap(parsedData);
    SanitizeDataMap(schemaData);

    std::map<CString, CString> currentData;
    m_dbManager.Bind_UI_To_Data(this, TABLE_NAME_HOME, currentData);
    for (const auto& [key, val] : schemaData) if (!val.IsEmpty()) currentData[key] = val;
    m_dbManager.Bind_Data_To_UI(this, TABLE_NAME_HOME, currentData);

    MessageBox(_T("Veriler aktar�ld�."), _T("Tamam"), MB_ICONINFORMATION);
}

// ============================================================================
// PARSE FONKS�YONU (D�zeltilmi� ve G��lendirilmi�)
// ============================================================================
std::map<CString, CString> CHomeDialog::ParseSahibindenText(const CString& rawText)
{
    std::map<CString, CString> out;
    CString text = rawText;

    // Sat�r sonlar�n� normalize et
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

    // Alias haritas� (E�anlaml�lar)
    static const std::map<CString, CString> alias = {
        {_T("Is�tma"), _T("Is�tma Tipi")},
        {_T("E�yal�"), _T("E�yal� m�")},
        {_T("Site ��erisinde"), _T("Site ��inde")},
        {_T("Aidat (TL)"), _T("Aidat")},
        // Emlak Tipi ve �lan Tarihi'ni a�a��da �zel olarak ele al�yoruz
    };

    for (size_t i = 0; i < lines.size(); ++i)
    {
        CString cLine = lines[i];

        // 1?? ADRES (�ehir / �l�e / Mahalle)
        if (cLine.Find(_T('/')) != -1 && cLine.Find(_T("�lan No")) == -1 && cLine.Find(_T("TL")) == -1)
        {
            CString temp = cLine;
            temp.Replace(_T(" / "), _T("/"));
            int p1 = temp.Find(_T('/'));
            int p2 = temp.Find(_T('/'), p1 + 1);
            if (p1 != -1) {
                out[_T("�ehir")] = temp.Left(p1);
                if (p2 != -1) {
                    out[_T("�l�e")] = temp.Mid(p1 + 1, p2 - p1 - 1);
                    out[_T("Mahalle")] = temp.Mid(p2 + 1);
                }
                else {
                    out[_T("�l�e")] = temp.Mid(p1 + 1);
                }
                CString addr = temp; addr.Replace(_T("/"), _T(", "));
                out[_T("Adres")] = addr;
            }
            continue;
        }

        // 2?? F�YAT
        if (cLine.Find(_T("TL")) != -1 || cLine.Find(_T("USD")) != -1 || cLine.Find(_T("EUR")) != -1)
        {
            if (out.find(_T("Fiyat")) == out.end()) { // Sadece ilk fiyat� al
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

        // 3?? �LAN NO (�zel Blok)
        if (cLine.Left(7).CompareNoCase(_T("�lan No")) == 0)
        {
            CString val = cLine.Mid(7); val.Trim();
            if (val == _T(":") || val.IsEmpty()) { // De�er alt sat�rda
                if (i + 1 < lines.size()) {
                    out[_T("�lan No")] = lines[i + 1];
                    i++;
                }
            }
            else { // De�er ayn� sat�rda (�lan No: 12345)
                if (val.Left(1) == _T(":")) val = val.Mid(1);
                out[_T("�lan No")] = val;
            }
            continue;
        }

        // 4?? �LAN TAR�H� (�zel Blok - D�zeltildi)
        if (cLine.Left(11).CompareNoCase(_T("�lan Tarihi")) == 0)
            // 4?? �LAN TAR�H� (�zel Blok - S�per Esnek Versiyon)
        {
            CString lower = cLine;
            lower.MakeLower();

            // sat�rda hem "ilan" hem de "tarihi" ge�iyorsa, bunu ilan tarihi sat�r� say
            if (lower.Find(_T("ilan")) != -1 && lower.Find(_T("tarihi")) != -1)
            {
                CString val;

                // E�er ayn� sat�rda ":" sonras� bir �ey varsa onu al
                int colonPos = cLine.Find(_T(':'));
                if (colonPos != -1)
                {
                    val = cLine.Mid(colonPos + 1);
                    val.Trim();
                }

                // De�er h�l� bo�sa alt sat�rdan al (sahibinden genelde b�yle yap�yor)
                if (val.IsEmpty() && i + 1 < (int)lines.size())
                {
                    val = lines[i + 1];
                    val.Trim();
                    i++;    // alt sat�r� da t�kettik
                }

                if (!val.IsEmpty())
                {
                    out[_T("�lan Tarihi")] = val;
                }

                continue;
            }
        }


        // 5?? EMLAK T�P� (�zel Blok - D�zeltildi)
        if (cLine.Left(10).CompareNoCase(_T("Emlak Tipi")) == 0)
        {
            CString val = cLine.Mid(10); val.Trim();
            if (val == _T(":") || val.IsEmpty()) { // De�er alt sat�rda
                if (i + 1 < lines.size()) {
                    out[_T("PropertyType")] = lines[i + 1];
                    i++;
                }
            }
            else { // De�er ayn� sat�rda (Emlak Tipi Sat�l�k Daire)
                if (val.Left(1) == _T(":")) val = val.Mid(1);
                val.Trim();
                out[_T("PropertyType")] = val;
            }
            continue;
        }

        // 6?? D��ER ALANLAR (Genel Tarama)
        CString key = cLine;
        bool handled = false;

        // Alias kontrol�
        if (alias.find(key) != alias.end()) key = alias.at(key);

        // Bilinen anahtarlar listesi (E�er alias de�ilse)
        CString knownKeys[] = {
            _T("Oda Say�s�"), _T("Bina Ya��"), _T("Bulundu�u Kat"), _T("Kat Say�s�"),
            _T("Is�tma"), _T("Banyo Say�s�"), _T("Mutfak"), _T("Balkon"), _T("Asans�r"),
            _T("Otopark"), _T("E�yal�"), _T("Kullan�m Durumu"), _T("Site ��erisinde"),
            _T("Site Ad�"), _T("Aidat"), _T("Krediye Uygun"), _T("Tapu Durumu"),
            _T("Kimden"), _T("Takas"), _T("m� (Br�t)"), _T("m� (Net)")
        };

        // Sat�r�n kendisi bir anahtar m�? (Alt alta durumu)
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

        // Sat�r "Anahtar: De�er" veya "Anahtar De�er" format�nda m�? (Yan yana durumu)
        // �rn: "Oda Say�s� 3+1"
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

    // 7?? Fiyat / m� hesapla (G��lendirilmi�)
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

    // Hem Fiyat hem de Net m� varsa hesapla
    auto itFiyat = out.find(_T("Fiyat"));
    auto itNet = out.find(_T("m� (Net)"));

    if (itFiyat != out.end() && itNet != out.end())
    {
        double fiyat = ToNumber(itFiyat->second);
        double net = ToNumber(itNet->second);

        if (fiyat > 0.0 && net > 0.0)
        {
            double per = fiyat / net;   // m� fiyat�
            CString perStr;
            perStr.Format(_T("%.0f"), per);   // tam say� olarak

            // Binlik ayra� ekle: 30950 -> 30.950
            int len = perStr.GetLength();
            for (int k = len - 3; k > 0; k -= 3)
                perStr.Insert(k, _T("."));

            out[_T("Fiyat/m�")] = perStr;
        }
    }

    out[_T("Website")] = _T("sahibinden.com");
    return out;
}

std::map<CString, CString> CHomeDialog::NormalizeToSchemaMap(const std::map<CString, CString>& rawFields)
{
    std::map<CString, CString> schema;
    auto get = [&](const CString& k) { auto it = rawFields.find(k); return (it != rawFields.end()) ? it->second : CString(); };

    schema[_T("ListingNo")] = get(_T("�lan No"));
    schema[_T("ListingDate")] = get(_T("�lan Tarihi"));
    schema[_T("Price")] = get(_T("Fiyat"));
    schema[_T("Currency")] = get(_T("Para Birimi"));
    schema[_T("PricePerM2")] = get(_T("Fiyat/m�"));

    schema[_T("City")] = get(_T("�ehir"));
    schema[_T("District")] = get(_T("�l�e"));
    schema[_T("Neighborhood")] = get(_T("Mahalle"));
    schema[_T("Address")] = get(_T("Adres"));

    // PropertyType do�rudan geliyor
    schema[_T("PropertyType")] = get(_T("PropertyType"));

    schema[_T("RoomCount")] = get(_T("Oda Say�s�"));
    schema[_T("NetArea")] = get(_T("m� (Net)"));
    schema[_T("GrossArea")] = get(_T("m� (Br�t)"));
    schema[_T("BuildingAge")] = get(_T("Bina Ya��"));
    schema[_T("Floor")] = get(_T("Bulundu�u Kat"));
    schema[_T("TotalFloor")] = get(_T("Kat Say�s�"));

    schema[_T("HeatingType")] = get(_T("Is�tma Tipi"));
    schema[_T("BathroomCount")] = get(_T("Banyo Say�s�"));
    schema[_T("KitchenType")] = get(_T("Mutfak"));

    schema[_T("Balcony")] = get(_T("Balkon"));
    schema[_T("Elevator")] = get(_T("Asans�r"));
    schema[_T("Parking")] = get(_T("Otopark"));
    schema[_T("Furnished")] = get(_T("E�yal� m�"));
    schema[_T("InSite")] = get(_T("Site ��inde"));
    schema[_T("SiteName")] = get(_T("Site Ad�"));
    schema[_T("Dues")] = get(_T("Aidat"));

    schema[_T("CreditEligible")] = get(_T("Krediye Uygun"));
    schema[_T("DeedStatus")] = get(_T("Tapu Durumu"));
    schema[_T("SellerType")] = get(_T("Kimden"));
    schema[_T("Swap")] = get(_T("Takas"));
    schema[_T("UsageStatus")] = get(_T("Kullan�m Durumu"));

    schema[_T("WebsiteName")] = _T("sahibinden.com");
    if (schema[_T("Status")].IsEmpty()) schema[_T("Status")] = _T("Aktif");

    return schema;
}

void CHomeDialog::SanitizeDataMap(std::map<CString, CString>& dataMap)
{
    // Veritaban� say�sal oldu�u i�in kay�t s�ras�nda bo� metin gitmemeli
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
        else if (v.CompareNoCase(_T("Hay�r")) == 0 || v.CompareNoCase(_T("Yok")) == 0 || v == _T("0") || v.CompareNoCase(_T("False")) == 0)
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
    std::vector<CString> yesno = { _T("Evet"), _T("Hay�r") };
    std::vector<CString> types = { _T("Sat�l�k Daire"), _T("Kiral�k Daire"), _T("Sat�l�k ��yeri"), _T("Kiral�k ��yeri"), _T("Sat�l�k Arsa"), _T("Kiral�k Arsa"), _T("Sat�l�k Villa"), _T("Kiral�k Villa"), _T("Sat�l�k Yazl�k"), _T("Kiral�k Yazl�k"), _T("Sat�l�k Bina"), _T("Devren Sat�l�k") };
    std::vector<CString> isitma = { _T("Kombi (Do�algaz)"), _T("Kombi (Elektrik)"), _T("Merkezi"), _T("Merkezi (Pay �l�er)"), _T("Soba"), _T("Do�algaz Sobas�"), _T("Kat Kaloriferi"), _T("Yerden Is�tma"), _T("Klima"), _T("Yok") };
    std::vector<CString> mutfak = { _T("Kapal�"), _T("A��k (Amerikan)"), _T("Ankastre") };
    std::vector<CString> tapu = { _T("Kat M�lkiyetli"), _T("Kat �rtifakl�"), _T("Hisseli"), _T("M�stakil"), _T("Arsa"), _T("Bilinmiyor") };
    std::vector<CString> kimden = { _T("Sahibinden"), _T("Emlak Ofisinden"), _T("Bankadan"), _T("�n�aat Firmas�ndan") };
    std::vector<CString> currency = { _T("TL"), _T("USD"), _T("EUR"), _T("GBP") };
    std::vector<CString> usage = { _T("Bo�"), _T("Kirac�l�"), _T("M�lk Sahibi") };
    std::vector<CString> status = { _T("Aktif"), _T("Opsiyonlu"), _T("Sat�ld�"), _T("Kiraland�"), _T("Pasif") };
    std::vector<CString> otopark = { _T("A��k Otopark"), _T("Kapal�"), _T("A��k & Kapal� Otopark"), _T("Yok") };
    std::vector<CString> website = { _T("sahibinden.com"), _T("hepsiemlak"), _T("emlakjet"), _T("zingat"), _T("Di�er") };

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

    if (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_BUTTON_ILANBILGIAL) {
        OnIlanBilgileriniAl();
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
        return FALSE; // default i�lemler devam edebilir
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}

void CHomeDialog::UpdateScrollInfo()
{
    CRect rc;
    rc =  GetClientRect();

    // ��erik y�ksekli�ini hesapla (�rn: Kontrollerin bitti�i yer)
    // E�er kontrolleriniz dinamikse buray� 600-800 gibi bir de�er yapabilirsiniz.
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

    // 2) Tab kontrol�ne uygula
    if (m_hTab)
        ::SendMessage(m_hTab, WM_SETFONT, (WPARAM)m_hUiFont, TRUE);

    // 3) Tab sayfalar�na ve i�indeki dinamik kontrollere uygula
    if (m_featuresPage1.GetHwnd())
        ApplyFontRecursive(m_featuresPage1, m_hUiFont);

    if (m_featuresPage2.GetHwnd())
        ApplyFontRecursive(m_featuresPage2, m_hUiFont);
}

// İlan Bilgilerini Al button click handler
void CHomeDialog::OnIlanBilgileriniAl() {
    CString ilanNumarasi;
    GetDlgItemText(IDC_EDIT_ILANNUMARASI, ilanNumarasi);

    if (ilanNumarasi.IsEmpty()) {
        MessageBox(_T("İlan numarasını girmelisiniz!"), _T("Hata"), MB_ICONERROR);
        return;
    }

    SahibindenImporter importer;
    auto ilanBilgisi = importer.FetchByIlanNumarasi(ilanNumarasi);

    if (ilanBilgisi) {
        // Map the fetched data to dialog controls
        // For now, we'll populate available fields
        // Note: The actual field mappings depend on the available controls
        
        // Set Title/Description to General Notes if available
        if (!ilanBilgisi->Baslik.empty()) {
            SetDlgItemText(IDC_EDIT_NOTE_GENERAL, ilanBilgisi->Baslik.c_str());
        }
        
        // Set Price if available
        if (!ilanBilgisi->Fiyat.empty()) {
            SetDlgItemText(ID_EDIT_FIYAT, ilanBilgisi->Fiyat.c_str());
        }
        
        // Append Description to internal notes if available
        if (!ilanBilgisi->Aciklama.empty()) {
            CString currentNotes;
            GetDlgItemText(IDC_EDIT_NOTE_INTERNAL, currentNotes);
            if (!currentNotes.IsEmpty()) {
                currentNotes += _T("\r\n");
            }
            currentNotes += ilanBilgisi->Aciklama.c_str();
            SetDlgItemText(IDC_EDIT_NOTE_INTERNAL, currentNotes);
        }
        
        // Also set the İlan No field
        SetDlgItemText(IDC_EDIT_ILAN_NO, ilanNumarasi);
        
        MessageBox(_T("İlan bilgileri başarıyla alındı!"), _T("Bilgi"), MB_ICONINFORMATION);
    } else {
        MessageBox(_T("İlan bilgileri alınamadı! Lütfen ilan numarasını kontrol edin."), _T("Hata"), MB_ICONERROR);
    }
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
