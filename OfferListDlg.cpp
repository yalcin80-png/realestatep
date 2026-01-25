#include "stdafx.h"
#include "dataIsMe.h"
#include "OfferListDlg.h"

COfferListDlg::COfferListDlg(const CString& propID) 
    : CDialog(IDD_OFFER_LIST_DIALOG), m_propertyID(propID)
{
}

BOOL COfferListDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ListView'i bağla
    m_list.AttachDlgItem(IDC_OFFER_LIST_VIEW, *this);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    SetupColumns();
    LoadData();

    return TRUE;
}

void COfferListDlg::SetupColumns()
{
    m_list.InsertColumn(0, _T("Tarih"), LVCFMT_LEFT, 90);
    m_list.InsertColumn(1, _T("Teklif Veren"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(2, _T("Tutar"), LVCFMT_RIGHT, 100);
    m_list.InsertColumn(3, _T("Durum"), LVCFMT_CENTER, 80);
    m_list.InsertColumn(4, _T("Notlar"), LVCFMT_LEFT, 150);
}

void COfferListDlg::LoadData()
{
    m_list.DeleteAllItems();
    DatabaseManager& db = DatabaseManager::GetInstance();
    
    // Verileri Çek
    std::vector<Offer_cstr> offers = db.GetOffersByPropertyID(m_propertyID);

    int index = 0;
    for (const auto& o : offers)
    {
        int i = m_list.InsertItem(index, o.OfferDate);
        m_list.SetItemText(i, 1, o.CustomerID); // Burada Müşteri Adı var
        m_list.SetItemText(i, 2, o.OfferPrice);
        m_list.SetItemText(i, 3, o.Status);
        m_list.SetItemText(i, 4, o.Notes);
        index++;
    }
}