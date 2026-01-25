#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"
#include "HomeFeaturesPage.h"

// 1. Temel Sayfa Sýnýfý
class CVillaPageBase : public Win32xx::CDialog
{
public:
    CVillaPageBase(UINT id);
protected:
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;
    virtual BOOL OnEraseBkgnd(CDC& dc);
    virtual HBRUSH OnCtlColor(CDC& dc, HWND hwnd, UINT nCtlColor);
};

// 2. Tab Sayfalarý Ýçin Ara Sýnýf
class CVillaPage : public CVillaPageBase
{
public:
    explicit CVillaPage(UINT idd = 0);
    void Create(Win32xx::CTab& tab);
};

// 3. Ana Villa Dialog Sýnýfý
class CVillaDialog : public CVillaPageBase
{
public:
    CVillaDialog(DatabaseManager& db, DialogMode mode, const CString& cariKod, const CString& villaCode = _T(""));

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;
    void OnOK() override;

private:
    DatabaseManager& m_db;
    DialogMode       m_mode;
    CString          m_cariKod;
    CString          m_villaCode;

    Win32xx::CFont m_font;
    Win32xx::CTab  m_tab;

    // Sayfalar doðrudan ana diyaloða baðlý
    CVillaPage        m_pageGenel;
    CVillaPage        m_pageNotlar;
    CHomeFeaturesPage m_pageOzellik1; // Cephe / Ýç
    CHomeFeaturesPage m_pageOzellik2; // Dýþ / Muhit / Diðer

    void InitCombos();
    void OnSize(int width, int height);
    void RecalcLayout();
    void ShowPage(int page);
    void LoadFromDB();
};