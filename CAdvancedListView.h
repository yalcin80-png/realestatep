#pragma once
#ifndef CADVANCEDLISTVIEW_H
#define CADVANCEDLISTVIEW_H

#include "stdafx.h"
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

// ============================================================================
// 📊 DATA STRUCTURES
// ============================================================================

enum class CustomerGroup
{
    VIP = 0,
    Regular = 1,
    Pending = 2,
    Inactive = 3
};

struct CustomerData
{
    CString id;     // Cari Kod
    CString name;       // Ad Soyadı
    CString phone;               // Telefon
    CString email;     // E-Posta
    CString city;        // Şehir
    CString status;// Durum (Çalışıyor/Çalışmıyor)
    CustomerGroup group;      // Grup (VIP/Regular/Pending)
    DWORD createdDate;     // Kayıt Tarihi (timestamp)
    
    // Sıralama için yardımcı
    bool operator<(const CustomerData& other) const
  {
    return id < other.id;
    }
};

// ============================================================================
// 📋 DATA MANAGER (Virtual Mode için)
// ============================================================================

class CCustomerDataManager
{
public:
    CCustomerDataManager() : m_filterGroup((CustomerGroup)-1) {}
    
 void AddCustomer(const CustomerData& customer);
    void RemoveCustomer(const CString& id);
    void UpdateCustomer(const CString& id, const CustomerData& data);
    
    const CustomerData* GetCustomer(int index) const;
    int GetCustomerCount() const { return (int)m_customers.size(); }
    
    void SortBy(int columnIndex, bool ascending = true);
 void GroupByStatus();
    void FilterByGroup(CustomerGroup group);
    void ClearFilter() { m_filterGroup = (CustomerGroup)-1; }
    
    // Group yönetimi
    struct GroupInfo
    {
    CustomerGroup groupId;
   CString groupName;
   int itemCount;
        int startIndex;
    };
    
    const std::vector<GroupInfo>& GetGroups() const { return m_groups; }
    int GetGroupIndex(int itemIndex) const;
    CString GetGroupName(CustomerGroup group) const;

private:
    std::vector<CustomerData> m_customers;
    std::vector<GroupInfo> m_groups;
    CustomerGroup m_filterGroup = (CustomerGroup)-1;
    bool m_groupingEnabled = false;
    
    void RebuildGroups();
};

// ============================================================================
// 🔤 INDEX BAR (A-Z Navigasyon)
// ============================================================================

class CIndexBar : public CWnd
{
public:
    CIndexBar() : m_hotIndex(-1), m_selectedIndex(-1) {}
    
    void SetLetters(const CString& letters);
    int GetSelectedIndex() const { return m_selectedIndex; }

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CString m_letters;    // "ABCDEFGHIJKLMNOPQRSTUVWXYZ0-9"
    int m_hotIndex;              // Üzerine gelen harf indeksi
    int m_selectedIndex;         // Seçili harf indeksi
    
    LRESULT OnPaint();
    LRESULT OnLButtonDown(LPARAM lparam);
    LRESULT OnMouseMove(LPARAM lparam);
    LRESULT OnMouseLeave();
    
    int HitTest(POINT pt) const;
    void NotifyParent(int letterIndex);
};

// ============================================================================
// 📊 ADVANCED LIST VIEW (Virtual Mode + Grouping + Dual View)
// ============================================================================

class CAdvancedListView : public CListView
{
public:
    CAdvancedListView();
    virtual ~CAdvancedListView() override;
    
    // Data Management
    void SetDataManager(std::shared_ptr<CCustomerDataManager> pDataMgr);
    void Refresh();
    
    // View Mode
void SetViewMode(int mode); // VIEW_MODE_REPORT = 0, VIEW_MODE_CARD = 1
    int GetViewMode() const { return m_viewMode; }
    
    // Grouping
    void EnableGrouping(bool enable);
    void GroupByCustomerType();

    // Navigation
    void ScrollToLetter(TCHAR letter);
    
    // Accessors
    const CustomerData* GetSelectedCustomer() const;
    std::vector<const CustomerData*> GetSelectedCustomers() const;

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);
    
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);

private:
    std::shared_ptr<CCustomerDataManager> m_pDataMgr;
    int m_viewMode = 0; // 0 = REPORT, 1 = CARD
    bool m_groupingEnabled = false;
    int m_hoverIndex = -1;
    
    // Callback handlers
    LRESULT OnGetDispInfo(void* pDispInfo);  // LPNMLVDISPINFO yerine void*
    LRESULT OnCustomDraw(void* pLVCD);  // LPNMLVCUSTOMDRAW yerine void*
    
    // Diğer mesajlar
  LRESULT OnLvnOdFindItem(LPNMLVFINDITEM pFindItem);
    LRESULT OnNMDblClick(LPNMITEMACTIVATE pItemActivate);
    
    // Drawing helpers
    void DrawCardMode(LPNMLVCUSTOMDRAW pLVCD);
    void DrawReportMode(LPNMLVCUSTOMDRAW pLVCD);
    COLORREF GetGroupColor(CustomerGroup group) const;
    CString GetGroupIcon(CustomerGroup group) const;
    
    // Setup
    void SetupColumns();
    void SetupVirtualMode();
    void SetupGrouping();
};

// ============================================================================
// 🏗️ CONTAINER (Left ListView + Right IndexBar)
// ============================================================================

class CCustomerListContainer : public CWnd
{
public:
    CCustomerListContainer();
 virtual ~CCustomerListContainer() ;
    
    // Creation
    virtual HWND Create(HWND hParentWnd, UINT nId, 
    const RECT& rc = CRect(0, 0, 0, 0));
  
    // Accessors
    CAdvancedListView& GetListView() { return m_listView; }
    CIndexBar& GetIndexBar() { return m_indexBar; }
    std::shared_ptr<CCustomerDataManager> GetDataManager() { return m_pDataMgr; }
    
    // Layout
    void RecalcLayout();

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CAdvancedListView m_listView;
    CIndexBar m_indexBar;
    std::shared_ptr<CCustomerDataManager> m_pDataMgr;
    
    static const int INDEX_BAR_WIDTH = 25;
    
    LRESULT OnSize(UINT nType, int cx, int cy);
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    
    // Index Bar mesajları
    UINT OnIndexBarSelectLetter(WPARAM wParam, LPARAM lParam);
};

#endif // CADVANCEDLISTVIEW_H
