#include "stdafx.h"
#include "vCommercialDlg.h"
#include "dataIsMe.h"
#include "resource.h"

BOOL CCommercialDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Attach controls
    m_eCode     .AttachDlgItem(IDC_EDIT_COMM_CODE, *this);
    m_eAdres    .AttachDlgItem(IDC_EDIT_COMM_ADRES, *this);
    m_eLokasyon .AttachDlgItem(IDC_EDIT_COMM_LOKASYON, *this);
    m_eAlan     .AttachDlgItem(IDC_EDIT_COMM_ALAN, *this);
    m_eKat      .AttachDlgItem(IDC_EDIT_COMM_KAT, *this);
    m_cbCephe   .AttachDlgItem(IDC_COMBO_COMM_CEPHE, *this);
    m_cbKullanim.AttachDlgItem(IDC_COMBO_COMM_KULLANIM, *this);
    m_eFiyat    .AttachDlgItem(IDC_EDIT_COMM_FIYAT, *this);
    m_eTeklifler.AttachDlgItem(IDC_EDIT_COMM_TEKLIFLER, *this);
    m_eDurum    .AttachDlgItem(IDC_EDIT_COMM_DURUM, *this);

    // Combobox contents
    m_cbCephe.AddString(_T("Kuzey"));
    m_cbCephe.AddString(_T("GÃ¼ney"));
    m_cbCephe.AddString(_T("DoÄŸu"));
    m_cbCephe.AddString(_T("BatÄ±"));
    m_cbCephe.AddString(_T("KÃ¶ÅŸe"));

    m_cbKullanim.AddString(_T("MaÄŸaza"));
    m_cbKullanim.AddString(_T("Ofis"));
    m_cbKullanim.AddString(_T("Depo"));
    m_cbKullanim.AddString(_T("AtÃ¶lye"));
    m_cbKullanim.AddString(_T("DiÄŸer"));

    if (m_mode == DialogMode::IUPDATEUSER && !m_commCodeToEdit.IsEmpty())
    {
        m_data = m_db.GetCommercialByCode(m_commCodeToEdit);
        SetWindowText(_T("Ticari KayÄ±t GÃ¼ncelle"));
        SetDataToControls(m_data);
        m_eCode.SetReadOnly(TRUE);
    }
    else
    {
        SetWindowText(_T("Yeni Ticari KayÄ±t"));
        Commercial_cstr blank;
        blank.Cari_Kod       = m_cariKod;
        blank.Commercial_Code= m_db.GenerateNextCommercialCode();
        SetDataToControls(blank);
    }

    return TRUE;
}

void CCommercialDialog::SetDataToControls(const Commercial_cstr& d)
{
    m_eCode.SetWindowText(d.Commercial_Code);
    m_eAdres.SetWindowText(d.Adres);
    m_eLokasyon.SetWindowText(d.Lokasyon);
    m_eAlan.SetWindowText(d.Alan);
    m_eKat.SetWindowText(d.Kat);
    m_cbCephe.SelectString(-1, d.Cephe);
    m_cbKullanim.SelectString(-1, d.Kullanim);
    m_eFiyat.SetWindowText(d.Fiyat);
    m_eTeklifler.SetWindowText(d.Teklifler);
    m_eDurum.SetWindowText(d.Durum);
}

void CCommercialDialog::GetDialogData(Commercial_cstr& d)
{
    d.Cari_Kod        = m_cariKod;
    d.Commercial_Code = m_eCode.GetWindowText();
    d.Adres           = m_eAdres.GetWindowText();
    d.Lokasyon        = m_eLokasyon.GetWindowText();
    d.Alan            = m_eAlan.GetWindowText();
    d.Kat             = m_eKat.GetWindowText();

    CString s;
    s = m_cbCephe.GetWindowText();     d.Cephe    = s;
    s = m_cbKullanim.GetWindowText();  d.Kullanim = s;

    d.Fiyat           = m_eFiyat.GetWindowText();
    d.Teklifler       = m_eTeklifler.GetWindowText();
    d.Durum           = m_eDurum.GetWindowText();
}



bool CCommercialDialog::SaveCommercial( Commercial_cstr& d)
{
    if (m_mode == DialogMode::IUPDATEUSER)
        return m_db.UpdateGlobal(d);
    return m_db.InsertGlobal(d);
}

void CCommercialDialog::OnOK()
{
    Commercial_cstr d;
    GetDialogData(d);

    if (d.Commercial_Code.IsEmpty())
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t kodu boÅŸ olamaz."), _T("Hata"), MB_ICONERROR);
        return;
    }

    if (SaveCommercial(d))
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t baÅŸarÄ±yla kaydedildi."), _T("Bilgi"), MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t sÄ±rasÄ±nda hata oluÅŸtu."), _T("Hata"), MB_ICONERROR);
    }
}

INT_PTR CCommercialDialog::DialogProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DialogProcDefault(msg, wp, lp);
}
