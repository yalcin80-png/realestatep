#pragma once
#include "stdafx.h"
#include "resource.h"
#include "dataIsMe.h"

class CPropertySelectDlg : public CDialog
{
public:
    // Müşteri kodunu alarak başlatıyoruz
    CPropertySelectDlg(const CString& cariKod);

    // Seçilen verileri dışarıya vermek için
    CString GetSelectedPropertyID() const { return m_selectedID; }
    CString GetSelectedPropertyType() const { return m_selectedType; }
    CString GetSelectedPropertyInfo() const { return m_selectedInfo; }

protected:
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void RefreshList(); // Listeyi doldurur

private:
    CString m_cariKod;
    CString m_selectedID;
    CString m_selectedType;
    CString m_selectedInfo;

    CListView m_list;
    CComboBox m_comboType; // Ev, Arsa, Tarla filtresi
};