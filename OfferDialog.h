#pragma once
#ifndef OFFER_DIALOG_H
#define OFFER_DIALOG_H

#include "resource.h"
#include "dataIsMe.h"      // Offer_cstr ve Customer_cstr tanımları burada

class COfferDialog : public CDialog
{
public:
    // Kurucu: Veritabanı referansı ve Mülk Bilgisi (Örn: "Satılık Daire - İlan No: 123") alır
    COfferDialog(DatabaseManager& db, const CString& propertyInfo);
    virtual ~COfferDialog();

    // Diyalog kapandıktan sonra verileri almak için
    Offer_cstr GetOfferData() const { return m_offer; }
    Customer_cstr GetCustomerData() const { return m_customer; }

protected:
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual void OnOK() override; // Kaydet butonu

    // Yardımcılar
    void OnSelectCustomer(); // Müşteri Seç butonu işlemi
    void FillCombos();       // ComboBox'ları doldur

private:
    DatabaseManager& m_db;
    CString m_propertyInfo;

    // Veri Yapıları
    Offer_cstr m_offer;
    Customer_cstr m_customer;
};

#endif