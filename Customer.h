#pragma once


#include "stdafx.h"
#include "resource.h"
#include "dataIsMe.h"
#include <fstream> // Dosya okuma için
#include <sstream> // String parçalama için (std::stringstream)




class CCustomerDialog : public CDialog
{
public:
     template <typename T> inline bool InsertGlobal(const T& data);

    CCustomerDialog(UINT nResID = IDD_CUSTOMER_DIALOG);
    CCustomerDialog(DatabaseManager& dbManagerRef, DialogMode mode, const CString& cariKodToEdit = _T(""));
    virtual ~CCustomerDialog() {}
    Customer_cstr m_customerData;
    CButton btnOK;
    DatabaseManager& dbManager = DatabaseManager::GetInstance();; // DatabaseManager örneği
    Customer_cstr GetCustomerData() const { return m_customer; }
    void SetCustomerData(const Customer_cstr& data) { m_customer = data; }
    void GetDialogData(Customer_cstr& data);
    void OnOK();
    void SetDataToControls(const Customer_cstr& data);

    void OnSetCtrl();

    /****************CSV ile ilgili işlemler*******************/
    void OnLoadCsvClicked();
    bool LoadDataFromCsv(const CString& filePath);

    /**************************************************************/
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

private:
    void LoadCustomerToControls();
    void SaveControlsToCustomer();
    CString m_cariKodToEdit;
    DialogMode m_dialogMode;
    // YENİ: Kaydetme sırasında InsertCustomer mı yoksa UpdateCustomer mı yapılacağını belirleyen metot
    bool SaveCustomerData(const Customer_cstr& data);
    CEdit    m_editCariKod;
    CEdit    m_editAdSoyad;
    CEdit    m_editTelefon;
    CEdit    m_editTelefon2;
    CEdit    m_editEmail;
    CEdit    m_editAdres;
    CEdit    m_editSehir;
    CEdit    m_editIlce;
    CEdit    m_editTCKN;
    CEdit    m_editVergiNo;
    CEdit    m_editKayitTarihi;
    CEdit    m_editNotlar;

    CComboBox m_comboMusteriTipi;
    CComboBox m_comboDurum;

    CComboBox cbType;
    CComboBox cbDurum;

    Customer_cstr m_customer;
};



// !CUSTOMER_H