#include "stdafx.h"
#include "OfferDialog.h"
#include "CustomerListDlg.h"
#include <ctime>

COfferDialog::COfferDialog(DatabaseManager& db, const CString& propertyInfo)
    : CDialog(IDD_OFFER_DIALOG), m_db(db), m_propertyInfo(propertyInfo)
{
}

COfferDialog::~COfferDialog()
{
}

BOOL COfferDialog::OnInitDialog()
{
    // Temel sınıfı çağır
    CDialog::OnInitDialog();

    // 1. Mülk Bilgisini Yaz (Salt Okunur Alan)
    SetDlgItemText(IDC_OFFER_PROPERTY_INFO, m_propertyInfo);

    // 2. ComboBox'ları Doldur
    FillCombos();

    // 3. Tarih Varsayılanı (Bugün + 7 Gün geçerlilik)
    // Basit bir tarih formatı atıyoruz
    time_t now = time(0);
    tm ltm; localtime_s(&ltm, &now);
    CString dateStr;
    dateStr.Format(_T("%02d.%02d.%04d"), ltm.tm_mday, 1 + ltm.tm_mon, 1900 + ltm.tm_year);
    
    // Geçerlilik alanına varsayılan değer
    SetDlgItemText(IDC_OFFER_VALIDITY_DATE, _T("7 Gün")); 

    return TRUE;
}

void COfferDialog::FillCombos()
{
    // Para Birimi
    HWND hCurrency = GetDlgItem(IDC_OFFER_CURRENCY);
    SendMessage(hCurrency, CB_ADDSTRING, 0, (LPARAM)_T("TL"));
    SendMessage(hCurrency, CB_ADDSTRING, 0, (LPARAM)_T("USD"));
    SendMessage(hCurrency, CB_ADDSTRING, 0, (LPARAM)_T("EUR"));
    SendMessage(hCurrency, CB_SETCURSEL, 0, 0); // Varsayılan TL

    // Ödeme Şekli
    HWND hPay = GetDlgItem(IDC_OFFER_PAYMENT_TYPE);
    SendMessage(hPay, CB_ADDSTRING, 0, (LPARAM)_T("Nakit"));
    SendMessage(hPay, CB_ADDSTRING, 0, (LPARAM)_T("Konut Kredisi"));
    SendMessage(hPay, CB_ADDSTRING, 0, (LPARAM)_T("Takas + Nakit"));
    SendMessage(hPay, CB_SETCURSEL, 0, 0);

    // Durum
    HWND hStatus = GetDlgItem(IDC_OFFER_STATUS);
    SendMessage(hStatus, CB_ADDSTRING, 0, (LPARAM)_T("Beklemede"));
    SendMessage(hStatus, CB_ADDSTRING, 0, (LPARAM)_T("Kabul Edildi"));
    SendMessage(hStatus, CB_ADDSTRING, 0, (LPARAM)_T("Reddedildi"));
    SendMessage(hStatus, CB_ADDSTRING, 0, (LPARAM)_T("Geri Çekildi"));
    SendMessage(hStatus, CB_SETCURSEL, 0, 0); // Varsayılan Beklemede
}

BOOL COfferDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT id = LOWORD(wParam);

    switch (id)
    {
    case IDC_OFFER_BTN_SELECT_CUST:
        OnSelectCustomer();
        return TRUE;
    }

    return CDialog::OnCommand(wParam, lParam);
}

void COfferDialog::OnSelectCustomer()
{

    CCustomerListDlg dlg;

    if (dlg.DoModal(*this) == IDOK) // Kullanıcı seçip Tamam'a bastıysa
    {
        // Seçilen müşteriyi al
        m_customer = dlg.GetSelectedCustomer();

        // Formdaki alanları doldur (Readonly alanlar)
        SetDlgItemText(IDC_OFFER_CUST_NAME, m_customer.AdSoyad);
        SetDlgItemText(IDC_OFFER_CUST_TC, m_customer.TCKN);
        SetDlgItemText(IDC_OFFER_CUST_TEL, m_customer.Telefon);
    }
    ::EnableWindow(GetDlgItem(IDC_OFFER_CUST_NAME), TRUE);
    ::EnableWindow(GetDlgItem(IDC_OFFER_CUST_TC), TRUE);
    ::EnableWindow(GetDlgItem(IDC_OFFER_CUST_TEL), TRUE);
    ::SetFocus(GetDlgItem(IDC_OFFER_CUST_NAME));
}

void COfferDialog::OnOK()
{
    // 1. Arayüzden Verileri Topla

    // Müşteri Bilgileri
    m_customer.AdSoyad = GetDlgItemText(IDC_OFFER_CUST_NAME);
    m_customer.TCKN = GetDlgItemText(IDC_OFFER_CUST_TC);
    m_customer.Telefon = GetDlgItemText(IDC_OFFER_CUST_TEL);

    if (m_customer.AdSoyad.IsEmpty()) {
        MessageBox(_T("Lütfen bir müşteri seçin veya adını girin."), _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // Teklif Bilgileri (Edit Box'lardan okuma)
    m_offer.OfferPrice = GetDlgItemText(IDC_OFFER_PRICE);
    m_offer.DepositAmount = GetDlgItemText(IDC_OFFER_DEPOSIT);
    m_offer.ValidityDate = GetDlgItemText(IDC_OFFER_VALIDITY_DATE);
    m_offer.Notes = GetDlgItemText(IDC_OFFER_NOTES);

    // --- HATALI KISIM DÜZELTİLDİ ---
    // ComboBox ve Edit verilerini CString olarak doğrudan alıyoruz.
    // Buffer (TCHAR buf[100]) kullanmaya gerek yok.

    // 1. Para Birimi
    CString currency = GetDlgItemText(IDC_OFFER_CURRENCY);

    // Fiyatın yanına para birimini ekle (Örn: "5000000" -> "5000000 TL")
    if (!m_offer.OfferPrice.IsEmpty())
    {
        m_offer.OfferPrice += _T(" ") + currency;
    }

    // 2. Ödeme Şekli
    m_offer.PaymentMethod = GetDlgItemText(IDC_OFFER_PAYMENT_TYPE);

    // 3. Durum
    m_offer.Status = GetDlgItemText(IDC_OFFER_STATUS);

    // Tarih (Bugün) - Burası aynı kalıyor
    time_t now = time(0);
    tm ltm; localtime_s(&ltm, &now);
    CString dateStr;
    dateStr.Format(_T("%02d.%02d.%04d"), ltm.tm_mday, 1 + ltm.tm_mon, 1900 + ltm.tm_year);
    m_offer.OfferDate = dateStr;

    // Kapat ve IDOK döndür
    CDialog::OnOK();
}