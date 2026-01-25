#include "stdafx.h"
#include "PropertySelectDlg.h"

// Resource.h içine eklemen gereken ID'ler:
// #define IDD_PROP_SELECT_DLG 7000
// #define IDC_PROP_SEL_LIST   7001
// #define IDC_PROP_SEL_COMBO  7002

CPropertySelectDlg::CPropertySelectDlg(const CString& cariKod) 
    : CDialog(IDD_PROP_SELECT_DLG), m_cariKod(cariKod) 
{
}

BOOL CPropertySelectDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Kontrolleri Bağla (Win32++ Attach)
    m_list.AttachDlgItem(IDC_PROP_SEL_LIST, *this);
    m_comboType.AttachDlgItem(IDC_PROP_SEL_COMBO, *this);

    // Listview Kolonları
    m_list.InsertColumn(0, _T("Kod / İlan No"), LVCFMT_LEFT, 100);
    m_list.InsertColumn(1, _T("Tip"), LVCFMT_LEFT, 80);
    m_list.InsertColumn(2, _T("Özet Bilgi (Adres/Ada/Parsel)"), LVCFMT_LEFT, 250);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Combo Doldur
    m_comboType.AddString(_T("Tümü"));
    m_comboType.AddString(_T("Konut"));
    m_comboType.AddString(_T("Arsa"));
    m_comboType.AddString(_T("Tarla"));
    m_comboType.SetCurSel(0); // Tümü seçili

    RefreshList();
    return TRUE;
}

void CPropertySelectDlg::RefreshList()
{
    m_list.DeleteAllItems();
    int filter = m_comboType.GetCurSel(); // 0:Tümü, 1:Konut, 2:Arsa, 3:Tarla
    
    DatabaseManager& db = DatabaseManager::GetInstance();

    // 1. KONUTLAR
    if (filter == 0 || filter == 1) {
        auto homes = db.GetHomesByCariKod(m_cariKod);
        for (const auto& h : homes) {
            int n = m_list.InsertItem(0, h.Home_Code);
            m_list.SetItemText(n, 1, _T("Konut"));
            // Özet bilgi: İl/İlçe + Adres
            CString info; info.Format(_T("%s / %s - %s"), h.City.GetString(), h.District.GetString(), h.Address.GetString());
            m_list.SetItemText(n, 2, info);
        }
    }

    // 2. ARSALAR
    if (filter == 0 || filter == 2) {
        auto lands = db.GetLandsByCariKod(m_cariKod);
        for (const auto& l : lands) {
            int n = m_list.InsertItem(0, l.Land_Code);
            m_list.SetItemText(n, 1, _T("Arsa"));
            CString info; info.Format(_T("Ada:%s Parsel:%s - %s"), l.Ada.GetString(), l.Parsel.GetString(), l.Lokasyon.GetString());
            m_list.SetItemText(n, 2, info);
        }
    }

    // 3. TARLALAR
    if (filter == 0 || filter == 3) {
        auto fields = db.GetFieldsByCariKod(m_cariKod);
        for (const auto& f : fields) {
            int n = m_list.InsertItem(0, f.Field_Code);
            m_list.SetItemText(n, 1, _T("Tarla"));
            CString info; info.Format(_T("m²:%s - %s"), f.Metrekare.GetString(), f.Lokasyon.GetString());
            m_list.SetItemText(n, 2, info);
        }
    }
}

void CPropertySelectDlg::OnOK()
{
    int sel = m_list.GetSelectionMark();
    if (sel == -1) {
        MessageBox(_T("Lütfen listeden bir mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING);
        return;
    }

    m_selectedID = m_list.GetItemText(sel, 0);   // Kod
    m_selectedType = m_list.GetItemText(sel, 1); // Tip
    m_selectedInfo = m_list.GetItemText(sel, 2); // Bilgi

    CDialog::OnOK();
}

INT_PTR CPropertySelectDlg::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Combo değişirse listeyi yenile
    if (uMsg == WM_COMMAND && HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PROP_SEL_COMBO) {
        RefreshList();
        return TRUE;
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}