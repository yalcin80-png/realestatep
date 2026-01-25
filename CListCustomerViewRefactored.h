//////////////////////////////////////////////////////////////////////
// CListCustomerViewRefactored.h
// 
// Profesyonel Müþteri ListView Sýnýfý (Win32++ Uyumlu)
// =====================================================
// Özellikler:
// - Dual View Mode (Report + Card Mode GDI+)
// - Virtual scrolling ready
// - Category filtering with Registry persistence
// - Alphabetic filtering (A-Z)
// - Inline editing (Edit + Combo)
// - Mouse hover info popup
// - Custom draw rendering
// - Thread-safe data operations
//
// Gereksinimler:
// - GDI+ (gdiplus.lib)
// - Win32++ v10.20+
// - DatabaseManager singleton
//
//////////////////////////////////////////////////////////////////////

#pragma once
#ifndef CLISTCUSTOMERVIEWREFACTORED_H
#define CLISTCUSTOMERVIEWREFACTORED_H

#include <vector>
#include <memory>
#include <algorithm>
#include <gdiplus.h>

using namespace Gdiplus;

// ====================================================================
// VIEW MODE CONSTANTS
// ====================================================================

enum class EViewMode
{
    Report = 0,  // LVS_REPORT
  Card = 1     // LVS_ICON + custom draw
};

// ====================================================================
// FILTER CONSTANTS
// ====================================================================

enum class EFilterCategory
{
    All = 0,
    HotBuyer = 1,
    HotSeller = 2,
    ActiveProspect = 3,
    PotentialBuyer = 4,
    Investor = 5,
    TenantCandidate = 6,
    RentalPropertyOwner = 7,
    NewCustomer = 8,
    UnderFollowUp = 9,
  HasAppointment = 10,
    Quoted = 11,
 TransactionComplete = 12,
    Inactive = 13
};

// ====================================================================
// HELPER STRUCTURES
// ====================================================================

/// <summary>
/// Inline edit/combo box parent interface
/// </summary>
class IInlineEditParent
{
public:
    virtual ~IInlineEditParent() = default;
    virtual void OnInlineEditCommit(const CString& newText) = 0;
    virtual void OnInlineComboCommit(const CString& newValue) = 0;
};

/// <summary>
/// Inline editable text box
/// </summary>
class CInlineEdit : public CEdit, public IInlineEditParent
{
public:
    CInlineEdit() : m_pParent(nullptr) {}
  
    void SetParent(IInlineEditParent* pParent) { m_pParent = pParent; }
    
protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    
private:
  IInlineEditParent* m_pParent;
};

/// <summary>
/// Inline combo box for status selection
/// </summary>
class CInlineCombo : public CComboBox, public IInlineEditParent
{
public:
    CInlineCombo() : m_pParent(nullptr) {}
    
    void SetParent(IInlineEditParent* pParent) { m_pParent = pParent; }
    
protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    
private:
    IInlineEditParent* m_pParent;
};

/// <summary>
/// Popup info window (Shift+Hover shows customer details)
/// </summary>
class CCustomerInfoPopup : public CWnd
{
public:
 CCustomerInfoPopup();
    virtual ~CCustomerInfoPopup() override = default;
    
    void ShowInfo(HWND parent, 
    const std::vector<CString>& headers,
    const std::vector<CString>& values,
            const POINT& pt,
         COLORREF rowColor);
    void Hide();
    
protected:
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;
    
private:
std::vector<CString> m_headers;
 std::vector<CString> m_values;
    COLORREF m_rowColor;
    bool m_visible;
};

// ====================================================================
// MAIN LISTVIEW CLASS
// ====================================================================

/// <summary>
/// Professional Customer ListView with dual view modes, filtering, and inline editing.
/// Thread-safe, high-performance implementation using Win32++ standards.
/// </summary>
class CListCustomerView : public CListView, public IInlineEditParent
{
public:
    // ================================================================
    // LIFECYCLE
    // ================================================================
    
    CListCustomerView();
    virtual ~CListCustomerView() override;
    
    CListCustomerView(const CListCustomerView&) = delete;
    CListCustomerView& operator=(const CListCustomerView&) = delete;
    
    // ================================================================
    // INITIALIZATION
    // ================================================================
    
    /// <summary>
    /// Called by framework when window is first created
    /// </summary>
    void OnInitialUpdate();
    
    /// <summary>
    /// Load UI state from registry
    /// </summary>
  void LoadSettings();
    
    /// <summary>
    /// Save UI state to registry
    /// </summary>
    void SaveSettings();
    
    // ================================================================
    // DATA MANAGEMENT
    // ================================================================
    
    /// <summary>
    /// Populate ListView with all customers from database
    /// </summary>
    void InsertItems();
    
    /// <summary>
    /// Add single customer to ListView
    /// </summary>
    void AddItemFromStruct(const Customer_cstr& c);
    
    /// <summary>
    /// Batch insert customers
    /// </summary>
    void AddCustomerData(const std::vector<Customer_cstr>& customers);
    
    /// <summary>
    /// Refresh entire list with current filters applied
 /// </summary>
    void RefreshCustomerList();
    
    /// <summary>
    /// Refresh with alphabetic filter
    /// </summary>
    void RefreshList();
    
    // ================================================================
    // FILTERING
    // ================================================================
    
    /// <summary>
    /// Apply category filter (ribbon selection)
    /// </summary>
    void SetCategoryFilter(int catID);
    
    /// <summary>
    /// Apply alphabetic filter (A-Z sidebar)
  /// </summary>
    void SetLetterFilter(const CString& letter);
    
  /// <summary>
    /// Apply alphabetic filter (public method)
    /// </summary>
    void SetFilter(const CString& filterLetter);
    
    // ================================================================
  // VIEW MODES
    // ================================================================
    
    /// <summary>
    /// Switch between Report and Card view modes
    /// </summary>
    void SetViewMode(EViewMode mode);
    
    EViewMode GetViewMode() const { return m_viewMode; }
    
    // ================================================================
    // DATA ACCESS
    // ================================================================
    
    /// <summary>
    /// Get currently selected customer
    /// </summary>
    Customer_cstr GetSelectedCustomerData();
    
    /// <summary>
    /// Get text of selected item's subitem
    /// </summary>
    CString GetSelectedItemText(int nSubItem);
    
    /// <summary>
    /// Get all selected customer codes
    /// </summary>
    std::vector<CString> GetSelectedCariCodes();
    
    // ================================================================
    // OPERATIONS
    // ================================================================
  
    void OnNewCustomer();
    void OnEditCustomer(const CString& cariKod);
    void UpdateCustomerStatus(const CString& cariKod, int commandID);
    
    void OnAddHome(const CString& cariKod);
    void OnAddLand(const CString& cariKod);
    void OnAddVineYard(const CString& cariKod);
    void OnAddCommerical(const CString& cariKod);
    void OnAddField(const CString& cariKod);
    void OnAddVilla(const CString& cariKod);
    
    void ExportToExcelCSV(const CString& filePath);
    void ImportFromExcelCSV(const CString& filePath);
    void OnSaveListToDatabase();
    
    // ================================================================
 // INTERNAL: DRAWING & RENDERING
    // ================================================================
    
protected:
    // Message handlers
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);
 
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;
    
    // Custom draw rendering
    LRESULT OnCustomDraw(NMCUSTOMDRAW* pNMCD);
    void DrawCardItemProfessional(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawHoverBar(LPNMLVCUSTOMDRAW pLVCD, CDC& dc);
    void DrawSubItem(NMLVCUSTOMDRAW* pLVCD);
 
    LRESULT HandleReportItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD);
    LRESULT HandleReportSubItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD);
    
    // Inline editing
    LRESULT OnGetDispInfo(LPNMLVDISPINFO pDispInfo) { return 0; }
    
    virtual void OnInlineEditCommit(const CString& newText) override;
    virtual void OnInlineComboCommit(const CString& newValue) override;
    
    // IInlineEditParent implementation
    void OnItemChanged(NMLISTVIEW* pNM);
    
private:
    // ================================================================
    // PRIVATE HELPERS
    // ================================================================
    
    void SetColumns();
    void SetupVirtualMode() {}
    void SetupGrouping() {}
    
    CString GetColumnFieldName(int subItem);
    int GetSelectedSubItem() const;
    
    void DeleteAllSelections();
    void HandlePopupMenu(int x, int y);
    void HandleMouseHoverPopup(WPARAM wparam, LPARAM lparam);
  
    LRESULT HandleKeyDown(WPARAM wparam);
    LRESULT HandleLButtonDown(LPARAM lparam);
    LRESULT HandleLButtonDblClk(LPARAM lparam);
    
    LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam);
    
    void SetDPIColumnWidths();
    void SetDPIImages();
    void AdjustCustomerColumnWidths();
    void OnSize(UINT nType, int cx, int cy);
    
    COLORREF GetGroupColor(CustomerGroup group) const;
    
    // ================================================================
    // MEMBER VARIABLES
    // ================================================================
    
    // View configuration
    EViewMode m_viewMode = EViewMode::Card;
    int m_filterCategory = 0;
    CString m_filterLetter = _T("HEPSI");
    CString m_currentFilter = _T("HEPSI");

    // Inline editing state
    CInlineEdit m_editBox;
    CInlineCombo m_comboBox;
    int m_editItem = -1;
 int m_editSubItem = -1;
    
    // Visual state
    int m_hoverItem = -1;
    int m_nStatusColumn = 14;
    
    // Colors
    COLORREF m_clrRowLight = RGB(250, 252, 255);
    COLORREF m_clrRowDark = RGB(240, 245, 250);
    COLORREF m_clrHover = RGB(220, 235, 255);
    COLORREF m_clrSelected = RGB(180, 210, 255);
    
    // Card mode constants
    const int LISTVIEW_TOP_OFFSET_BASE = 10;
    const int CARD_TOTAL_WIDTH_BASE = 270;
    const int CARD_TOTAL_HEIGHT_BASE = 130;
    const int CARD_SPACING_BASE = 3;
    const int CARD_PADDING_BASE = 7;
    const int CARD_RADIUS_BASE = 350;
  
    // Popup info window
    CCustomerInfoPopup m_infoPopup;
    DWORD m_lastHoverTick = 0;
    
    // Avatar images (GDI+)
    Bitmap* m_maleBitmap = nullptr;
    Bitmap* m_femaleBitmap = nullptr;
    
    // Database reference
    DatabaseManager& db_Manager = DatabaseManager::GetInstance();
    
  // Image list
    CImageList m_smallImages;
    
    // Helper for result tracking
    LRESULT result = 0;
};

#endif // CLISTCUSTOMERVIEWREFACTORED_H
