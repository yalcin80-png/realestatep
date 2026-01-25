#include "stdafx.h"
#include "vVineyardDlg.h"
#include "dataIsMe.h"
#include "resource.h"

BOOL CVineyardDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Kontrolleri baÄŸla
    
    m_eCode.AttachDlgItem(IDC_EDIT_VINEYARD_CODE, *this);
    m_eAdres.AttachDlgItem(IDC_EDIT_VINEYARD_ADRES, *this);
    m_eLokasyon.AttachDlgItem(IDC_EDIT_VINEYARD_LOKASYON, *this);
    m_eM2.AttachDlgItem(IDC_EDIT_VINEYARD_M2, *this);
    m_eAgac.AttachDlgItem(IDC_EDIT_VINEYARD_AGAC, *this);
    m_cbUrun.AttachDlgItem(IDC_COMBO_VINEYARD_URUN, *this);
    m_cbSulama.AttachDlgItem(IDC_COMBO_VINEYARD_SULAMA, *this);
    m_eFiyat.AttachDlgItem(IDC_EDIT_VINEYARD_FIYAT, *this);
    m_eTeklifler.AttachDlgItem(IDC_EDIT_VINEYARD_TEKLIFLER, *this);
    m_eDurum.AttachDlgItem(IDC_EDIT_VINEYARD_DURUM, *this);

    // Combobox iÃ§erikleri
    m_cbUrun.AddString(_T("ÃœzÃ¼m"));
    m_cbUrun.AddString(_T("Zeytin"));
    m_cbUrun.AddString(_T("KayÄ±sÄ±"));
    m_cbUrun.AddString(_T("Elma"));
    m_cbUrun.AddString(_T("Karma"));

    m_cbSulama.AddString(_T("Var"));
    m_cbSulama.AddString(_T("Yok"));

    // Yeni mi, GÃ¼ncelleme mi?
    if (m_mode == DialogMode::IUPDATEUSER && !m_vineyardCodeToEdit.IsEmpty())
    {
        m_data = m_db.GetVineyardByCode(m_vineyardCodeToEdit);
        SetWindowText(_T("BaÄŸ / BahÃ§e KaydÄ± GÃ¼ncelle"));
        SetDataToControls(m_data);
        m_eCode.SetReadOnly(TRUE);
    }
    else
    {
        SetWindowText(_T("Yeni BaÄŸ / BahÃ§e KaydÄ±"));
        Vineyard_cstr blank;
        blank.Cari_Kod = m_cariKod;
        blank.Vineyard_Code = m_db.GenerateNextVineyardCode();
        SetDataToControls(blank);
    }

    return TRUE;
}

void CVineyardDialog::SetDataToControls(const Vineyard_cstr& d)
{
    m_cariKod = d.Cari_Kod;
    m_eCode.SetWindowText(d.Vineyard_Code);
    m_eAdres.SetWindowText(d.Adres);
    m_eLokasyon.SetWindowText(d.Lokasyon);
    m_eM2.SetWindowText(d.Metrekare);
    m_eAgac.SetWindowText(d.AgacSayisi);
    m_cbUrun.SelectString(-1, d.UrunTuru);
    m_cbSulama.SelectString(-1, d.Sulama);
    m_eFiyat.SetWindowText(d.Fiyat);
    m_eTeklifler.SetWindowText(d.Teklifler);
    m_eDurum.SetWindowText(d.Durum);
}

void CVineyardDialog::GetDialogData(Vineyard_cstr& d)
{
    d.Cari_Kod = m_cariKod.c_str();
    d.Vineyard_Code = m_eCode.GetWindowText();
    d.Adres = m_eAdres.GetWindowText();
    d.Lokasyon = m_eLokasyon.GetWindowText();
    d.Metrekare = m_eM2.GetWindowText();
    d.AgacSayisi = m_eAgac.GetWindowText();

    CString s;
    s = m_cbUrun.GetWindowText(); d.UrunTuru = s;
    s = m_cbSulama.GetWindowText(); d.Sulama = s;

    d.Fiyat = m_eFiyat.GetWindowText();
    d.Teklifler = m_eTeklifler.GetWindowText();
    d.Durum = m_eDurum.GetWindowText();
}



bool CVineyardDialog::SaveVineyard( Vineyard_cstr& d)
{
    if (m_mode == DialogMode::IUPDATEUSER)
        return m_db.UpdateGlobal(d);
    else
        return m_db.InsertGlobal(d);
}

void CVineyardDialog::OnOK()
{
    Vineyard_cstr d;
    GetDialogData(d);

    if (d.Vineyard_Code.IsEmpty())
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t kodu boÅŸ olamaz."), _T("Hata"), MB_ICONERROR);
        return;
    }

    if (SaveVineyard(d))
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t baÅŸarÄ±yla kaydedildi."), _T("Bilgi"), MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else
    {
        ::MessageBox(GetHwnd(), _T("KayÄ±t sÄ±rasÄ±nda hata oluÅŸtu."), _T("Hata"), MB_ICONERROR);
    }
}

INT_PTR CVineyardDialog::DialogProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DialogProcDefault(msg, wp, lp);
}
