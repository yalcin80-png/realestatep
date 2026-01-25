#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"

// ============================================================================
//  vLandDlg.h - ARSA / ARAZİ PORTFÖY KARTI (Win32++)
//  - Home/Field ile uyumlu: DB kolonları GetProperties<Land_cstr> üzerinden gider
//  - Checkbox "Özellikler" ise tek kolon Attributes JSON içinde tutulur
// ============================================================================

class CLandDialog : public CDialog
{
public:
    CLandDialog(DatabaseManager& db, DialogMode mode,
        const CString& cariKod, const CString& landCodeToEdit = _T(""))
        : CDialog(IDD_LAND_DIALOG), m_db(db), m_mode(mode),
        m_cariKod(cariKod), m_landCodeToEdit(landCodeToEdit) {}

    virtual ~CLandDialog() override = default;

    void SetCariKod(const CString& code) { m_cariKod = code; }
    void SetPropertyCode(const CString& code) { m_landCodeToEdit = code; }
    CString GetPropertyCode() const { return m_landCodeToEdit; }

protected:
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;

private:
    DatabaseManager& m_db;
    DialogMode       m_mode;
    CString          m_cariKod;
    CString          m_landCodeToEdit;
    Land_cstr        m_data;

private:
    // Helpers
    CString GetText(int id) const;
    void    SetText(int id, const CString& text);

    void SetDataToControls(const Land_cstr& d);
    void GetDialogData(Land_cstr& d);

    void ApplyAttributesToCheckboxes(const Land_cstr& d);
    void ReadCheckboxesToAttributes(Land_cstr& d);

    void UpdatePricePerM2(Land_cstr& d);
    static double ToDoubleSafe(const CString& s);

    bool SaveLand(Land_cstr& d);
};
