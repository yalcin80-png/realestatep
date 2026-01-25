#pragma once
#include "resource.h"
#include "dataIsMe.h"

class CCompanyDialog : public CDialog
{
public:
    CCompanyDialog(DatabaseManager& db);
    virtual ~CCompanyDialog();

protected:
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void OnBtnBrowseLogo();

private:
    DatabaseManager& m_db;
    CompanyInfo_cstr m_data; // Formdaki veri
};