#include "stdafx.h"
#include "vLandDlg.h"
#include "dataIsMe.h"
#include "resource.h"
#include <vector>

namespace {
    struct AttrChk { int id; const char* key; };

    const AttrChk kLandAttrChecks[] = {
        // Altyapı
        { IDC_LAND_CHK_ELEKTRIK,        "Infra_Electric" },
        { IDC_LAND_CHK_SANAYI_ELEK,     "Infra_IndustrialElectric" },
        { IDC_LAND_CHK_SU,              "Infra_Water" },
        { IDC_LAND_CHK_DOGALGAZ,        "Infra_NaturalGas" },
        { IDC_LAND_CHK_KANAL,           "Infra_Sewerage" },
        { IDC_LAND_CHK_ARITMA,          "Infra_Treatment" },
        { IDC_LAND_CHK_ZEMIN,           "Infra_GroundSurvey" },
        { IDC_LAND_CHK_YOL_ACILMIS,     "Infra_RoadOpened" },
        { IDC_LAND_CHK_YOL_ACILMAMIS,   "Infra_RoadNotOpened" },
        { IDC_LAND_CHK_YOL_YOK,         "Infra_NoRoad" },
        { IDC_LAND_CHK_TELEFON,         "Infra_Phone" },
        { IDC_LAND_CHK_SONDAJ,          "Infra_Well" },

        // Konum
        { IDC_LAND_CHK_ANA_YOL,         "Loc_NearMainRoad" },
        { IDC_LAND_CHK_TOPLU_ULASIM,    "Loc_NearPublicTransport" },
        { IDC_LAND_CHK_DENIZE_SIFIR,    "Loc_Seaside" },
        { IDC_LAND_CHK_DENIZE_YAKIN,    "Loc_NearSea" },
        { IDC_LAND_CHK_HAVAALANI,       "Loc_NearAirport" },

        // Genel
        { IDC_LAND_CHK_IFRAZLI,         "Gen_Subdivided" },
        { IDC_LAND_CHK_PARSELLI,        "Gen_Parcelled" },
        { IDC_LAND_CHK_PROJELI,         "Gen_Project" },
        { IDC_LAND_CHK_KOSE,            "Gen_CornerParcel" },

        // Manzara
        { IDC_LAND_CHK_SEHIR,           "View_City" },
        { IDC_LAND_CHK_GOL,             "View_Lake" },
        { IDC_LAND_CHK_DENIZ,           "View_Sea" },
        { IDC_LAND_CHK_DOGA,            "View_Nature" },
        { IDC_LAND_CHK_BOGAZ,           "View_Bosphorus" },
    };
}

CString CLandDialog::GetText(int id) const
{
    CString s;
    ::GetDlgItemText(*this, id, s.GetBuffer(4096), 4096);
    s.ReleaseBuffer();
    s.Trim();
    return s;
}

void CLandDialog::SetText(int id, const CString& text)
{
    ::SetDlgItemText(*this, id, text);
}

double CLandDialog::ToDoubleSafe(const CString& s)
{
    CString t = s;
    t.Trim();
    t.Replace(_T("."), _T(",")); // TR kullanıcı girdisi için tolerans
    wchar_t* end = nullptr;
    double v = wcstod(t, &end);
    if (!end) return 0.0;
    return v;
}

BOOL CLandDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Kimlik alanları
    SetText(IDC_LAND_CARI_KOD, m_cariKod);

    // Yeni / Güncelle
    if (m_mode == IUPDATEUSER && !m_landCodeToEdit.IsEmpty())
    {
        m_data = m_db.GetLandByCode(m_landCodeToEdit);
        SetWindowText(_T("Arsa Kaydını Güncelle"));
        SetDataToControls(m_data);
    }
    else
    {
        SetWindowText(_T("Yeni Arsa Kaydı"));

        Land_cstr blank;
        blank.Cari_Kod = m_cariKod;
        blank.Land_Code = m_db.GenerateNextLandCode();
        blank.Currency = _T("TL");
        blank.Status = _T("Aktif");
        SetDataToControls(blank);
    }

    // Code ve Cari kod read-only
    ::SendMessage(GetDlgItem(IDC_EDIT_LAND_CODE), EM_SETREADONLY, TRUE, 0);
    ::SendMessage(GetDlgItem(IDC_LAND_CARI_KOD), EM_SETREADONLY, TRUE, 0);

    return TRUE;
}

void CLandDialog::SetDataToControls(const Land_cstr& d)
{
    // Kimlik
    SetText(IDC_LAND_CARI_KOD, d.Cari_Kod);
    SetText(IDC_EDIT_LAND_CODE, d.Land_Code);

    // İlan
    SetText(IDC_LAND_LISTING_NO, d.ListingNo);
    SetText(IDC_LAND_LISTING_DATE, d.ListingDate);
    SetText(IDC_LAND_PROPERTY_TYPE, d.PropertyType);
    SetText(IDC_LAND_SELLER_TYPE, d.SellerType);
    SetText(IDC_LAND_CREDIT_ELIGIBLE, d.CreditEligible);
    SetText(IDC_LAND_SWAP, d.Swap);

    // Konum / Tapu
    SetText(IDC_EDIT_LAND_ADRES, d.Adres);
    SetText(IDC_EDIT_LAND_LOKASYON, d.Lokasyon);
    SetText(IDC_EDIT_LAND_ADA, d.Ada);
    SetText(IDC_EDIT_LAND_PARSEL, d.Parsel);
    SetText(IDC_EDIT_LAND_PAFTA, d.Pafta);
    SetText(IDC_LAND_DEED_STATUS, d.DeedStatus);

    // İmar / ölçü
    SetText(IDC_EDIT_LAND_IMAR, d.ImarDurumu);
    SetText(IDC_LAND_KAKS, d.Kaks);
    SetText(IDC_LAND_GABARI, d.Gabari);
    SetText(IDC_EDIT_LAND_M2, d.Metrekare);

    // Finans
    SetText(IDC_EDIT_LAND_FIYAT, d.Fiyat);
    SetText(IDC_LAND_CURRENCY, d.Currency);
    SetText(IDC_LAND_PRICE_PER_M2, d.PricePerM2);

    // Yetki
    SetText(IDC_LAND_AUTH_START, d.YetkiBaslangic);
    SetText(IDC_LAND_AUTH_END, d.YetkiBitis);

    // Notlar
    SetText(IDC_EDIT_LAND_TEKLIF, d.Teklifler);
    SetText(IDC_EDIT_LAND_DURUM, d.Durum);

    // Checkbox -> Attributes
    ApplyAttributesToCheckboxes(d);
}

void CLandDialog::ApplyAttributesToCheckboxes(const Land_cstr& d)
{
    for (const auto& it : kLandAttrChecks)
    {
        const bool on = d.GetAttrBool(it.key);
        ::CheckDlgButton(*this, it.id, on ? BST_CHECKED : BST_UNCHECKED);
    }
}

void CLandDialog::ReadCheckboxesToAttributes(Land_cstr& d)
{
    for (const auto& it : kLandAttrChecks)
    {
        const bool on = (::IsDlgButtonChecked(*this, it.id) == BST_CHECKED);
        d.SetAttrBool(it.key, on);
    }
}

void CLandDialog::UpdatePricePerM2(Land_cstr& d)
{
    const double price = ToDoubleSafe(d.Fiyat);
    const double m2 = ToDoubleSafe(d.Metrekare);
    if (price > 0.0 && m2 > 0.0)
    {
        double ppm2 = price / m2;
        CString out;
        out.Format(_T("%.2f"), ppm2);
        d.PricePerM2 = out;
    }
    else
    {
        d.PricePerM2 = _T("");
    }
}

void CLandDialog::GetDialogData(Land_cstr& d)
{
    // Kimlik
    d.Cari_Kod = m_cariKod;
    d.Land_Code = GetText(IDC_EDIT_LAND_CODE);

    // İlan
    d.ListingNo = GetText(IDC_LAND_LISTING_NO);
    d.ListingDate = GetText(IDC_LAND_LISTING_DATE);
    d.PropertyType = GetText(IDC_LAND_PROPERTY_TYPE);
    d.SellerType = GetText(IDC_LAND_SELLER_TYPE);
    d.CreditEligible = GetText(IDC_LAND_CREDIT_ELIGIBLE);
    d.Swap = GetText(IDC_LAND_SWAP);

    // Konum / Tapu
    d.Adres = GetText(IDC_EDIT_LAND_ADRES);
    d.Lokasyon = GetText(IDC_EDIT_LAND_LOKASYON);
    d.Ada = GetText(IDC_EDIT_LAND_ADA);
    d.Parsel = GetText(IDC_EDIT_LAND_PARSEL);
    d.Pafta = GetText(IDC_EDIT_LAND_PAFTA);
    d.DeedStatus = GetText(IDC_LAND_DEED_STATUS);

    // İmar / ölçü
    d.ImarDurumu = GetText(IDC_EDIT_LAND_IMAR);
    d.Kaks = GetText(IDC_LAND_KAKS);
    d.Gabari = GetText(IDC_LAND_GABARI);
    d.Metrekare = GetText(IDC_EDIT_LAND_M2);

    // Finans
    d.Fiyat = GetText(IDC_EDIT_LAND_FIYAT);
    d.Currency = GetText(IDC_LAND_CURRENCY);

    // Yetki
    d.YetkiBaslangic = GetText(IDC_LAND_AUTH_START);
    d.YetkiBitis = GetText(IDC_LAND_AUTH_END);

    // Notlar
    d.Teklifler = GetText(IDC_EDIT_LAND_TEKLIF);
    d.Durum = GetText(IDC_EDIT_LAND_DURUM);

    // Attributes (checkbox)
    ReadCheckboxesToAttributes(d);

    // Hesaplanan alan
    UpdatePricePerM2(d);

    // Sistem
    d.Updated_At = m_db.GetCurrentIsoUtc();
}

bool CLandDialog::SaveLand(Land_cstr& d)
{
    if (m_mode == DialogMode::IUPDATEUSER)
        return m_db.UpdateGlobal(d);
    return m_db.InsertGlobal(d);
}

void CLandDialog::OnOK()
{
    Land_cstr d;
    GetDialogData(d);

    if (d.Land_Code.IsEmpty()) { ::MessageBox(GetHwnd(), _T("Arsa Kodu boş olamaz."), _T("Hata"), MB_ICONERROR); return; }
    if (d.Adres.IsEmpty())     { ::MessageBox(GetHwnd(), _T("Adres boş olamaz."), _T("Hata"), MB_ICONERROR); return; }

    if (SaveLand(d))
    {
        ::MessageBox(GetHwnd(), _T("Kayıt başarıyla kaydedildi."), _T("Başarılı"), MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else
    {
        ::MessageBox(GetHwnd(), _T("Kayıt sırasında hata oluştu."), _T("Hata"), MB_ICONERROR);
    }
}

INT_PTR CLandDialog::DialogProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DialogProcDefault(msg, wp, lp);
}
