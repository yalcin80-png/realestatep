#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"


class CVineyardDialog : public CDialog
{
public:
    CVineyardDialog(DatabaseManager& db, DialogMode mode,
                    const CString& cariKod, const CString& vineyardCodeToEdit = _T(""))
        : CDialog(IDD_VINEYARD_DIALOG),
          m_db(db),
          m_mode(mode),
          m_cariKod(cariKod),
          m_vineyardCodeToEdit(vineyardCodeToEdit) {}
    void SetCariKod(CString& code) { code  = m_cariKod; }
    void SetPropertyCode(const CString& code) { m_vineyardCodeToEdit = code; }
    CString GetPropertyCode() const { return m_vineyardCodeToEdit; }

    template <typename T> inline bool InsertGlobal(const T& data);

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    DatabaseManager&   m_db = DatabaseManager::GetInstance();
   

    DialogMode m_mode;
    CString            m_cariKod;
    CString            m_vineyardCodeToEdit;
    Vineyard_cstr      m_data;

    // Kontroller
    CEdit m_eCode, m_eAdres, m_eLokasyon, m_eM2, m_eAgac, m_eFiyat, m_eTeklifler, m_eDurum;
    CComboBox m_cbUrun, m_cbSulama;

    // YardÄ±mcÄ± fonksiyonlar
    void SetDataToControls(const Vineyard_cstr& d);
    void GetDialogData(Vineyard_cstr& d);
    bool SaveVineyard( Vineyard_cstr& d);
};