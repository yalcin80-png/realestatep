#include "stdafx.h"
#include "vVillaDlg.h"
#include "resource.h"

// --- Yard�mc�lar ---
static void ComboFillYesNo(HWND hCombo) {
    if (!hCombo) return;
    ::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Evet"));
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Hay�r"));
    ::SendMessage(hCombo, CB_SETCURSEL, 1, 0);
}

static void ComboFillCurrency(HWND hCombo) {
    if (!hCombo) return;
    ::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("TL"));
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("USD"));
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("EUR"));
    ::SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

static void ComboFillSellerType(HWND hCombo) {
    if (!hCombo) return;
    ::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Sahibinden"));
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Emlak Ofisi"));
    ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("�n�aat Firmas�"));
    ::SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

// CVillaPageBase Implementasyonu
CVillaPageBase::CVillaPageBase(UINT id) : CDialog(id) {}
INT_PTR CVillaPageBase::DialogProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ERASEBKGND: { CDC dc((HDC)wp); if (OnEraseBkgnd(dc)) return TRUE; break; }
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        CDC dc((HDC)wp);
        HBRUSH br = OnCtlColor(dc, (HWND)lp, msg);
        if (br) return (INT_PTR)br;
        break;
    }
    }
    return CDialog::DialogProc(msg, wp, lp);
}
BOOL CVillaPageBase::OnEraseBkgnd(CDC& dc) {
    CRect rc = GetClientRect();
    dc.FillRect(rc, (HBRUSH)GetSysColorBrush(COLOR_3DFACE));
    return TRUE;
}
HBRUSH CVillaPageBase::OnCtlColor(CDC& dc, HWND hwnd, UINT nCtlColor) {
    if (nCtlColor == WM_CTLCOLORSTATIC || nCtlColor == WM_CTLCOLORDLG) {
        dc.SetBkMode(TRANSPARENT);
        return GetSysColorBrush(COLOR_3DFACE);
    }
    return NULL;
}

// CVillaPage Implementasyonu
CVillaPage::CVillaPage(UINT idd) : CVillaPageBase(idd) {}
void CVillaPage::Create(Win32xx::CTab& tab) { CDialog::Create(tab); ShowWindow(SW_HIDE); }

// CVillaDialog Implementasyonu
CVillaDialog::CVillaDialog(DatabaseManager& db, DialogMode mode, const CString& cariKod, const CString& villaCode)
    : CVillaPageBase(IDD_VILLA_DIALOG), m_db(db), m_mode(mode), m_cariKod(cariKod), m_villaCode(villaCode),
    m_pageGenel(IDD_VILLA_PAGE_GENEL), m_pageNotlar(IDD_VILLA_PAGE_NOTLAR) {
}

BOOL CVillaDialog::OnInitDialog()
{
    CVillaPageBase::OnInitDialog();

    // Font Ayar�
    LOGFONT lf{}; SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
    _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));
    lf.lfHeight = -MulDiv(9, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72);
    m_font.CreateFontIndirect(lf);
    SetFont(m_font, TRUE);

    m_tab.AttachDlgItem(IDC_TAB_VILLA, *this);
    m_tab.SetFont(m_font, TRUE);

    // Sekmeleri do�rudan ekle (Tab i�inde Tab bitti)
    TCITEM ti0{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Genel") }; m_tab.InsertItem(0, &ti0);
    TCITEM ti1{ TCIF_TEXT, 0, 0, (LPTSTR)_T("A��klama / Notlar") }; m_tab.InsertItem(1, &ti1);
    TCITEM ti2{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Cephe / ��") }; m_tab.InsertItem(2, &ti2);
    TCITEM ti3{ TCIF_TEXT, 0, 0, (LPTSTR)_T("D�� / Muhit") }; m_tab.InsertItem(3, &ti3);

    // Sayfalar� Olu�tur
    m_pageGenel.Create(m_tab);
    m_pageNotlar.Create(m_tab);

    CRect rcPage = m_tab.GetClientRect();
    m_tab.AdjustRect(FALSE, rcPage);

    // �zellik sayfalar�n� do�rudan ana taba ba�l�yoruz
    m_pageOzellik1.Create(m_tab, rcPage, 9101, CHomeFeaturesPage::PageKind::Features1);
    m_pageOzellik2.Create(m_tab, rcPage, 9102, CHomeFeaturesPage::PageKind::Features2);

    m_pageOzellik1.SetFont(m_font, TRUE);
    m_pageOzellik2.SetFont(m_font, TRUE);

    InitCombos();
    RecalcLayout();
    ShowPage(0);

    if (m_mode != INEWUSER) LoadFromDB();
    else {
        if (!m_cariKod.IsEmpty()) ::SetWindowText(m_pageGenel.GetDlgItem(IDC_EDIT_VILLA_CARI_KOD), m_cariKod);
        if (m_villaCode.IsEmpty()) m_villaCode = m_db.GenerateNextVillaCode();
        ::SetWindowText(m_pageGenel.GetDlgItem(IDC_EDIT_VILLA_CODE), m_villaCode);
    }

    return TRUE;
}

INT_PTR CVillaDialog::DialogProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE: OnSize(LOWORD(lp), HIWORD(lp)); return TRUE;
    case WM_NOTIFY: {
        LPNMHDR pnm = (LPNMHDR)lp;
        if (pnm->idFrom == IDC_TAB_VILLA && pnm->code == TCN_SELCHANGE) {
            RecalcLayout();  // Ensure pages are properly sized before showing
            ShowPage(m_tab.GetCurSel());
            return TRUE;
        }
        break;
    }
    }
    return CVillaPageBase::DialogProc(msg, wp, lp);
}

void CVillaDialog::ShowPage(int page) {
    m_pageGenel.ShowWindow(page == 0 ? SW_SHOW : SW_HIDE);
    m_pageNotlar.ShowWindow(page == 1 ? SW_SHOW : SW_HIDE);
    m_pageOzellik1.ShowWindow(page == 2 ? SW_SHOW : SW_HIDE);
    m_pageOzellik2.ShowWindow(page == 3 ? SW_SHOW : SW_HIDE);
}

void CVillaDialog::OnSize(int width, int height) {
    if (m_tab.IsWindow()) {
        CRect rc; rc=  GetClientRect();
        CWnd btnOk = GetDlgItem(IDOK);
        if (btnOk.IsWindow()) {
            CRect rcBtn; rcBtn =  btnOk.GetWindowRect(); ScreenToClient(rcBtn);
            rc.bottom = rcBtn.top - 10;
        }
        m_tab.MoveWindow(rc, TRUE);
        RecalcLayout();
    }
}

void CVillaDialog::RecalcLayout() {
    if (!m_tab.IsWindow()) return;
    CRect rc = m_tab.GetClientRect();
    m_tab.AdjustRect(FALSE, rc);

    if (m_pageGenel.IsWindow()) m_pageGenel.MoveWindow(rc, TRUE);
    if (m_pageNotlar.IsWindow()) m_pageNotlar.MoveWindow(rc, TRUE);
    if (m_pageOzellik1.IsWindow()) m_pageOzellik1.MoveWindow(rc, TRUE);
    if (m_pageOzellik2.IsWindow()) m_pageOzellik2.MoveWindow(rc, TRUE);
}

void CVillaDialog::LoadFromDB() {
    if (m_villaCode.IsEmpty()) return;
    Villa_cstr d = m_db.GetVillaByCode(m_villaCode);

    m_db.Bind_Data_To_UI(&m_pageGenel, d);
    m_db.Bind_Data_To_UI(&m_pageNotlar, d);

    // Checkbox sayfalar� i�in map haz�rl���
    std::map<CString, CString> m;
    m[_T("Facades")] = d.Facades;
    m[_T("FeaturesInterior")] = d.FeaturesInterior;
    m[_T("FeaturesExterior")] = d.FeaturesExterior;
    m[_T("FeaturesNeighborhood")] = d.FeaturesNeighborhood;
    m[_T("FeaturesTransport")] = d.FeaturesTransport;
    m[_T("FeaturesView")] = d.FeaturesView;
    m[_T("HousingType")] = d.HousingType;
    m[_T("FeaturesAccessibility")] = d.FeaturesAccessibility;

    m_pageOzellik1.LoadFromMap(m);
    m_pageOzellik2.LoadFromMap(m);
}

void CVillaDialog::OnOK()
{
    Villa_cstr d;

    // 1. Genel ve Notlar sekmelerini standart binding ile oku
    m_db.Bind_UI_To_Data(&m_pageGenel, d);
    m_db.Bind_UI_To_Data(&m_pageNotlar, d);

    // 2. �zellik sayfalar� (Checkboxlar) i�in ayr� map'ler kullan
    std::map<CString, CString> map1, map2;

    // Sayfalar�n olu�turulup olu�turulmad���n� kontrol ederek verileri �ek
    if (m_pageOzellik1.IsWindow()) m_pageOzellik1.SaveToMap(map1);
    if (m_pageOzellik2.IsWindow()) m_pageOzellik2.SaveToMap(map2);

    // 3. Map1 verilerini aktar (Cephe / ��)
    if (map1.count(_T("Facades"))) d.Facades = map1[_T("Facades")];
    if (map1.count(_T("FeaturesInterior"))) d.FeaturesInterior = map1[_T("FeaturesInterior")];

    // 4. Map2 verilerini aktar (D�� / Muhit / Ula��m / Manzara / Konut Tipi / Eri�ilebilirlik)
    if (map2.count(_T("FeaturesExterior"))) d.FeaturesExterior = map2[_T("FeaturesExterior")];
    if (map2.count(_T("FeaturesNeighborhood"))) d.FeaturesNeighborhood = map2[_T("FeaturesNeighborhood")];
    if (map2.count(_T("FeaturesTransport"))) d.FeaturesTransport = map2[_T("FeaturesTransport")];
    if (map2.count(_T("FeaturesView"))) d.FeaturesView = map2[_T("FeaturesView")];
    if (map2.count(_T("HousingType"))) d.HousingType = map2[_T("HousingType")];
    if (map2.count(_T("FeaturesAccessibility"))) d.FeaturesAccessibility = map2[_T("FeaturesAccessibility")];

    // 5. Kimlik bilgilerini tamamla
    d.Cari_Kod = m_cariKod;
    if (d.Villa_Code.IsEmpty())
        d.Villa_Code = m_villaCode.IsEmpty() ? m_db.GenerateNextVillaCode() : m_villaCode;

    // 6. Veritaban� ��lemi
    bool ok = (m_mode == DialogMode::IUPDATEUSER) ? m_db.UpdateGlobal(d) : m_db.InsertGlobal(d);

    if (ok)
        EndDialog(IDOK);
    else
        ::MessageBox(GetHwnd(), _T("Kay�t s�ras�nda veritaban� hatas� olu�tu."), _T("Hata"), MB_ICONERROR);
}
void CVillaDialog::InitCombos() {
    HWND h;
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_HAVUZ); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_BAHCE); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_GARAJ); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_PARKING); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_FURNISHED); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_INSITE); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_CREDIT); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_SWAP); ComboFillYesNo(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_CURRENCY); ComboFillCurrency(h);
    h = m_pageGenel.GetDlgItem(IDC_COMBO_VILLA_SELLER_TYPE); ComboFillSellerType(h);
}