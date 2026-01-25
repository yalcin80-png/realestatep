#include "stdafx.h"
#include "AppointmentListDlg.h"
#include "AppointmentDlg.h" // Detay penceresini buradan çağıracağız

CAppointmentListDlg::CAppointmentListDlg(const CString& custID, const CString& custName)
    : CDialog(IDD_APP_LIST_DLG), m_custID(custID), m_custName(custName)
{
}

BOOL CAppointmentListDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetWindowText(_T("Randevular: ") + m_custName);

    m_list.AttachDlgItem(IDC_APP_LIST_VIEW, *this);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Kolonlar
    m_list.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 0); // Gizli ID
    m_list.InsertColumn(1, _T("Tarih"), LVCFMT_LEFT, 110);
    m_list.InsertColumn(2, _T("Konu"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(3, _T("Durum"), LVCFMT_LEFT, 80);
    m_list.InsertColumn(4, _T("Mülk"), LVCFMT_LEFT, 80);

    RefreshList();
    return TRUE;
}

void CAppointmentListDlg::RefreshList()
{
    m_list.DeleteAllItems();
    auto apps = DatabaseManager::GetInstance().GetAppointmentsByCustomer(m_custID);

    for (const auto& a : apps)
    {
        int n = m_list.InsertItem(0, a.AppID);
        m_list.SetItemText(n, 1, a.AppDate);
        m_list.SetItemText(n, 2, a.Title);
        m_list.SetItemText(n, 3, a.Status);
        m_list.SetItemText(n, 4, a.PropertyID);
    }
}

void CAppointmentListDlg::OnDoubleClick()
{
    int nItem = m_list.GetSelectionMark();
    if (nItem == -1) return;

    // Seçili Randevu ID'sini al
    CString appID = m_list.GetItemText(nItem, 0);

    // Mevcut dialogu (CAppointmentDlg) açacağız ama parametre olarak ID vereceğiz
    // Bunun için CAppointmentDlg'yi güncellememiz lazım (3. Adıma bak)
    CAppointmentDlg dlg(m_custID, m_custName, _T(""), _T("")); 
    dlg.SetEditMode(appID); // Özel mod
    
    if (dlg.DoModal() == IDOK) {
        RefreshList(); // Değişiklik varsa listeyi yenile
    }
}

INT_PTR CAppointmentListDlg::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Çift Tıklama Yakalama (NM_DBLCLK)
    if (uMsg == WM_NOTIFY) {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->code == NM_DBLCLK && pnmh->idFrom == IDC_APP_LIST_VIEW) {
            OnDoubleClick();
            return TRUE;
        }
    }
    // Yeni Randevu Butonu
    if (uMsg == WM_COMMAND && LOWORD(wParam) == IDOK) {
        CAppointmentDlg dlg(m_custID, m_custName);
        if (dlg.DoModal() == IDOK) RefreshList();
        return TRUE;
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}