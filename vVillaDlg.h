#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"
#include "HomeFeaturesPage.h"

// 1. Temel Sayfa S�n�f�
class CVillaPageBase : public Win32xx::CDialog
{
public:
    CVillaPageBase(UINT id);
protected:
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;
    virtual BOOL OnEraseBkgnd(CDC& dc);
    virtual HBRUSH OnCtlColor(CDC& dc, HWND hwnd, UINT nCtlColor);
};

// 2. Tab Sayfalar� ��in Ara S�n�f
class CVillaPage : public CVillaPageBase
{
public:
    explicit CVillaPage(UINT idd = 0);
    void Create(Win32xx::CTab& tab);
};

// 3. Ana Villa Dialog S�n�f�
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

    // Sayfalar do�rudan ana diyalo�a ba�l�
    CVillaPage        m_pageGenel;
    CVillaPage        m_pageNotlar;
    CHomeFeaturesPage m_pageOzellik1; // Cephe / ��
    CHomeFeaturesPage m_pageOzellik2; // D�� / Muhit / Di�er

    void InitCombos();
    void OnSize(int width, int height);
    void RecalcLayout();
    void ShowPage(int page);
    void LoadFromDB();
    
    // Dinamik Oda Yönetimi
    std::vector<RoomInfo> m_rooms;
    void InitRoomControls();
    void LoadRoomsFromJson(const CString& jsonStr);
    CString SaveRoomsToJson();
    void RefreshRoomListView();
    void OnAddRoom();
    void OnRemoveRoom();
};