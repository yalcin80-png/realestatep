#include "stdafx.h"
#include "vCompanyDlg.h"

CCompanyDialog::CCompanyDialog(DatabaseManager& db) 
    : CDialog(IDD_COMPANY_DIALOG), m_db(db)
{
}

CCompanyDialog::~CCompanyDialog()
{
}

BOOL CCompanyDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 1. Veritabanından mevcut bilgileri çek (ID=1)
    m_data = m_db.GetCompanyInfo();

    // 2. Eğer kayıt yoksa (boşsa), varsayılan değerler atayabilirsin
    if (m_data.ID.IsEmpty()) {
        m_data.CompanyName = _T("Yeni Emlak Firması");
    }

    // 3. UI Binder ile kontrolleri doldur
    // (UIBinder Map yerine Struct destekliyorsa direkt struct verebilirsin, 
    // yoksa Map'e çevirip bind etmelisin. Aşağıda manuel örnek var.)
    
    SetDlgItemText(IDC_COMP_NAME, m_data.CompanyName);
    SetDlgItemText(IDC_COMP_OWNER, m_data.OwnerName);
    SetDlgItemText(IDC_COMP_TTBS, m_data.TTBSNo);
    SetDlgItemText(IDC_COMP_TAX_OFF, m_data.TaxOffice);
    SetDlgItemText(IDC_COMP_TAX_NO, m_data.TaxNo);
    SetDlgItemText(IDC_COMP_MERSIS, m_data.MersisNo);
    
    SetDlgItemText(IDC_COMP_TEL_FIX, m_data.PhoneFixed);
    SetDlgItemText(IDC_COMP_TEL_MOB, m_data.PhoneMobile);
    SetDlgItemText(IDC_COMP_EMAIL, m_data.Email);
    SetDlgItemText(IDC_COMP_WEB, m_data.Website);
    SetDlgItemText(IDC_COMP_ADDRESS, m_data.Address);
    
    SetDlgItemText(IDC_COMP_LOGO_PATH, m_data.LogoPath);
    SetDlgItemText(IDC_COMP_ABOUT, m_data.AboutUs);

    return TRUE;
}

void CCompanyDialog::OnOK()
{
    // Formdan verileri al
    m_data.CompanyName=GetDlgItemText(IDC_COMP_NAME);
    m_data.OwnerName  =GetDlgItemText(IDC_COMP_OWNER);
    m_data.TTBSNo     =GetDlgItemText(IDC_COMP_TTBS);
    m_data.TaxOffice  =GetDlgItemText(IDC_COMP_TAX_OFF);
    m_data.TaxNo      =GetDlgItemText(IDC_COMP_TAX_NO);
    m_data.MersisNo   =GetDlgItemText(IDC_COMP_MERSIS);
                      
    m_data.PhoneFixed =GetDlgItemText(IDC_COMP_TEL_FIX);
    m_data.PhoneMobile=GetDlgItemText(IDC_COMP_TEL_MOB);
    m_data.Email      =GetDlgItemText(IDC_COMP_EMAIL);
    m_data.Website    =GetDlgItemText(IDC_COMP_WEB);
    m_data.Address    =GetDlgItemText(IDC_COMP_ADDRESS);
                      
    m_data.LogoPath   =GetDlgItemText(IDC_COMP_LOGO_PATH);
    m_data.AboutUs    =GetDlgItemText(IDC_COMP_ABOUT                   );

    // Veritabanına Kaydet (Tekil Kayıt)
    if (m_db.SaveCompanyInfo(m_data))
    {
        MessageBox(_T("Firma bilgileri başarıyla güncellendi."), _T("Bilgi"), MB_OK | MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else
    {
        MessageBox(_T("Kayıt sırasında bir hata oluştu!"), _T("Hata"), MB_ICONERROR);
    }
}

void CCompanyDialog::OnBtnBrowseLogo()
{
    CFileDialog dlg(TRUE, _T("png"), NULL, OFN_FILEMUSTEXIST, _T("Resim Dosyaları (*.png;*.jpg)|*.png;*.jpg||"));
    if (dlg.DoModal(*this) == IDOK)
    {
        CString path = dlg.GetPathName();
        SetDlgItemText(IDC_COMP_LOGO_PATH, path);
    }
}

INT_PTR CCompanyDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COMMAND)
    {
        if (LOWORD(wParam) == IDC_COMP_BTN_LOGO)
        {
            OnBtnBrowseLogo();
            return TRUE;
        }
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}