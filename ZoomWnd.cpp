#include "stdafx.h"
#include "ZoomWnd.h"
#include "RibbonApp.h"
#include <cmath>
#include <tchar.h>


#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

// ---- Yardımcı ----
// --- Cursor yükleyici ---
HCURSOR CZoomWnd::LoadAppCursor(UINT id, LPCTSTR sysFallback)
{
    HINSTANCE hInst = GetApp()->GetInstanceHandle();
    HCURSOR h = ::LoadCursor(hInst, MAKEINTRESOURCE(id));

    if (!h)
        h = ::LoadCursor(nullptr, sysFallback);

    return h;
}


// ---- Ctor / Dtor ----
CZoomWnd::CZoomWnd()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    HINSTANCE hInst = GetApp()->GetInstanceHandle();

    m_hCursorZoomIn = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_ZOOMIN));
    m_hCursorZoomOut = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_ZOOMOUT));
    m_hCursorOpen = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_OPENHAND));
    m_hCursorClosed = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_CLOSEDHAND));
}


CZoomWnd::~CZoomWnd()
{
    // m_hbitmap sahiplik yoksa (kopya handle) serbest bırakmayın; sahiplik sizdeyse DeleteObject yapın.
    // ::DeleteObject(m_hbitmap);

    GdiplusShutdown(m_gdiplusToken);



}

void CZoomWnd::PreRegisterClass(WNDCLASS& wc)
{
    wc.lpszClassName = GetClassName();
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
}





// ---- Create ----
BOOL CZoomWnd::Create(HWND hParent, DWORD style, DWORD exStyle, const RECT& rc)
{
    const int x = rc.left;
    const int y = rc.top;
    const int cx = rc.right - rc.left;
    const int cy = rc.bottom - rc.top;

    // Child kontrol ID'si
    const UINT id = 1001;
    HMENU hMenu = hParent ? reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)) : nullptr;

    return CWnd::CreateEx(exStyle,
        GetClassName(),
        _T("ZoomWnd"),
        style,
        x, y, cx, cy,
        hParent,
        hMenu,
        nullptr) != nullptr;

}



// ---- WndProc ----
LRESULT CZoomWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:   return OnEraseBkgnd(wParam, lParam);
    case WM_PAINT:        return OnPaint(wParam, lParam);
    case WM_SIZE:         return OnSize(wParam, lParam);
    case WM_KEYDOWN:      return OnKeyDown(wParam, lParam);
    case WM_KEYUP:        return OnKeyUp(wParam, lParam);
    case WM_MOUSEMOVE:    return OnMouseMove(wParam, lParam);
    case WM_LBUTTONDOWN:  return OnMouseDown(WM_LBUTTONDOWN, wParam, lParam);
    case WM_MBUTTONDOWN:  return OnMouseDown(WM_MBUTTONDOWN, wParam, lParam);
    case WM_LBUTTONUP:    return OnMouseUp(wParam, lParam);
    case WM_MBUTTONUP:    return OnMouseUp(wParam, lParam);
    case WM_SETCURSOR:    return OnSetCursor(uMsg, wParam, lParam);
    case WM_HSCROLL:      return OnScroll(WM_HSCROLL, wParam, lParam);
    case WM_VSCROLL:      return OnScroll(WM_VSCROLL, wParam, lParam);
    case WM_MOUSEWHEEL:   return OnWheelTurn(wParam, lParam);
    case WM_SETFOCUS:     return OnSetFocus();
    }
    return WndProcDefault(uMsg, wParam, lParam);
}

// ---- Boyama / Arka plan ----
LRESULT CZoomWnd::OnEraseBkgnd(WPARAM wParam, LPARAM)
{
    HDC hdc = (HDC)wParam;
    RECT rcFill{};

    // Görüntü dışındaki bölgeleri boyayın (şeffaf resim desteği kaldırıldı)
    // Sol
    rcFill.left = 0; rcFill.top = m_ptszDest.y;
    rcFill.right = m_ptszDest.x; rcFill.bottom = m_ptszDest.y + m_ptszDest.cy;
    if (rcFill.right > rcFill.left) ::FillRect(hdc, &rcFill, (HBRUSH)(COLOR_SCROLLBAR + 1));

    // Sağ
    rcFill.left = m_ptszDest.x + m_ptszDest.cx; rcFill.right = m_cxWindow;
    if (rcFill.right > rcFill.left) ::FillRect(hdc, &rcFill, (HBRUSH)(COLOR_SCROLLBAR + 1));

    // Üst
    rcFill.left = 0; rcFill.top = 0; rcFill.right = m_cxWindow; rcFill.bottom = m_ptszDest.y;
    if (rcFill.bottom > rcFill.top) ::FillRect(hdc, &rcFill, (HBRUSH)(COLOR_SCROLLBAR + 1));

    // Alt
    rcFill.top = m_ptszDest.y + m_ptszDest.cy; rcFill.bottom = m_cyWindow;
    if (rcFill.bottom > rcFill.top) ::FillRect(hdc, &rcFill, (HBRUSH)(COLOR_SCROLLBAR + 1));

    return TRUE;
}

// CPreviewDlg (veya CZoomWnd) sınıfı için oluşturulmuştur.
// Not: Bu kodu kullanmadan önce, ID'lerinizi (IDC_ZOOMIN, IDC_PRINT vb.) resource.h içinde tanımladığınızdan emin olun.
void CZoomWnd::OnInitialUpdate()
{
}

LRESULT CZoomWnd::OnPaint(WPARAM, LPARAM)
{
    PAINTSTRUCT ps;
    HDC hdcDraw = ::BeginPaint(*this,&ps);

    ::SetMapMode(hdcDraw, MM_TEXT);
    ::SetStretchBltMode(hdcDraw, COLORONCOLOR);

    if (m_hpal)
    {
        ::SelectPalette(hdcDraw, m_hpal, TRUE);
        ::RealizePalette(hdcDraw);
    }

    if (m_hbitmap)
    {
        HDC hdcBitmap = ::CreateCompatibleDC(hdcDraw);
        HGDIOBJ old = ::SelectObject(hdcBitmap, m_hbitmap);

        ::StretchBlt(
            hdcDraw,
            m_ptszDest.x, m_ptszDest.y,
            m_ptszDest.cx, m_ptszDest.cy,
            hdcBitmap,
            0, 0, m_cxImage, m_cyImage,
            SRCCOPY
        );

        ::SelectObject(hdcBitmap, old);
        ::DeleteDC(hdcBitmap);
    }
    else
    {
        RECT rc{ 0,0,m_cxWindow,m_cyWindow };
        ::FillRect(hdcDraw, &rc, (HBRUSH)(COLOR_WINDOW + 1));
        // Basit bir yazı göstermek isterseniz:
        DrawText(hdcDraw, _T("Turan YALÇIN 2025"), -1, &rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }

    ::EndPaint(*this, &ps);
    return 0;
}

// ---- Boyut değişimi ----
LRESULT CZoomWnd::OnSize(WPARAM, LPARAM lParam)
{
    m_cxWindow = LOWORD(lParam);
    m_cyWindow = HIWORD(lParam);

    if (m_bBestFit)
        BestFit();
    else
        AdjustRectPlacement();

    return TRUE;
}

// ---- Klavye ----
LRESULT CZoomWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_PRIOR:  OnScroll(WM_VSCROLL, m_fCtrlDown ? SB_TOP : SB_PAGEUP, 0); break;
    case VK_NEXT:   OnScroll(WM_VSCROLL, m_fCtrlDown ? SB_BOTTOM : SB_PAGEDOWN, 0); break;
    case VK_END:    OnScroll(WM_HSCROLL, m_fCtrlDown ? SB_BOTTOM : SB_PAGEDOWN, 0); break;
    case VK_HOME:   OnScroll(WM_HSCROLL, m_fCtrlDown ? SB_TOP : SB_PAGEUP, 0); break;
    case VK_CONTROL:
    case VK_SHIFT:
        if (!m_fPanning)
        {
            if (wParam == VK_CONTROL) m_fCtrlDown = true;
            if (wParam == VK_SHIFT)   m_fShiftDown = true;
            OnSetCursor(0, 0, 0);
        }
        break;
    default:
        if (GetParent() == nullptr)
            Destroy();
        return 1;
    }
    return 0;
}

LRESULT CZoomWnd::OnKeyUp(WPARAM wParam, LPARAM)
{
    if (wParam == VK_CONTROL) { m_fCtrlDown = false; OnSetCursor(0, 0, 0); }
    else if (wParam == VK_SHIFT) { m_fShiftDown = false; OnSetCursor(0, 0, 0); }
    return 0;
}

// ---- Fare ----
LRESULT CZoomWnd::OnMouseDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
    m_xPosMouse = (short)LOWORD(lParam);
    m_yPosMouse = (short)HIWORD(lParam);
    assert(m_fPanning == false);

    if ((wParam & MK_CONTROL) || (msg == WM_MBUTTONDOWN))
    {
        m_fPanning = true;
        OnSetCursor(0, 0, 0);
        ::SetCapture(*this);
    }
    else
    {
        bool bZoomIn = (m_modeDefault != MODE_ZOOMOUT) ^ ((wParam & MK_SHIFT) ? 1 : 0);
        m_cxCenter = MulDiv(m_xPosMouse - m_ptszDest.x, m_cxImage, m_ptszDest.cx);
        m_cyCenter = MulDiv(m_yPosMouse - m_ptszDest.y, m_cyImage, m_ptszDest.cy);
        bZoomIn ? ZoomIn() : ZoomOut();
    }
    return 0;
}

LRESULT CZoomWnd::OnMouseUp(WPARAM, LPARAM)
{
    if (m_fPanning) ::ReleaseCapture();
    m_fPanning = false;
    return 0;
}

LRESULT CZoomWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    m_fCtrlDown = (wParam & MK_CONTROL) != 0;
    m_fShiftDown = (wParam & MK_SHIFT) != 0;

    if (!(wParam & (MK_LBUTTON | MK_MBUTTON)) || !m_fPanning || !m_hbitmap)
        return TRUE;

    POINTS pt = MAKEPOINTS(lParam);
    PTSZ ptszDest = m_ptszDest;

    // X ekseni
    if (m_ptszDest.cx > m_cxWindow)
        ptszDest.x = m_ptszDest.x + pt.x - m_xPosMouse;

    // Y ekseni
    if (m_ptszDest.cy > m_cyWindow)
        ptszDest.y = m_ptszDest.y + pt.y - m_yPosMouse;

    // Sınırlar / ortalama
    if (ptszDest.cx < m_cxWindow)
        ptszDest.x = (m_cxWindow - ptszDest.cx) / 2;
    else
    {
        if (ptszDest.x < (m_cxWindow - ptszDest.cx)) ptszDest.x = m_cxWindow - ptszDest.cx;
        if (ptszDest.x > 0) ptszDest.x = 0;
    }

    if (ptszDest.cy < m_cyWindow)
        ptszDest.y = (m_cyWindow - ptszDest.cy) / 2;
    else
    {
        if (ptszDest.y < (m_cxWindow - ptszDest.cx)) {} // no-op
        if (ptszDest.y < (m_cyWindow - ptszDest.cy)) ptszDest.y = m_cyWindow - ptszDest.cy;
        if (ptszDest.y > 0) ptszDest.y = 0;
    }

    m_xPosMouse = pt.x;
    m_yPosMouse = pt.y;

    SetScrollBars();

    if (ptszDest.x != m_ptszDest.x || ptszDest.y != m_ptszDest.y ||
        ptszDest.cx != m_ptszDest.cx || ptszDest.cy != m_ptszDest.cy)
    {
        RECT rcInvalid;
        rcInvalid.left = MIN(ptszDest.x, m_ptszDest.x);
        rcInvalid.top = MIN(ptszDest.y, m_ptszDest.y);
        rcInvalid.right = MAX(ptszDest.x + ptszDest.cx, m_ptszDest.x + m_ptszDest.cx);
        rcInvalid.bottom = MAX(ptszDest.y + ptszDest.cy, m_ptszDest.y + m_ptszDest.cy);
        m_ptszDest = ptszDest;
        ::InvalidateRect(*this, &rcInvalid, FALSE);
    }

    m_cxCenter = MulDiv(m_cxWindow / 2 - m_ptszDest.x, m_cxImage, m_ptszDest.cx);
    m_cyCenter = MulDiv(m_cyWindow / 2 - m_ptszDest.y, m_cyImage, m_ptszDest.cy);

    return TRUE;
}





LRESULT CZoomWnd::OnSetCursor(UINT, WPARAM, LPARAM)
{
    if (m_fPanning)
        ::SetCursor(m_hCursorClosed);
    else
    {
        switch (m_modeDefault)
        {
        case MODE_ZOOMIN:  ::SetCursor(m_hCursorZoomIn);  break;
        case MODE_ZOOMOUT: ::SetCursor(m_hCursorZoomOut); break;
        case MODE_PAN:     ::SetCursor(m_hCursorOpen);    break;
        default:           ::SetCursor(m_hCursorClosed); break;
        }
    }
    return TRUE;
}

// ---- Scroll ----
LRESULT CZoomWnd::OnScroll(UINT msg, WPARAM wParam, LPARAM)
{
    if (!m_hbitmap) return 0;

    int iWindow = (msg == WM_HSCROLL) ? m_cxWindow : m_cyWindow;
    LONG* piTL = (msg == WM_HSCROLL) ? &m_ptszDest.x : &m_ptszDest.y;
    LONG  iWH = (msg == WM_HSCROLL) ? m_ptszDest.cx : m_ptszDest.cy;
    int   bar = (msg == WM_HSCROLL) ? SB_HORZ : SB_VERT;

    if (iWindow >= iWH) return 0;

    switch (LOWORD(wParam))
    {
    case SB_TOP:        *piTL = 0; break;
    case SB_PAGEUP:     *piTL += iWindow; break;
    case SB_LINEUP:     (*piTL)++; break;
    case SB_LINEDOWN:   (*piTL)--; break;
    case SB_PAGEDOWN:   *piTL -= iWindow; break;
    case SB_BOTTOM:     *piTL = iWindow - iWH; break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: *piTL = -HIWORD(wParam); break;
    case SB_ENDSCROLL:  return 0;
    }

    if (0 < *piTL) *piTL = 0;
    else if ((iWindow - iWH) > *piTL) *piTL = iWindow - iWH;

    ::SetScrollPos(*this, bar, -(*piTL), TRUE);

    if (msg == WM_HSCROLL)
        m_cxCenter = MulDiv((m_cxWindow / 2) - m_ptszDest.x, m_cxImage, m_ptszDest.cx);
    else
        m_cyCenter = MulDiv((m_cyWindow / 2) - m_ptszDest.y, m_cyImage, m_ptszDest.cy);

    ::InvalidateRect(*this, nullptr, FALSE);
    return 0;
}

LRESULT CZoomWnd::OnWheelTurn(WPARAM wParam, LPARAM)
{
    bool bZoomIn = ((short)HIWORD(wParam) > 0);
    bZoomIn ? ZoomIn() : ZoomOut();
    
    // Ana pencereyi bilgilendir (zoom değişti)
    HWND hParent = ::GetParent(*this);
    if (hParent)
        ::PostMessage(hParent, WM_USER + 102, 0, 0); // Zoom değişti mesajı
    
    return TRUE;
}

LRESULT CZoomWnd::OnSetFocus()
{
    ::SetFocus(::GetParent(*this));
    return 0;
}

// ---- API ----
bool CZoomWnd::SetMode(MODE modeNew)
{
    if (m_modeDefault == modeNew) return false;
    m_modeDefault = modeNew;
    return true;
}

void CZoomWnd::ActualSize()
{
    m_bBestFit = false;
    m_ptszDest.cx = m_cxImage;
    m_ptszDest.cy = m_cyImage;
    m_ptszDest.x = (m_cxWindow - m_cxImage) / 2;
    m_ptszDest.y = (m_cyWindow - m_cyImage) / 2;
    m_cxCenter = m_cxImage / 2;
    m_cyCenter = m_cyImage / 2;
    SetScrollBars();
    ::InvalidateRect(*this, nullptr, FALSE);
}

void CZoomWnd::BestFit()
{
    m_bBestFit = true;

    DWORD dwStyle = (DWORD)::GetWindowLongPtr(*this, GWL_STYLE);
    if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
    {
        m_cxWindow += (dwStyle & WS_VSCROLL) ? m_cxVScroll : 0;
        m_cyWindow += (dwStyle & WS_HSCROLL) ? m_cyHScroll : 0;
    }

    if (m_cxImage <= m_cxWindow && m_cyImage <= m_cyWindow)
    {
        m_ptszDest.x = (m_cxWindow - m_cxImage) / 2;
        m_ptszDest.y = (m_cyWindow - m_cyImage) / 2;
        m_ptszDest.cx = m_cxImage;
        m_ptszDest.cy = m_cyImage;
    }
    else if (m_cxImage * m_cyWindow < m_cxWindow * m_cyImage)
    {
        int iNewWidth = MulDiv(m_cyWindow, m_cxImage, m_cyImage);
        m_ptszDest.x = (m_cxWindow - iNewWidth) / 2;
        m_ptszDest.y = 0;
        m_ptszDest.cx = iNewWidth;
        m_ptszDest.cy = m_cyWindow;
    }
    else
    {
        int iNewHeight = MulDiv(m_cxWindow, m_cyImage, m_cxImage);
        m_ptszDest.x = 0;
        m_ptszDest.y = (m_cyWindow - iNewHeight) / 2;
        m_ptszDest.cx = m_cxWindow;
        m_ptszDest.cy = iNewHeight;
    }

    if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
        SetScrollBars();

    ::InvalidateRect(*this, nullptr, FALSE);
}

void CZoomWnd::AdjustRectPlacement()
{
    DWORD dwStyle = (DWORD)::GetWindowLongPtr(*this, GWL_STYLE);
    if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
    {
        if ((m_ptszDest.cx < (m_cxWindow + ((dwStyle & WS_VSCROLL) ? m_cxVScroll : 0))) &&
            (m_ptszDest.cy < (m_cyWindow + ((dwStyle & WS_HSCROLL) ? m_cyHScroll : 0))))
        {
            m_cxWindow += (dwStyle & WS_VSCROLL) ? m_cxVScroll : 0;
            m_cyWindow += (dwStyle & WS_HSCROLL) ? m_cyHScroll : 0;
            SetScrollBars();
        }
    }

    if ((m_ptszDest.cx < m_cxWindow) && (m_ptszDest.cy < m_cyWindow))
    {
        if ((m_ptszDest.cx < m_cxImage) && (m_ptszDest.cy < m_cyImage))
        {
            BestFit();
            return;
        }
    }

    m_ptszDest.x = (m_cxWindow / 2) - MulDiv(m_cxCenter, m_ptszDest.cx, m_cxImage);
    m_ptszDest.y = (m_cyWindow / 2) - MulDiv(m_cyCenter, m_ptszDest.cy, m_cyImage);

    if (m_ptszDest.cx < m_cxWindow)
        m_ptszDest.x = (m_cxWindow - m_ptszDest.cx) / 2;
    else
    {
        if (m_ptszDest.x < (m_cxWindow - m_ptszDest.cx)) m_ptszDest.x = m_cxWindow - m_ptszDest.cx;
        if (m_ptszDest.x > 0) m_ptszDest.x = 0;
    }

    if (m_ptszDest.cy < m_cyWindow)
        m_ptszDest.y = (m_cyWindow - m_ptszDest.cy) / 2;
    else
    {
        if (m_ptszDest.y < (m_cyWindow - m_ptszDest.cy)) m_ptszDest.y = m_cyWindow - m_ptszDest.cy;
        if (m_ptszDest.y > 0) m_ptszDest.y = 0;
    }

    SetScrollBars();
    ::InvalidateRect(*this, nullptr, FALSE);
}

void CZoomWnd::StatusUpdate(int iStatus)
{
    m_hbitmap = nullptr;
    m_iStrID = iStatus;
    if (*this) ::InvalidateRect(*this, nullptr, TRUE);
}

void CZoomWnd::SetBitmap(HBITMAP hbitmap)
{
    m_hbitmap = hbitmap;
    if (m_hbitmap)
    {
        BITMAP bm{};
        if (::GetObject(m_hbitmap, sizeof(bm), &bm))
        {
            m_cxImage = bm.bmWidth;
            m_cyImage = bm.bmHeight;
            m_cxCenter = m_cxImage / 2;
            m_cyCenter = m_cyImage / 2;

            if (*this)
            {
                BestFit();
                ::InvalidateRect(*this, nullptr, FALSE);
            }
            return;
        }
    }
    // yükleme hatası
    m_iStrID = /*IDS_LOADFAILED*/ -1;
}

void CZoomWnd::SetPalette(HPALETTE hpal)
{
    m_hpal = hpal;
}

void CZoomWnd::Zoom(WPARAM wParam, LPARAM lParam)
{
    switch (wParam & 0xFF)
    {
    case 0: /*IVZ_CENTER*/ break;
        // IVZ_POINT / IVZ_RECT gibi özel durumları isterseniz genişletin
    default: break;
    }
    if (wParam /*& IVZ_ZOOMOUT*/) // prototip: opsiyonel
        ZoomOut();
    else
        ZoomIn();
}

void CZoomWnd::SetScrollBars()
{
    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nMax = m_ptszDest.cx;
    si.nPage = m_cxWindow + 1;
    si.nPos = 0 - m_ptszDest.x;

    ::SetScrollInfo(*this, SB_HORZ, &si, TRUE);

    si.nMax = m_ptszDest.cy;
    si.nPage = m_cyWindow + 1;
    si.nPos = 0 - m_ptszDest.y;

    ::SetScrollInfo(*this, SB_VERT, &si, TRUE);
}

void CZoomWnd::ZoomIn()
{
    if (!m_hbitmap) return;

    m_bBestFit = false;
    m_ptszDest.cy = (LONG)std::ceil(m_ptszDest.cy * 1.200); // 20% büyüt
    if (m_ptszDest.cy >= m_cyImage * 16) m_ptszDest.cy = m_cyImage * 16;
    m_ptszDest.cx = MulDiv(m_ptszDest.cy, m_cxImage, m_cyImage);
    AdjustRectPlacement();
    ::InvalidateRect(*this, nullptr, TRUE);
    ::UpdateWindow(*this);
    
    // Ana pencereyi bilgilendir (zoom değişti)
    HWND hParent = ::GetParent(*this);
    if (hParent)
        ::PostMessage(hParent, WM_USER + 102, 0, 0); // Zoom değişti mesajı
}

void CZoomWnd::ZoomOut()
{
    if (!m_hbitmap) return;

    if ((m_ptszDest.cx <= MIN(m_cxWindow, m_cxImage)) &&
        (m_ptszDest.cy <= MIN(m_cyWindow, m_cyImage)))
    {
        m_bBestFit = true;
        return;
    }

    m_ptszDest.cy = (LONG)std::floor(m_ptszDest.cy * 0.833); // ~%16 küçült
    m_ptszDest.cx = MulDiv(m_ptszDest.cy, m_cxImage, m_cyImage);
    AdjustRectPlacement();
    ::InvalidateRect(*this, nullptr, TRUE);
    ::UpdateWindow(*this);
    
    // Ana pencereyi bilgilendir (zoom değişti)
    HWND hParent = ::GetParent(*this);
    if (hParent)
        ::PostMessage(hParent, WM_USER + 102, 0, 0); // Zoom değişti mesajı
}
