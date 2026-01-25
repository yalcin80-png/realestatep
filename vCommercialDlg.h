#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"


class CCommercialDialog : public CDialog
{
public:
    CCommercialDialog(DatabaseManager& db,
                      DialogMode mode,
                      const CString& cariKod,
                      const CString& commercialCodeToEdit = _T(""))
        : CDialog(IDD_COMMERCIAL_DIALOG),
          m_db(db),
          m_mode(mode),
          m_cariKod(cariKod),
          m_commCodeToEdit(commercialCodeToEdit) {}
    void SetCariKod(const CString& code) { m_cariKod = code; }
    void SetPropertyCode(const CString& code) { m_commCodeToEdit = code; }
    CString GetPropertyCode() const { return m_commCodeToEdit; }

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    //DatabaseManager&       m_db;
    DatabaseManager& m_db = DatabaseManager::GetInstance();; // DatabaseManager örneği

    DialogMode   m_mode;
    CString                m_cariKod;
    CString                m_commCodeToEdit;
    Commercial_cstr        m_data;

    // Controls
    CEdit     m_eCode, m_eAdres, m_eLokasyon, m_eAlan, m_eKat, m_eFiyat, m_eTeklifler, m_eDurum;
    CComboBox m_cbCephe, m_cbKullanim;

    // Helpers
    void     SetDataToControls(const Commercial_cstr& d);
    void     GetDialogData(Commercial_cstr& d);

    bool     SaveCommercial( Commercial_cstr& d);
};
