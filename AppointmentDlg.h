#pragma once
#include "resource.h"
#include "dataIsMe.h"
#include "GoogleServices.h"
#include "PropertySelectDlg.h"
class CAppointmentDlg : public CDialog
{
public:
    // Kurucu: Müşteri Kodu ve Emlak Kodu (Opsiyonel) alır
    CAppointmentDlg(const CString& custID, const CString& custName, 
                    const CString& propID = _T(""), const CString& propInfo = _T(""));
    virtual ~CAppointmentDlg() {}
    void OnSelectProperty();
    CString m_propType;
    BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    void SetEditMode(const CString& appID) { m_appID = appID; m_isEditMode = true; }
    void OnPrintToDlg();


protected:
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void OnPrint(); // Yazdırma fonksiyonu

private:
    bool m_isEditMode = false;
    CString m_appID; // Düzenlenecek Kaydın ID'si


    CString m_custID;
    CString m_custName;
    CString m_propID;
    CString m_propInfo;
    DatabaseManager& m_db = DatabaseManager::GetInstance();
};