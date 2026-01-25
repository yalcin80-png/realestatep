#pragma once
#include "stdafx.h"
#include "dataIsMe.h" 
#include "resource.h" 
#include <map>
#include <vector>

#include <commctrl.h>
#include "HomeFeaturesPage.h"
#include "SimpleBrowser.h"

class CHomeDialog : public CDialog
{
public:
    // Yap�c�
    CHomeDialog(DatabaseManager& dbManagerRef, DialogMode mode, const CString& cariKod, const CString& homeCodeToEdit = _T(""));
    virtual ~CHomeDialog() override = default;

    // Temel Fonksiyonlar
    void SetCariKod(const CString& code) { m_cariKod = code; }
    void SetPropertyCode(const CString& code) { m_homeCodeToEdit = code; }

    // Panodan Veri �ekme (Text Parse)
    void OnLoadFromClipboard();
    void OnIlanBilgileriniAl();
    bool ValidateData(std::map<CString, CString>& dataMap);
    // Yard�mc�lar
    CString GetClipboardText();
    std::map<CString, CString> ParseSahibindenText(const CString& rawText);
    std::map<CString, CString> NormalizeToSchemaMap(const std::map<CString, CString>& rawFields);
    void SanitizeDataMap(std::map<CString, CString>& dataMap);
    // Scroll yard�mc�lar�
    void UpdateScrollInfo();
    int m_nVscrollPos = 0;
    int m_nHscrollPos = 0;
    int m_nVscrollMax = 0;
    int m_nHscrollMax = 0;
protected:
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void OnSetCtrl();     // ComboBox'lar� doldurur
    void FillCombo(int id, const std::vector<CString>& items);

private:
    DatabaseManager& m_dbManager;
    DialogMode m_dialogMode;
    CString m_cariKod;
    CString m_homeCodeToEdit;
private:
    HFONT m_hUiFont = nullptr;

    void ApplyFontRecursive(HWND hWnd, HFONT hFont);
    void FixTabFonts();
    // Seviye-2: Tab tabanl� �zellikler
    HWND m_hTab = nullptr;
    CHomeFeaturesPage m_featuresPage1;
    CHomeFeaturesPage m_featuresPage2;
    std::vector<HWND> m_generalControls; // Tab1'de g�sterilecek olanlar
    bool m_layoutShifted = false;

    void InitTabs();
    void CollectGeneralControls();
    void ShiftGeneralControlsForTabHeader();
    void LayoutTabAndPages();
    void SwitchTab(int index);
    
    // Browser for fetching single property data
    CSimpleBrowser m_browser;
    bool m_browserInitialized = false;
    CString m_pendingIlanNumarasi;
    
    void InitBrowserIfNeeded();
    void FetchPropertyData(const CString& ilanNumarasi);
    void OnPropertyDataFetched(const CString& url, bool success);
};