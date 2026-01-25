#pragma once
#ifndef FILES_H
#define FILES_H

// ============================================================================
//  📦 BASE (Win32++)
// ============================================================================
#include "wxx_wincore.h"
#include "wxx_controls.h"

// ============================================================================
//  📦 PROJECT
// ============================================================================
#include "InlineControls.h"
#include "AlphaFilterPanel.h"
#include "StatusFilterPanel.h"
#include "dataIsMe.h"
#include "vHomeDlg.h"
#include "Customer.h"
#include "CTreeListVDlg.h"  // CMyTreeListView (portfolio view)

// ============================================================================
//  📦 SYSTEM
// ============================================================================
#include <gdiplus.h>
#include <Uxtheme.h>

#include <vector>
#include <memory>
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "UxTheme.lib")

#define UWM_CONTEXT_CHANGED   (WM_APP + 101)
// wParam: 1=Customer, 2=Property

// ============================================================================
//  🔁 FILES VIEW NAVIGATION (ListView <-> TreeListView)
// ============================================================================
// We keep navigation strictly inside Files module. The ListView (customer cards)
// triggers a deferred message to its container (CContainFiles). The container
// then switches the visible child: customer list <-> customer portfolio (TreeList).
#define UWM_FILES_SHOW_PORTFOLIO   (WM_APP + 201) // lParam: CString* (Cari_Kod)
#define UWM_FILES_SHOW_CUSTOMERS   (WM_APP + 202) // no params

// Global: veri değişti (dialog/import vs.) -> ana view yenile
#define UWM_DATA_CHANGED        (WM_APP + 203) // wParam=0, lParam=0

// ListView -> StatusFilterPanel counts update
#define UWM_STATUS_COUNTS       (WM_APP + 204) // lParam = std::vector<std::pair<int,int>>*

// Local toolbar command for back navigation (doesn't need to exist in resources).
#ifndef IDM_FILES_BACK
  #define IDM_FILES_BACK  84001
#endif

// ============================================================================
//  🔢 GENERAL
// ============================================================================
#define VIEW_MODE_REPORT     0
#define VIEW_MODE_CARD       1
#ifndef IDM_COMPANY_INFO
  #define IDM_COMPANY_INFO     81726
#endif

// ============================================================================
//  📍 BAR POSITION
// ============================================================================
enum class BarPosition { Top = 0, Left = 1, Right = 2 };

// ============================================================================
//  🌟 CUSTOMER STATUS
// ============================================================================
#define STS_RANDEVU         5001
#define STS_TEKLIF          5002
#define STS_SICAK_ALICI     5003
#define STS_SICAK_SATICI    5004
#define STS_ISLEM_TAMAM     5005

#define STS_YENI            5010
#define STS_DUSUNUYOR       5011
#define STS_TAKIP           5012
#define STS_ULASILAMADI     5013

#define STS_PAHALI          5020
#define STS_OLUMSUZ         5021
#define STS_PASIF           5022

// ============================================================================
//  🪟 CUSTOMER INFO POPUP
// ============================================================================
class CCustomerInfoPopup : public CWnd
{
public:
    CCustomerInfoPopup() : m_visible(false) {}

    int m_statusID = 0;
    COLORREF m_rowColor = RGB(245, 248, 255);

    void ShowInfo(HWND parent,
        const std::vector<CString>& headers,
        const std::vector<CString>& values,
        const POINT& pt,
        COLORREF rowColor);

    void Hide();

protected:
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    bool m_visible;
    std::vector<CString> m_headers;
    std::vector<CString> m_values;
};

// ============================================================================
//  📋 CUSTOMER LIST VIEW
// ============================================================================
class CListCustomerView : public CListView
{
public:
    // --------------------------------------------------------------------
    //  Backward-compat message id
    // --------------------------------------------------------------------
    // Some legacy code paths (e.g., MainFrame routing) may still reference a
    // class-scoped message id. We keep it as an alias of the Files-module
    // navigation message so the project compiles while the new architecture
    // keeps navigation inside CContainFiles.
    static constexpr UINT UWM_OPEN_CUSTOMER_PROPERTIES = UWM_FILES_SHOW_PORTFOLIO;
    // --------------------------------------------------------------------
    //  Customer double-click (Navigation trigger)
    // --------------------------------------------------------------------
    // Double-click must NOT rebuild layouts or fill TreeListView directly.
    // We only send a deferred message to our container (CContainFiles).
    // The container owns the portfolio TreeListView and switches views.

    CListCustomerView() = default;
    virtual ~CListCustomerView() override;

    // ---- Core ----
    void OnInitialUpdate();
    BOOL OnCommand(WPARAM wparam, LPARAM lParam);
    void OnSize(UINT, int cx, int cy);

    // ---- Settings ----
    void SaveSettings();
    void LoadSettings();

    // ---- DPI / Focus ----
    LRESULT OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam);

    // ---- View Mode ----
    int  GetViewMode() const { return m_viewMode; }
    void SetViewMode(int mode) { ApplyViewMode(mode); }
    void ApplyViewMode(int mode);
    void OnSetFocus();

    // ---- List / DB ----
    void RefreshCustomerList();
    void RefreshList();
    void InsertItems();
    void AddCustomerData(const std::vector<Customer_cstr>& customers);
    void AddItemFromStruct(const Customer_cstr& c);
    void OnSaveListToDatabase();
    void SetDatabasePath(const CString& path);

    // ---- Filter ----
    void SetLetterFilter(const CString& letter);
    void SetStatusFilter(int statusID);
    void SetCategoryFilter(int catID);
    void SetFilter(const CString& filterLetter);

    int  GetCategoryFilter() const { return m_filterCategory; }

    // ---- CRUD ----
    void OnNewCustomer();
    void OnEditCustomer(const CString& cariKod);
    void DeleteSelectedItems();
    void DeleteAllSelections();

    // ---- Portfolio navigation (TreeListView fill) ----
    // Kept for compatibility with older call sites. New navigation should
    // route via UWM_FILES_SHOW_PORTFOLIO to CContainFiles.
    void OpenCustomerPropertiesInTreeList(const CString& cariKod);

    // ---- Property ----
    void OnAddHome(const CString& cariKod);
    void OnAddLand(const CString& cariKod);
    void OnAddField(const CString& cariKod);
    void OnAddVineYard(const CString& cariKod);
    void OnAddVilla(const CString& cariKod);
    void OnAddCommerical(const CString& cariKod);
    void OnAddCar(const CString& cariKod);

    // ---- Status / Category ----
    void UpdateCustomerStatus(const CString& cariKod, int commandID);
    void FilterByCategory(CustomerCategory category);
    void ApplyCustomerCategory(Customer_cstr& customer, const CString& notlar, const CString& calismaStatus);
    CustomerCategory GetCustomerCategory(const Customer_cstr& customer);

    // ---- Export / Import ----
    void ExportToExcelCSV(const CString& filePath);
    void ImportFromExcelCSV(const CString& filePath);

    // ---- Selection ----
    CString GetSelectedItemText(int nSubItem);
    Customer_cstr GetSelectedCustomerData();
    std::vector<CString> GetSelectedCariCodes();
    int GetSelectedSubItem() const;

    // ---- Inline ----
    CString GetColumnFieldName(int col);
    void OnInlineEditCommit(const CString& newText);
    void OnInlineComboCommit(const CString& newValue);
    void OnItemChanged(NMLISTVIEW* pNM);

    // ---- Mouse ----
    LRESULT HandleKeyDown(WPARAM wparam);
    LRESULT HandleLButtonDown(LPARAM lparam);
    LRESULT HandleLButtonUp(LPARAM lparam);
    LRESULT HandleLButtonDblClk(LPARAM lparam);
    // ---- Navigation (handled by CContainFiles) ----

    /* Duran YALÇIN 17/01/2026
    Fonksiyon Adı.:   CListCustomerView::OpenCustomerPropertiesInTreeList
    Açıklama :        (Legacy/compat) Seçili müşterinin portföyünü TreeListView
                     üzerinde açar. Yeni mimaride normal akış CContainFiles
                     üzerinden UWM_FILES_SHOW_PORTFOLIO ile yürür.
    */
   // void OpenCustomerPropertiesInTreeList(const CString& cariKod);

    LRESULT HandleRButtonDown(LPARAM lparam);
    LRESULT OnMouseMove(UINT, WPARAM, LPARAM);
    LRESULT OnMouseLeave(UINT, WPARAM, LPARAM);

    // ---- Popup ----
    VOID HandlePopupMenu(int x, int y);
    void HandleMouseHoverPopup(WPARAM wparam, LPARAM lparam);

    // ---- Card helpers ----
    CRect GetAddButtonRect(int nItem);
    bool IsPointInAddButton(POINT pt, int& outItem);
    void ShowQuickAddMenu(POINT pt, const CString& cariKod);
    void ShowRadialMenuAt(POINT pt, const CString& cariKod);



    // CListCustomerView.h içine:

// Report View Çizim Fonksiyonları
    void DrawReportItem(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawReportCell(Graphics& g, int item, int subItem, CRect rcCell, bool isSelected, int statusID);
    void DrawReportActionButtons(Graphics& g, CRect rcItem, bool isHovered, bool isSelected);

    // Yardımcılar
    CRect GetReportButtonRect(CRect rcItem, int btnIndex); // Tıklama kontrolü için
    bool IsPointInReportButton(POINT pt, int& outItem, int& outBtnIndex);

    void UpdateColumnWidths();



    void SetRowHeight(int height);

    void DrawReportItemMultiLine(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);

    // ---- Custom Draw ----
    LRESULT OnCustomDraw(NMCUSTOMDRAW* pNMCD);
    LRESULT ProcessCustomDraw(LPARAM lParam);
    void DrawSubItem(NMLVCUSTOMDRAW* pLVCD);

    LRESULT HandleReportItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD);
    LRESULT HandleReportSubItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD);

    void DrawCardItemUltra(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawCardItem(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawCardItemNormal(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawCardItemCompact(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawCardItemProfessional(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawHoverBar(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    CString m_filterLetter = _T("HEPSI");

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    LRESULT OnNotifyReflect(WPARAM wParam, LPARAM lParam);

private:
    // ---- Helpers ----
    void SetColumns();
    void AdjustCustomerColumnWidths();
    void SetDPIColumnWidths();
    void SetDPIImages();
    void LoadAvatarBitmaps();

private:
    // ---- Core Members ----
    int m_viewMode = VIEW_MODE_CARD;
    bool m_bTrackingMouse = false;

    DWORD m_lastHoverTick = 0;
    int m_hoverItem = -1;
    int m_nStatusColumn = 14;

    CString m_currentLetter;
    CString m_currentFilter;
    int     m_filterCategory = 0;
    int     m_currentStatus = 0;

    CCustomerInfoPopup m_infoPopup;
    CInlineEdit  m_editBox;
    CInlineCombo m_comboBox;

    int m_editItem = -1;
    int m_editSubItem = -1;

    COLORREF m_clrRowLight = RGB(250, 252, 255);
    COLORREF m_clrRowDark = RGB(240, 245, 250);
    COLORREF m_clrHover = RGB(220, 235, 255);
    COLORREF m_clrSelected = RGB(180, 210, 255);

    CImageList m_smallImages;
    // Keeps a persistent imagelist to control Report row height.
    // IMPORTANT: Do NOT create the imagelist on the stack and assign to ListView,
    // otherwise it will be destroyed and crash on exit.
    CImageList m_reportRowHeightIL;

    Gdiplus::Bitmap* m_maleBitmap = nullptr;
    Gdiplus::Bitmap* m_femaleBitmap = nullptr;

    DatabaseManager& db_Manager = DatabaseManager::GetInstance();

public:
    const int LISTVIEW_TOP_OFFSET_BASE = 10;
    const int CARD_TOTAL_WIDTH_BASE = 270;
    const int CARD_TOTAL_HEIGHT_BASE = 130;
    const int CARD_SPACING_BASE = 3;
    const int CARD_PADDING_BASE = 7;
    const int CARD_RADIUS_BASE = 350;
};

// ============================================================================
//  📦 CONTAINER
// ============================================================================
class CContainFiles : public CDockContainer
{
public:
    CContainFiles();
    virtual ~CContainFiles() override = default;

    void OnAttach();
    void RecalcLayout();
    BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    int     m_savedTopIndex = -1;   // ListView scroll position
    void SetupToolBar();

    // View helpers
    void SetViewMode(DWORD dwView);
    void ToggleViewMode();

    // Data helpers
    void AddFileItem(const CString& name, const CString& size, const CString& type, int imageIndex);
    void RemoveSelectedItem();
    void ClearAllItems();
    void RefreshFileList();

    // Selection
    CString GetSelectedFileName() const;
    bool HasSelection() const;

    // Filter
    void ApplyFilter(const CString& filterText);

    CListCustomerView& GetListView() { return m_viewFiles; }

    // Portfolio view (TreeList) helpers
    bool IsPortfolioVisible() const { return m_showPortfolio; }
    void ShowCustomersView();
    void ShowPortfolioView(const CString& cariKod);

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    LRESULT OnNotify(WPARAM wparam, LPARAM lparam);

private:
    enum { IDC_ALPHA_FILTER = 59701 };
    enum { IDC_STATUS_FILTER = 59702 };

    BarPosition m_barPos = BarPosition::Top;

    CToolBar m_ToolBar;
    CImageList m_ToolBarImages;

    CListCustomerView   m_viewFiles;
    CAlphaFilterBarTop  m_tabFilter;
    CStatusFilterBarTop m_statusFilter;

    // Embedded TreeListView: shows the selected customer's portfolio
    CMyTreeListView m_portfolioView;
    bool            m_showPortfolio = false;
    CString         m_activeCariKod;

    // --- Explorer-like navigation state (Customers view) ---
    // When navigating into Portfolio, remember selection/scroll so Back can
    // restore the exact spot in the customer list.
    //int             m_savedTopIndex = -1;  // ListView scroll position (LVM_GETTOPINDEX)
    int             m_savedSelIndex = -1;
    CString         m_savedCariKod;
};

// ============================================================================
//  🧱 DOCKER
// ============================================================================
class CDockFiles : public CDocker
{
public:
    CDockFiles();
    virtual ~CDockFiles() override = default;
    CContainFiles& GetContainer() { return (CContainFiles&)GetView(); }

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    CContainFiles m_files;
};

#endif // FILES_H
