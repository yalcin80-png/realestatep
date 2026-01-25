#pragma once
#ifndef CUSTOMER_LIST_DLG_H
#define CUSTOMER_LIST_DLG_H

#include "resource.h"
#include "stdafx.h"
#include "dataIsMe.h"      

class CCustomerListDlg : public CDialog
{
public:
    CCustomerListDlg();
    virtual ~CCustomerListDlg();

    // Seçilen müşteriyi döndürür
    Customer_cstr GetSelectedCustomer() const { return m_selectedCustomer; }

protected:
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

    // Yardımcılar
    void SetupColumns();
    void LoadCustomers(const CString& filter = _T(""));
    void OnListViewDoubleClick();

private:
    CListView m_list; // ListView Kontrolü
    Customer_cstr m_selectedCustomer;
};

#endif