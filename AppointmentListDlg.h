#pragma once
#include "resource.h"
#include "dataIsMe.h"

class CAppointmentListDlg : public CDialog
{
public:
    CAppointmentListDlg(const CString& custID, const CString& custName);

protected:
    virtual BOOL OnInitDialog() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void RefreshList();
    void OnDoubleClick(); // Detay açmak için

private:
    CString m_custID;
    CString m_custName;
    CListView m_list;
};