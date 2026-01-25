#include "stdafx.h"
#include "CAdvancedListView.h"
#include <cctype>
#include <commctrl.h>

// ============================================================================
// ?? CCustomerDataManager Implementation
// ============================================================================

void CCustomerDataManager::AddCustomer(const CustomerData& customer)
{
    m_customers.push_back(customer);
    RebuildGroups();
}

void CCustomerDataManager::RemoveCustomer(const CString& id)
{
    auto it = std::find_if(m_customers.begin(), m_customers.end(),
        [&id](const CustomerData& c) { return c.id == id; });
    
  if (it != m_customers.end())
    {
 m_customers.erase(it);
        RebuildGroups();
  }
}

void CCustomerDataManager::UpdateCustomer(const CString& id, const CustomerData& data)
{
    auto it = std::find_if(m_customers.begin(), m_customers.end(),
        [&id](const CustomerData& c) { return c.id == id; });
    
    if (it != m_customers.end())
    {
        *it = data;
 RebuildGroups();
    }
}

const CustomerData* CCustomerDataManager::GetCustomer(int index) const
{
    if (index < 0 || index >= (int)m_customers.size())
        return nullptr;
    return &m_customers[index];
}

void CCustomerDataManager::SortBy(int columnIndex, bool ascending)
{
  // Basit sýralama: columnIndex 0 = ID, 1 = Ad, vb.
    if (columnIndex == 0)
    {
        std::sort(m_customers.begin(), m_customers.end(),
            [ascending](const CustomerData& a, const CustomerData& b)
            {
  int cmp = a.id.Compare(b.id);
            return ascending ? (cmp < 0) : (cmp > 0);
      });
    }
    else if (columnIndex == 1)
    {
        std::sort(m_customers.begin(), m_customers.end(),
     [ascending](const CustomerData& a, const CustomerData& b)
          {
   int cmp = a.name.Compare(b.name);
     return ascending ? (cmp < 0) : (cmp > 0);
            });
  }
    RebuildGroups();
}

void CCustomerDataManager::GroupByStatus()
{
    // Durum'a göre grupla (VIP, Regular, Pending, Inactive)
    std::map<CustomerGroup, std::vector<CustomerData>> grouped;
  
    for (const auto& c : m_customers)
    {
 grouped[c.group].push_back(c);
    }
    
    m_customers.clear();
 m_groups.clear();
    
    int startIndex = 0;
 for (auto& [group, items] : grouped)
    {
        GroupInfo gi;
        gi.groupId = group;
     gi.groupName = GetGroupName(group);
        gi.itemCount = (int)items.size();
  gi.startIndex = startIndex;
    m_groups.push_back(gi);
        
        for (auto& item : items)
          m_customers.push_back(item);
        
        startIndex += gi.itemCount;
  }
}

void CCustomerDataManager::FilterByGroup(CustomerGroup group)
{
    m_filterGroup = group;
}

int CCustomerDataManager::GetGroupIndex(int itemIndex) const
{
    for (size_t i = 0; i < m_groups.size(); ++i)
    {
        if (itemIndex >= m_groups[i].startIndex &&
     itemIndex < m_groups[i].startIndex + m_groups[i].itemCount)
  return (int)i;
    }
    return -1;
}

CString CCustomerDataManager::GetGroupName(CustomerGroup group) const
{
  switch (group)
    {
    case CustomerGroup::VIP: return _T("VIP Müþteriler");
    case CustomerGroup::Regular: return _T("Standart Müþteriler");
    case CustomerGroup::Pending: return _T("Beklemede");
    case CustomerGroup::Inactive: return _T("Pasif");
    default: return _T("Diðer");
    }
}

void CCustomerDataManager::RebuildGroups()
{
    if (m_customers.empty())
    {
      m_groups.clear();
    return;
    }
    
    // Otomatik olarak gruplarý yeniden oluþtur (GroupByStatus çaðrýlýrsa)
}

// ============================================================================
// ?? CIndexBar Implementation
// ============================================================================

LRESULT CIndexBar::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_PAINT: return OnPaint();
    case WM_LBUTTONDOWN:     return OnLButtonDown(lparam);
    case WM_MOUSEMOVE:  return OnMouseMove(lparam);
    case WM_MOUSELEAVE:      return OnMouseLeave();
    }
    return WndProcDefault(msg, wparam, lparam);
}

LRESULT CIndexBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    return CWnd::WndProcDefault(msg, wparam, lparam);
}

void CIndexBar::SetLetters(const CString& letters)
{
    m_letters = letters;
    if (IsWindow()) Invalidate();
}

LRESULT CIndexBar::OnPaint()
{
    CPaintDC dc(*this);
    CRect rc = GetClientRect();
    
    // Arka plan
 dc.FillRect(rc, (HBRUSH)GetSysColorBrush(COLOR_BTNFACE));
    dc.DrawEdge(rc, BDR_SUNKENINNER, BF_RECT);
    
    if (m_letters.IsEmpty()) return 0;
    
    int letterCount = m_letters.GetLength();
    int itemH = rc.Height() / letterCount;
    if (itemH < 8) itemH = 8;
    
    for (int i = 0; i < letterCount; ++i)
    {
    CRect rItem(rc.left, rc.top + i * itemH, rc.right, rc.top + (i + 1) * itemH);
        
        // Seçili veya hover imi
        if (i == m_selectedIndex)
            dc.FillRect(rItem, (HBRUSH)GetSysColorBrush(COLOR_HIGHLIGHT));
        else if (i == m_hotIndex)
            dc.FillRect(rItem, (HBRUSH)GetSysColorBrush(COLOR_BTNHIGHLIGHT));
        
      // Metin
        dc.SetBkMode(TRANSPARENT);
        TCHAR ch = m_letters[i];
dc.DrawText(CString(ch), -1, rItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    
    return 0;
}

LRESULT CIndexBar::OnLButtonDown(LPARAM lparam)
{
    POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
    int idx = HitTest(pt);
    if (idx >= 0)
    {
   m_selectedIndex = idx;
        Invalidate();
        NotifyParent(idx);
    }
    return 0;
}

LRESULT CIndexBar::OnMouseMove(LPARAM lparam)
{
    TRACKMOUSEEVENT tme{};
 tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = GetHwnd();
    ::TrackMouseEvent(&tme);
    
    POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
    int idx = HitTest(pt);
    if (idx != m_hotIndex)
    {
        m_hotIndex = idx;
        Invalidate();
    }
    return 0;
}

LRESULT CIndexBar::OnMouseLeave()
{
    m_hotIndex = -1;
    Invalidate();
    return 0;
}

int CIndexBar::HitTest(POINT pt) const
{
    CRect rc = GetClientRect();
    if (m_letters.IsEmpty() || rc.IsRectEmpty()) return -1;
    
    int letterCount = m_letters.GetLength();
    int itemH = rc.Height() / letterCount;
    if (itemH < 1) itemH = 1;
    
    int idx = pt.y / itemH;
    return (idx >= 0 && idx < letterCount) ? idx : -1;
}

void CIndexBar::NotifyParent(int letterIndex)
{
    if (letterIndex >= 0 && letterIndex < m_letters.GetLength())
    {
        TCHAR letter = m_letters[letterIndex];
        // Parent'a UWM_LETTER_SELECTED mesajý gönder
        ::SendMessage(GetParent(), WM_APP + 100, (WPARAM)letter, 0);
    }
}

// ============================================================================
// ?? CAdvancedListView Implementation
// ============================================================================

CAdvancedListView::CAdvancedListView()
{
}

CAdvancedListView::~CAdvancedListView()
{
}

void CAdvancedListView::SetDataManager(std::shared_ptr<CCustomerDataManager> pDataMgr)
{
    m_pDataMgr = pDataMgr;
    SetupVirtualMode();
    Refresh();
}

void CAdvancedListView::Refresh()
{
    if (!m_pDataMgr) return;
 
    SetItemCount(m_pDataMgr->GetCustomerCount());
    Invalidate();
}

void CAdvancedListView::SetViewMode(int mode)
{
    if (m_viewMode == mode) return;
    
    m_viewMode = mode;
    DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
    
    if (mode == 0) // REPORT
    {
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);
     SetupColumns();
    }
    else // CARD mode (LVS_ICON)
    {
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_ICON);
    }
    
    Invalidate();
}

void CAdvancedListView::EnableGrouping(bool enable)
{
    m_groupingEnabled = enable;
    if (enable && m_pDataMgr)
    {
      m_pDataMgr->GroupByStatus();
        SetupGrouping();
    }
    Refresh();
}

void CAdvancedListView::GroupByCustomerType()
{
 if (m_pDataMgr)
    {
        m_pDataMgr->GroupByStatus();
        SetupGrouping();
     Refresh();
    }
}

void CAdvancedListView::ScrollToLetter(TCHAR letter)
{
    if (!m_pDataMgr) return;
    
    letter = (TCHAR)::toupper((unsigned char)letter);
    
    for (int i = 0; i < m_pDataMgr->GetCustomerCount(); ++i)
    {
  const auto* pCustomer = m_pDataMgr->GetCustomer(i);
        if (pCustomer && !pCustomer->name.IsEmpty())
        {
     TCHAR firstChar = (TCHAR)::toupper((unsigned char)pCustomer->name[0]);
   if (firstChar == letter)
            {
          EnsureVisible(i, FALSE);
   return;
 }
        }
 }
}

const CustomerData* CAdvancedListView::GetSelectedCustomer() const
{
    int idx = GetNextItem(-1, LVNI_SELECTED);
    if (idx >= 0 && m_pDataMgr)
        return m_pDataMgr->GetCustomer(idx);
    return nullptr;
}

std::vector<const CustomerData*> CAdvancedListView::GetSelectedCustomers() const
{
    std::vector<const CustomerData*> result;
    int idx = -1;
    
    while ((idx = GetNextItem(idx, LVNI_SELECTED)) >= 0)
    {
        if (m_pDataMgr)
    {
       const auto* pCustomer = m_pDataMgr->GetCustomer(idx);
          if (pCustomer) result.push_back(pCustomer);
        }
    }
    
    return result;
}

LRESULT CAdvancedListView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_COMMAND: return OnCommand(wparam, lparam);
    }
    return WndProcDefault(msg, wparam, lparam);
}

LRESULT CAdvancedListView::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    return CListView::WndProcDefault(msg, wparam, lparam);
}

BOOL CAdvancedListView::OnCommand(WPARAM wparam, LPARAM lparam)
{
    return FALSE;
}

LRESULT CAdvancedListView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR pNMHDR = (LPNMHDR)lparam;
    
    switch (pNMHDR->code)
    {
    case LVN_GETDISPINFO:
return OnGetDispInfo((NMLVDISPINFO*)lparam);
    case NM_CUSTOMDRAW:
        return OnCustomDraw((LPNMLVCUSTOMDRAW)lparam);
    }
  
    return CListView::OnNotifyReflect(wparam, lparam);
}

LRESULT CAdvancedListView::OnGetDispInfo(void* pDispInfo)
{
    LV_DISPINFO* pDispInfoEx = (LV_DISPINFO*)pDispInfo;
    if (!m_pDataMgr) return 0;
    
    int itemIdx = pDispInfoEx->item.iItem;
    int subIdx = pDispInfoEx->item.iSubItem;
    
    const auto* pCustomer = m_pDataMgr->GetCustomer(itemIdx);
    if (!pCustomer) return 0;
    
    if (pDispInfoEx->item.mask & LVIF_TEXT)
    {
 CString text;
     switch (subIdx)
   {
     case 0: text = pCustomer->id; break;
 case 1: text = pCustomer->name; break;
     case 2: text = pCustomer->phone; break;
   case 3: text = pCustomer->email; break;
        case 4: text = pCustomer->city; break;
      }
 
   if (pDispInfoEx->item.pszText && pDispInfoEx->item.cchTextMax > 0)
       ::wcscpy_s(pDispInfoEx->item.pszText, pDispInfoEx->item.cchTextMax, (LPCWSTR)(LPCTSTR)text);
    }
  
    return 0;
}

LRESULT CAdvancedListView::OnCustomDraw(void* pLVCD)
{
    LPNMLVCUSTOMDRAW pCustomDraw = (LPNMLVCUSTOMDRAW)pLVCD;
    if (m_viewMode == 0)
  return DrawReportMode(pCustomDraw), CDRF_DODEFAULT;
  else
     return DrawCardMode(pCustomDraw), CDRF_DODEFAULT;
}

void CAdvancedListView::DrawCardMode(LPNMLVCUSTOMDRAW pLVCD)
{
    // Kart modunda özel çizim (GDI+ veya basit GDI)
    // Þimdilik stub
}

void CAdvancedListView::DrawReportMode(LPNMLVCUSTOMDRAW pLVCD)
{
    // Report modunda satýr renklendirme
    if (!m_pDataMgr) return;
    
    int itemIdx = (int)pLVCD->nmcd.dwItemSpec;
    const auto* pCustomer = m_pDataMgr->GetCustomer(itemIdx);
    if (!pCustomer) return;
    
    // Grup rengini al
    pLVCD->clrTextBk = GetGroupColor(pCustomer->group);
    pLVCD->clrText = GetSysColor(COLOR_WINDOWTEXT);
}

COLORREF CAdvancedListView::GetGroupColor(CustomerGroup group) const
{
    switch (group)
    {
    case CustomerGroup::VIP: return RGB(255, 250, 200);   // Açýk sarý
    case CustomerGroup::Regular: return RGB(220, 255, 220);    // Açýk yeþil
    case CustomerGroup::Pending: return RGB(255, 240, 200);    // Açýk turuncu
    case CustomerGroup::Inactive: return RGB(235, 235, 235);   // Açýk gri
    default: return GetSysColor(COLOR_WINDOW);
    }
}

CString CAdvancedListView::GetGroupIcon(CustomerGroup group) const
{
    switch (group)
    {
case CustomerGroup::VIP: return _T("?");
    case CustomerGroup::Regular: return _T("•");
    case CustomerGroup::Pending: return _T("?");
    case CustomerGroup::Inactive: return _T("?");
    default: return _T("");
    }
}

void CAdvancedListView::SetupColumns()
{
    DeleteAllItems();
    
    LV_COLUMN lvc{};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;
 
    struct { const wchar_t* header; int width; } columns[] =
    {
        { L"Cari Kod", 100 },
        { L"Ad Soyadý", 200 },
        { L"Telefon", 120 },
    { L"E-Posta", 200 },
        { L"Þehir", 100 }
    };
    
    for (int i = 0; i < _countof(columns); ++i)
    {
        lvc.pszText = const_cast<wchar_t*>(columns[i].header);
lvc.cx = columns[i].width;
        InsertColumn(i, lvc);
    }
}

void CAdvancedListView::SetupVirtualMode()
{
    // Virtual mode (LVS_OWNERDATA) ayarla
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
    
    DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
    SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_REPORT | LVS_OWNERDATA);
  
    SetupColumns();
}

void CAdvancedListView::SetupGrouping()
{
    if (!m_groupingEnabled) return;
    
    // Gruplama Win32xx'te tam desteklenmiyor olabilir
  // Bu yapý sadece placeholder
}

// ============================================================================
// ??? CCustomerListContainer Implementation
// ============================================================================

CCustomerListContainer::CCustomerListContainer()
{
    m_pDataMgr = std::make_shared<CCustomerDataManager>();
}

CCustomerListContainer::~CCustomerListContainer()
{
}

HWND CCustomerListContainer::Create(HWND hParentWnd, UINT nId, const RECT& rc)
{
    // Container penceresini oluþtur
    HWND hWnd = CWnd::CreateEx(
 0,  // exStyle
   _T("STATIC"),  // className
  _T("CustomerListContainer"),  // windowName
        WS_CHILD | WS_VISIBLE,  // style
        rc,  // rect
        hParentWnd,  // parent
        nId,  // id
        nullptr  // lparam
    );
    
    if (!hWnd) return nullptr;
    
    // ListView'ý oluþtur
    CRect rcClient = GetClientRect();
    CRect rcList(0, 0, rcClient.Width() - INDEX_BAR_WIDTH, rcClient.Height());
m_listView.Create(hWnd);
    m_listView.MoveWindow(rcList);
    m_listView.SetDataManager(m_pDataMgr);
    
    // Index Bar'ý oluþtur
    CRect rcBar(rcClient.Width() - INDEX_BAR_WIDTH, 0, rcClient.Width(), rcClient.Height());
    m_indexBar.Create(hWnd);
  m_indexBar.MoveWindow(rcBar);
    m_indexBar.SetLetters(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    
  return hWnd;
}

void CCustomerListContainer::RecalcLayout()
{
    CRect rc = GetClientRect();
    
    if (::IsWindow(m_listView.GetHwnd()))
        m_listView.MoveWindow(CRect(0, 0, rc.Width() - INDEX_BAR_WIDTH, rc.Height()));
    
    if (::IsWindow(m_indexBar.GetHwnd()))
        m_indexBar.MoveWindow(CRect(rc.Width() - INDEX_BAR_WIDTH, 0, rc.Width(), rc.Height()));
}

LRESULT CCustomerListContainer::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_SIZE:
        return OnSize(HIWORD(lparam), LOWORD(lparam), HIWORD(lparam));
    case WM_CREATE:
      return OnCreate((LPCREATESTRUCT)lparam);
    case WM_APP + 100:
        return OnIndexBarSelectLetter(wparam, lparam);
  }
    return WndProcDefault(msg, wparam, lparam);
}

LRESULT CCustomerListContainer::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    return CWnd::WndProcDefault(msg, wparam, lparam);
}

LRESULT CCustomerListContainer::OnSize(UINT nType, int cx, int cy)
{
    RecalcLayout();
    return 0;
}

LRESULT CCustomerListContainer::OnCreate(LPCREATESTRUCT pcs)
{
    return 0;
}

UINT CCustomerListContainer::OnIndexBarSelectLetter(WPARAM wParam, LPARAM lParam)
{
    TCHAR letter = (TCHAR)wParam;
    m_listView.ScrollToLetter(letter);
    return 0;
}
