//////////////////////////////////////////////////////////////////////
// CListCustomerViewRefactored.cpp
//
// Profesyonel Müþteri ListView Implementasyonu (Win32++ Uyumlu)
// ==============================================================
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CListCustomerViewRefactored.h"
#include "Resource.h"
#include <locale.h>

#pragma comment(lib, "gdiplus.lib")

// ====================================================================
// INLINE EDIT - IMPLEMENTATION
// ====================================================================

LRESULT CInlineEdit::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_KEYDOWN)
    {
        if (wparam == VK_RETURN)
   {
            CString text = GetWindowText();
         if (m_pParent)
     m_pParent->OnInlineEditCommit(text);
 Destroy();
        return 0;
        }
     if (wparam == VK_ESCAPE)
        {
            Destroy();
  return 0;
        }
    }
    return CEdit::WndProc(msg, wparam, lparam);
}

// ====================================================================
// INLINE COMBO - IMPLEMENTATION
// ====================================================================

LRESULT CInlineCombo::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_KEYDOWN)
    {
        if (wparam == VK_RETURN)
        {
            int sel = GetCurSel();
  CString val;
     val.Format(_T("%d"), sel);
            if (m_pParent)
  m_pParent->OnInlineComboCommit(val);
     Destroy();
         return 0;
      }
        if (wparam == VK_ESCAPE)
        {
        Destroy();
         return 0;
        }
    }
    return CComboBox::WndProc(msg, wparam, lparam);
}

// ====================================================================
// CUSTOMER INFO POPUP - IMPLEMENTATION
// ====================================================================

CCustomerInfoPopup::CCustomerInfoPopup()
    : m_rowColor(RGB(245, 248, 255)), m_visible(false)
{
}

void CCustomerInfoPopup::ShowInfo(HWND parent,
const std::vector<CString>& headers,
    const std::vector<CString>& values,
 const POINT& pt,
    COLORREF rowColor)
{
    if (!::IsWindow(parent))
        return;

    m_headers = headers;
    m_values = values;
    m_rowColor = rowColor;

    if (!GetHwnd())
    {
        CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            _T("STATIC"),
  _T("InfoPopup"),
    WS_POPUP | WS_BORDER,
          0, 0, 0, 0,
            parent,
     0U
    );
    }

    const int height = 25 + (int)m_headers.size() * 22;
    const int width = 280;

    SetWindowPos(HWND_TOPMOST, pt.x + 15, pt.y + 15, width, height,
  SWP_NOACTIVATE | SWP_SHOWWINDOW);

 m_visible = true;
    RedrawWindow();
}

void CCustomerInfoPopup::Hide()
{
    if (m_visible)
{
        ShowWindow(SW_HIDE);
        m_visible = false;
    }
}

LRESULT CCustomerInfoPopup::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_PAINT)
    {
        PAINTSTRUCT ps{};
   HDC hdc = BeginPaint(ps);
        
        if (hdc)
        {
        Graphics g(hdc);
            g.SetSmoothingMode(SmoothingModeAntiAlias);

            Rect r(0, 0, ps.rcPaint.right - ps.rcPaint.left, 
   ps.rcPaint.bottom - ps.rcPaint.top);

      // Arka plan: gradyan
Color baseColor(GetRValue(m_rowColor), GetGValue(m_rowColor), GetBValue(m_rowColor));
   Color topColor(
         (BYTE)std::min(GetRValue(m_rowColor) + 15, 255),
      (BYTE)std::min(GetGValue(m_rowColor) + 15, 255),
       (BYTE)std::min(GetBValue(m_rowColor) + 15, 255)
            );

            LinearGradientBrush bg(r, topColor, baseColor, LinearGradientModeVertical);
g.FillRectangle(&bg, r);

     // Kenarlýk
       Pen borderPen(Color(160, 160, 160), 1.0f);
   g.DrawRectangle(&borderPen, r);

 // Baþlýk ve deðerleri çiz
       Gdiplus::Font font(L"Segoe UI", 9);
    SolidBrush textBrush(Color(25, 25, 25));
      Pen linePen(Color(200, 200, 200), 1);

   int y = 10;
 const int rowHeight = 22;
      const int headerWidth = 100;

            for (size_t i = 0; i < m_headers.size() && i < m_values.size(); ++i)
            {
    if (i % 2 == 1)
    {
    SolidBrush altBrush(Color(20, 255, 255, 255));
          g.FillRectangle(&altBrush, 5, y, r.Width - 10, rowHeight);
      }

   CString header = m_headers[i] + _T(":");
                CString value = m_values[i];

                RectF rectHeader(10, (REAL)y, (REAL)headerWidth, (REAL)rowHeight);
       RectF rectValue(15 + headerWidth, (REAL)y, 
(REAL)(r.Width - headerWidth - 25), (REAL)rowHeight);

       g.DrawString(header, -1, &font, rectHeader, NULL, &textBrush);
    g.DrawString(value, -1, &font, rectValue, NULL, &textBrush);

       if (i < m_headers.size() - 1)
   g.DrawLine(&linePen, 8, y + rowHeight - 1, 
   r.Width - 8, y + rowHeight - 1);

          y += rowHeight;
            }

            EndPaint(ps);
     }
   return 0;
    }

    return CWnd::WndProcDefault(msg, wparam, lparam);
}

// ====================================================================
// MAIN LISTCUSTOMERVIEW - IMPLEMENTATION
// ====================================================================

CListCustomerView::CListCustomerView()
    : m_viewMode(EViewMode::Card),
 m_filterCategory(0),
      m_filterLetter(_T("HEPSI")),
      m_currentFilter(_T("HEPSI")),
   m_editItem(-1),
      m_editSubItem(-1),
      m_hoverItem(-1),
      m_nStatusColumn(14),
      m_clrRowLight(RGB(250, 252, 255)),
      m_clrRowDark(RGB(240, 245, 250)),
      m_clrHover(RGB(220, 235, 255)),
      m_clrSelected(RGB(180, 210, 255))
{
}

CListCustomerView::~CListCustomerView()
{
    SaveSettings();
    if (IsWindow())
        DeleteAllItems();
}

void CListCustomerView::OnInitialUpdate()
{
    LoadSettings();

    m_viewMode = EViewMode::Card;
    m_currentFilter = _T("HEPSI");

    // Setup list view style
    DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
    SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_ICON);

    // Setup icon spacing for card mode
    const int CARD_TOTAL_WIDTH = DpiScaleInt(CARD_TOTAL_WIDTH_BASE);
    const int CARD_TOTAL_HEIGHT = DpiScaleInt(CARD_TOTAL_HEIGHT_BASE);
    
    ListView_SetIconSpacing(GetHwnd(), CARD_TOTAL_WIDTH, CARD_TOTAL_HEIGHT - 3);

    // Setup extended styles
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_DOUBLEBUFFER);

    SetColumns();
    InsertItems();
}

void CListCustomerView::LoadSettings()
{
  HKEY hKey = nullptr;
    LONG lRes = ::RegOpenKeyEx(HKEY_CURRENT_USER, 
       _T("Software\\EmlakCRM\\ListViewSettings"), 0, KEY_READ, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        DWORD dwVal = 0;
        DWORD cbData = sizeof(DWORD);
      
        if (::RegQueryValueEx(hKey, _T("LastCategory"), nullptr, nullptr,
     (LPBYTE)&dwVal, &cbData) == ERROR_SUCCESS)
        {
            m_filterCategory = (int)dwVal;
     }

        TCHAR szBuffer[64];
      DWORD cbString = sizeof(szBuffer);
      
    if (::RegQueryValueEx(hKey, _T("LastLetter"), nullptr, nullptr,
            (LPBYTE)szBuffer, &cbString) == ERROR_SUCCESS)
        {
            m_filterLetter = szBuffer;
     }

        ::RegCloseKey(hKey);
    }
    else
    {
        m_filterLetter = _T("HEPSI");
        m_filterCategory = 0;
    }
}

void CListCustomerView::SaveSettings()
{
    HKEY hKey = nullptr;
    DWORD dwDisposition = 0;

    LONG lRes = ::RegCreateKeyEx(HKEY_CURRENT_USER,
        _T("Software\\EmlakCRM\\ListViewSettings"),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, &dwDisposition);

    if (lRes == ERROR_SUCCESS)
    {
      DWORD dwVal = (DWORD)m_filterCategory;
     ::RegSetValueEx(hKey, _T("LastCategory"), 0, REG_DWORD,
            (const BYTE*)&dwVal, sizeof(DWORD));

     ::RegSetValueEx(hKey, _T("LastLetter"), 0, REG_SZ,
  (const BYTE*)m_filterLetter.c_str(),
       (m_filterLetter.GetLength() + 1) * sizeof(TCHAR));

  ::RegCloseKey(hKey);
  }
}

// ====================================================================
// DATA MANAGEMENT - IMPLEMENTATION
// ====================================================================

void CListCustomerView::SetColumns()
{
  DeleteAllItems();

    LV_COLUMN column{};
    column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    column.fmt = LVCFMT_LEFT;

    const WCHAR* headers[] = {
        L"Cari Kod",
      L"Ad Soyadý",
        L"Telefon",
        L"Ýlan No",
        L"E-Posta",
        L"Web",
        L"Þehir",
 L"Ýlçe",
  L"TCKN",
        L"Vergi No",
        L"Tipi",
        L"Tarihi",
        L"Durum",
        L"Notlar",
        L"Çalýþma"
    };

    for (int i = 0; i < _countof(headers); ++i)
    {
    column.pszText = const_cast<LPWSTR>(static_cast<LPCWSTR>(headers[i]));
    column.cx = (i == 0) ? DpiScaleInt(3) : DpiScaleInt(100);
InsertColumn(i, column);
    }
}

void CListCustomerView::InsertItems()
{
    DeleteAllItems();

    if (!db_Manager.IsConnected())
        db_Manager.EnsureConnection();

    std::vector<Customer_cstr> customerList = db_Manager.GetCustomers();
    SetRedraw(FALSE);

    int itemCount = 0;

    for (size_t i = 0; i < customerList.size(); ++i)
    {
   const Customer_cstr& c = customerList[i];

        if (m_currentFilter != _T("HEPSI"))
    {
       if (c.AdSoyad.IsEmpty())
   continue;

        CString firstLetter = c.AdSoyad.Left(1);
            int result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
     firstLetter, 1, m_currentFilter, 1);

        if (result != CSTR_EQUAL)
     continue;
  }

     int item = InsertItem(itemCount, c.Cari_Kod);
      SetItemText(item, 1, c.AdSoyad);
        SetItemText(item, 2, c.Telefon);
    SetItemText(item, 3, c.Telefon2);
        SetItemText(item, 4, c.Email);
        SetItemText(item, 5, c.Adres);
        SetItemText(item, 6, c.Sehir);
        SetItemText(item, 7, c.Ilce);
        SetItemText(item, 8, c.TCKN);
        SetItemText(item, 9, c.VergiNo);
        SetItemText(item, 10, c.MusteriTipi);
        SetItemText(item, 11, c.KayitTarihi);
        SetItemText(item, 12, c.Durum);
        SetItemText(item, 13, c.Notlar);
     SetItemText(item, 14, c.Calisma_Durumu);
        SetItemData(item, i);

itemCount++;
    }

  SetRedraw(TRUE);
    Invalidate();
}

void CListCustomerView::AddItemFromStruct(const Customer_cstr& c)
{
    int nIndex = GetItemCount();
  int item = InsertItem(nIndex, c.Cari_Kod);

    SetItemText(item, 1, c.AdSoyad);
SetItemText(item, 2, c.Telefon);
    SetItemText(item, 3, c.Telefon2);
    SetItemText(item, 4, c.Email);
    SetItemText(item, 5, c.Adres);
    SetItemText(item, 6, c.Sehir);
    SetItemText(item, 7, c.Ilce);
    SetItemText(item, 8, c.TCKN);
    SetItemText(item, 9, c.VergiNo);
    SetItemText(item, 10, c.MusteriTipi);
    SetItemText(item, 11, c.KayitTarihi);
    SetItemText(item, 12, c.Durum);
    SetItemText(item, 13, c.Notlar);
    SetItemText(item, 14, c.Calisma_Durumu);

    SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    EnsureVisible(item, FALSE);
}

void CListCustomerView::AddCustomerData(const std::vector<Customer_cstr>& customers)
{
DeleteAllItems();
    SetRedraw(FALSE);

    for (size_t i = 0; i < customers.size(); ++i)
    {
        const Customer_cstr& c = customers[i];

 int nItem = InsertItem((int)i, c.Cari_Kod);

    SetItemText(nItem, 1, c.AdSoyad);
    SetItemText(nItem, 2, c.Telefon);
        SetItemText(nItem, 3, c.Telefon2);
   SetItemText(nItem, 4, c.Email);
        SetItemText(nItem, 5, c.Adres);
   SetItemText(nItem, 6, c.Sehir);
   SetItemText(nItem, 7, c.Ilce);
        SetItemText(nItem, 8, c.TCKN);
        SetItemText(nItem, 9, c.VergiNo);
 SetItemText(nItem, 10, c.MusteriTipi);
   SetItemText(nItem, 11, c.KayitTarihi);
        SetItemText(nItem, 12, c.Durum);
        SetItemText(nItem, 13, c.Notlar);
        SetItemText(nItem, 14, c.Calisma_Durumu);

        SetItemData(nItem, i);
    }

    SetRedraw(TRUE);
    Invalidate();
}

void CListCustomerView::RefreshCustomerList()
{
    SetRedraw(FALSE);
    DeleteAllItems();

 std::vector<Customer_cstr> allCustomers = db_Manager.GetCustomers();

    for (const auto& c : allCustomers)
    {
        if (m_currentFilter != _T("HEPSI"))
        {
       if (c.AdSoyad.IsEmpty())
       continue;

CString firstLetter = c.AdSoyad.Left(1);
            int result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
     firstLetter, 1, m_currentFilter, 1);

            if (result != CSTR_EQUAL)
  continue;
        }

        AddItemFromStruct(c);
    }

    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

void CListCustomerView::RefreshList()
{
    SetRedraw(FALSE);
    DeleteAllItems();

    std::vector<Customer_cstr> allCustomers = db_Manager.GetCustomers();

    for (const auto& c : allCustomers)
    {
        // Kategori filtresi
      bool bCatMatch = false;
        if (m_filterCategory == 0)
        {
       bCatMatch = true;
        }
     else
        {
    int cStatus = _ttoi(c.Calisma_Durumu);
if (cStatus == m_filterCategory)
                bCatMatch = true;
    }

        if (!bCatMatch)
    continue;

        // Harf filtresi
  bool bLetterMatch = false;
        if (m_filterLetter == _T("HEPSI"))
        {
            bLetterMatch = true;
        }
        else
     {
   if (!c.AdSoyad.IsEmpty())
            {
        CString first = c.AdSoyad.Left(1);
                if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
 first, 1, m_filterLetter, 1) == CSTR_EQUAL)
       bLetterMatch = true;
  }
        }

        if (bLetterMatch)
        {
            AddItemFromStruct(c);
        }
    }

    SetRedraw(TRUE);
    Invalidate();
}

// ====================================================================
// FILTERING - IMPLEMENTATION
// ====================================================================

void CListCustomerView::SetCategoryFilter(int catID)
{
    m_filterCategory = catID;
    SaveSettings();
    RefreshList();
}

void CListCustomerView::SetLetterFilter(const CString& letter)
{
    m_filterLetter = letter;
    SaveSettings();
    RefreshList();
}

void CListCustomerView::SetFilter(const CString& filterLetter)
{
    m_currentFilter = filterLetter;
    RefreshCustomerList();
}

void CListCustomerView::SetViewMode(EViewMode mode)
{
 if (m_viewMode == mode)
        return;

    m_viewMode = mode;
    DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);

    if (mode == EViewMode::Report)
    {
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);
        SetColumns();
    }
    else
    {
        SetWindowLongPtr(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_ICON);
        const int CARD_TOTAL_WIDTH = DpiScaleInt(CARD_TOTAL_WIDTH_BASE);
        const int CARD_TOTAL_HEIGHT = DpiScaleInt(CARD_TOTAL_HEIGHT_BASE);
     ListView_SetIconSpacing(GetHwnd(), CARD_TOTAL_WIDTH, CARD_TOTAL_HEIGHT);
    }

 Invalidate();
}

// ====================================================================
// DATA ACCESS - IMPLEMENTATION
// ====================================================================

Customer_cstr CListCustomerView::GetSelectedCustomerData()
{
    Customer_cstr c;

    int iSelectedItem = GetNextItem(-1, LVNI_SELECTED);
    if (iSelectedItem < 0)
        return c;

    c.Cari_Kod = GetItemText(iSelectedItem, 0);
    c.AdSoyad = GetItemText(iSelectedItem, 1);
    c.Telefon = GetItemText(iSelectedItem, 2);
    c.Telefon2 = GetItemText(iSelectedItem, 3);
 c.Email = GetItemText(iSelectedItem, 4);
    c.Adres = GetItemText(iSelectedItem, 5);
  c.Sehir = GetItemText(iSelectedItem, 6);
    c.Ilce = GetItemText(iSelectedItem, 7);
    c.TCKN = GetItemText(iSelectedItem, 8);
    c.VergiNo = GetItemText(iSelectedItem, 9);
    c.MusteriTipi = GetItemText(iSelectedItem, 10);
    c.KayitTarihi = GetItemText(iSelectedItem, 11);
    c.Durum = GetItemText(iSelectedItem, 12);
    c.Notlar = GetItemText(iSelectedItem, 13);
    c.Calisma_Durumu = GetItemText(iSelectedItem, 14);

    return c;
}

CString CListCustomerView::GetSelectedItemText(int nSubItem)
{
    int iSelectedItem = GetNextItem(-1, LVNI_SELECTED);
    if (iSelectedItem >= 0)
        return GetItemText(iSelectedItem, nSubItem);
    return _T("");
}

std::vector<CString> CListCustomerView::GetSelectedCariCodes()
{
    std::vector<CString> selectedCodes;
    int nItem = -1;

    while ((nItem = GetNextItem(nItem, LVNI_SELECTED)) != -1)
    {
    CString cariKod = GetItemText(nItem, 0);
        if (!cariKod.IsEmpty())
            selectedCodes.push_back(cariKod);
    }

    return selectedCodes;
}

// ====================================================================
// STUB IMPLEMENTATIONS (TODO: Complete these)
// ====================================================================

void CListCustomerView::OnNewCustomer() { /* TODO */ }
void CListCustomerView::OnEditCustomer(const CString& cariKod) { /* TODO */ }
void CListCustomerView::UpdateCustomerStatus(const CString& cariKod, int commandID) { /* TODO */ }
void CListCustomerView::OnAddHome(const CString& cariKod) { /* TODO */ }
void CListCustomerView::OnAddLand(const CString& cariKod) { /* TODO */ }
void CListCustomerView::OnAddVineYard(const CString& cariKod) { /* TODO */ }
void CListCustomerView::OnAddCommerical(const CString& cariKod) { /* TODO */ }
void CListCustomerView::OnAddField(const CString& cariKod) { /* TODO */ }
void CListCustomerView::OnAddVilla(const CString& cariKod) { /* TODO */ }
void CListCustomerView::ExportToExcelCSV(const CString& filePath) { /* TODO */ }
void CListCustomerView::ImportFromExcelCSV(const CString& filePath) { /* TODO */ }
void CListCustomerView::OnSaveListToDatabase() { /* TODO */ }

void CListCustomerView::OnInlineEditCommit(const CString& newText) { /* TODO */ }
void CListCustomerView::OnInlineComboCommit(const CString& newValue) { /* TODO */ }
void CListCustomerView::OnItemChanged(NMLISTVIEW* pNM) { /* TODO */ }

void CListCustomerView::DrawCardItemProfessional(LPNMLVCUSTOMDRAW pLVCD, CDC& dc) { /* TODO */ }
void CListCustomerView::DrawHoverBar(LPNMLVCUSTOMDRAW pLVCD, CDC& dc) { /* TODO */ }
void CListCustomerView::DrawSubItem(NMLVCUSTOMDRAW* pLVCD) { /* TODO */ }

LRESULT CListCustomerView::HandleReportItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD) { return CDRF_DODEFAULT; }
LRESULT CListCustomerView::HandleReportSubItemPrePaint(LPNMLVCUSTOMDRAW pNMLVCD) { return CDRF_DODEFAULT; }

CString CListCustomerView::GetColumnFieldName(int subItem) { return _T(""); }
int CListCustomerView::GetSelectedSubItem() const { return -1; }
void CListCustomerView::DeleteAllSelections() { /* TODO */ }
void CListCustomerView::HandlePopupMenu(int x, int y) { /* TODO */ }
void CListCustomerView::HandleMouseHoverPopup(WPARAM wparam, LPARAM lparam) { /* TODO */ }

LRESULT CListCustomerView::HandleKeyDown(WPARAM wparam) { return -1; }
LRESULT CListCustomerView::HandleLButtonDown(LPARAM lparam) { return -1; }
LRESULT CListCustomerView::HandleLButtonDblClk(LPARAM lparam) { return -1; }

LRESULT CListCustomerView::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
{
    SetFocus();
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CListCustomerView::OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam)
{
    SetDPIImages();
    SetDPIColumnWidths();
    return FinalWindowProc(msg, wparam, lparam);
}

void CListCustomerView::SetDPIColumnWidths() { /* TODO */ }
void CListCustomerView::SetDPIImages() { /* TODO */ }
void CListCustomerView::AdjustCustomerColumnWidths() { /* TODO */ }
void CListCustomerView::OnSize(UINT nType, int cx, int cy) { /* TODO */ }

COLORREF CListCustomerView::GetGroupColor(CustomerGroup group) const
{
    return RGB(200, 200, 200);
}

// ====================================================================
// MESSAGE HANDLERS - STUBS
// ====================================================================

LRESULT CListCustomerView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
switch (msg)
 {
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MOUSEMOVE:
    case WM_MOUSELEAVE:
    case WM_CONTEXTMENU:
    case WM_DPICHANGED_BEFOREPARENT:
        break;
    }

    return WndProcDefault(msg, wparam, lparam);
}

LRESULT CListCustomerView::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
{
    return CListView::WndProcDefault(msg, wparam, lparam);
}

BOOL CListCustomerView::OnCommand(WPARAM wparam, LPARAM lparam)
{
 return FALSE;
}

LRESULT CListCustomerView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lparam);

    if (pNMHDR->code == NM_CUSTOMDRAW)
    {
        return OnCustomDraw(reinterpret_cast<NMCUSTOMDRAW*>(lparam));
    }

    switch (pNMHDR->code)
    {
    case LVN_ITEMCHANGED:
        OnItemChanged(reinterpret_cast<NMLISTVIEW*>(lparam));
        return 0;
    }

    return CListView::OnNotifyReflect(wparam, lparam);
}

LRESULT CListCustomerView::OnCustomDraw(NMCUSTOMDRAW* pNMCD)
{
    return CDRF_DODEFAULT;
}

#endif // CLISTCUSTOMERVIEWREFACTORED_CPP
