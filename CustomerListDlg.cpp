#include "stdafx.h"
#include "CustomerListDlg.h"

CCustomerListDlg::CCustomerListDlg() : CDialog(IDD_CUSTOMER_LIST_DIALOG)
{
}

CCustomerListDlg::~CCustomerListDlg()
{
}

BOOL CCustomerListDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ListView kontrolünü bağla
    m_list.AttachDlgItem(IDC_CUST_LIST_VIEW, *this);
    
    // Satır seçimi stilini ekle
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Kolonları Oluştur
    SetupColumns();

    // Verileri Yükle
    LoadCustomers();

    return TRUE;
}

void CCustomerListDlg::SetupColumns()
{
    m_list.InsertColumn(0, _T("Ad Soyad"), LVCFMT_LEFT, 150);
    m_list.InsertColumn(1, _T("Telefon"), LVCFMT_LEFT, 100);
    m_list.InsertColumn(2, _T("TC Kimlik"), LVCFMT_LEFT, 100);
    m_list.InsertColumn(3, _T("Cari Kod"), LVCFMT_LEFT, 0); // Gizli Kolon (ID)
}

void CCustomerListDlg::LoadCustomers(const CString& filter)
{
    m_list.DeleteAllItems();

    DatabaseManager& db = DatabaseManager::GetInstance();
    
    // Tüm müşterileri çek (Gerçek senaryoda burada SQL ile filtreleme yapılabilir)
    // Şimdilik hepsini çekip C++ tarafında süzüyoruz veya DB fonksiyonuna filtre ekleyebilirsin.
    std::vector<Customer_cstr> customers = db.GetAllGlobal<Customer_cstr>();

    int index = 0;
    for (const auto& c : customers)
    {
        // Basit Filtreleme
        if (!filter.IsEmpty())
        {
            CString allText = c.AdSoyad + c.Telefon + c.TCKN;
            allText.MakeLower();
            CString filterLower = filter;
            filterLower.MakeLower();
            
            if (allText.Find(filterLower) == -1) continue;
        }

        // Listeye Ekle
        int i = m_list.InsertItem(index, c.AdSoyad);
        m_list.SetItemText(i, 1, c.Telefon);
        m_list.SetItemText(i, 2, c.TCKN);
        m_list.SetItemText(i, 3, c.Cari_Kod); // ID'yi sakla
        
        index++;
    }
}

// ListView Mesajlarını Yakala (Çift Tıklama İçin)
INT_PTR CCustomerListDlg::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_NOTIFY)
    {
        LPNMHDR pnmh = (LPNMHDR)lparam;
        if (pnmh->idFrom == IDC_CUST_LIST_VIEW && pnmh->code == NM_DBLCLK)
        {
            OnListViewDoubleClick();
            return TRUE;
        }
    }
    return CDialog::DialogProc(msg, wparam, lparam);
}

BOOL CCustomerListDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT id = LOWORD(wParam);

    // Arama Butonu
    if (id == IDC_CUST_LIST_BTN_SEARCH)
    {
        CString filter = GetDlgItemText(IDC_CUST_LIST_SEARCH);
        LoadCustomers(filter);
        return TRUE;
    }

    return CDialog::OnCommand(wParam, lParam);
}

void CCustomerListDlg::OnListViewDoubleClick()
{
    // Çift tıklandığında "Tamam" butonuna basılmış gibi davran
    OnOK();
}

void CCustomerListDlg::OnOK()
{
    // Seçili satırı bul
    int nSel = m_list.GetNextItem(-1, LVNI_SELECTED);
    if (nSel == -1)
    {
        MessageBox(_T("Lütfen listeden bir müşteri seçiniz."), _T("Seçim Yok"), MB_ICONWARNING);
        return;
    }

    // Seçili satırdan "Cari Kod"u al (3. Kolon - Gizli)
    CString cariKod = m_list.GetItemText(nSel, 3);

    // Veritabanından bu müşterinin TAM bilgilerini çek
    // (DatabaseManager'da GetCustomerByCode gibi bir fonksiyonun olduğunu varsayıyoruz)
    // Eğer yoksa, GetAllGlobal içinden bulabiliriz (ama yavaştır).
    // En temizi tekil sorgudur. Şimdilik listeyi tarayalım:
    
    DatabaseManager& db = DatabaseManager::GetInstance();
    std::vector<Customer_cstr> all = db.GetAllGlobal<Customer_cstr>();
    
    bool found = false;
    for(const auto& c : all) {
        if(c.Cari_Kod == cariKod) {
            m_selectedCustomer = c;
            found = true;
            break;
        }
    }

    if(found) {
        CDialog::OnOK(); // Pencereyi kapat ve IDOK dön
    } else {
        MessageBox(_T("Müşteri verisi bulunamadı."), _T("Hata"), MB_ICONERROR);
    }
}