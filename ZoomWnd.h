#pragma once
// ZoomWnd.h - Win32++ CWnd-tabanlı Print Preview / Zoom-Pan penceresi
// Turan YALÇIN için, __cpm_ZoomWnd_cls kodu Win32++ stiline taşınmıştır.

#include <vector>
#include "stdafx.h"
#include "resource.h"
#include <windows.h>
#include <assert.h>

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

//
//#define IDC_OPENHAND                "res\\harrow.cur"
//#define IDC_CLOSEDHAND              "res\\hmove.cur"
//#define IDC_ZOOMOUT                 "res\\zoomout.cur"
//#define IDC_ZOOMIN                  "res\\zoomin.cur"

//#ifndef IDC_OPENHAND
//#define IDC_OPENHAND   IDC_HAND
//#endif
//#ifndef IDC_CLOSEDHAND
//#define IDC_CLOSEDHAND IDC_SIZEALL
//#endif
//#ifndef IDC_ZOOMIN
//#define IDC_ZOOMIN     IDC_UPARROW
//#endif
//#ifndef IDC_ZOOMOUT
//#define IDC_ZOOMOUT    IDC_SIZENS
//#endif

#ifndef IDS_LOADFAILED
#define IDS_LOADFAILED 1
#endif

namespace RenderConfig
{
    // Boyutlar (A4 ölçekli)
    const int MARGIN = 150;
    const int ROW_HEIGHT = 35;
    const int PADDING = 12;

    // -----------------------------------------------------------
    //  KURUMSAL RENKLER (Realty World Gazişehir)
    // -----------------------------------------------------------

    // Ana marka kırmızısı – Realty World logolarındaki ton
    const COLORREF COL_PRIMARY = RGB(200, 30, 45);

    // Başlık / Bölüm başlıkları için koyu siyahımsı gri
    const COLORREF COL_HEADER = RGB(45, 45, 45);

    // Standart metin rengi – soft siyah
    const COLORREF COL_TEXT = RGB(25, 25, 25);

    // Fiyat vurgusu – daha koyu, premium kırmızı
    const COLORREF COL_PRICE = RGB(170, 20, 30);

    // Meta / açıklama yazıları – hafif gri
    const COLORREF COL_META = RGB(110, 110, 110);

    // Zebra satırları – çok açık gri, profesyonel görünüm
    const COLORREF COL_ZEBRA = RGB(242, 242, 242);

    // Çizgi ve kutular – premium gri
    const COLORREF COL_LINE = RGB(200, 200, 200);
}



#define IDC_FIT     2365
#define IDC_BESTFIT     2366
#define IDC_ACTUALSIZE 2367
#define IDC_NORMAL 2368

#define IDC_PRINT 2369

#define IDC_PDF 2370






// (İsteğe bağlı) Kaynaklarınız varsa dahil edin
// #include "resource.h"

// ---- Yardımcı tip ----
typedef struct tagPOINTSIZE_WP
{
    LONG x;
    LONG y;
    LONG cx;
    LONG cy;
} PTSZ, * PPTSZ, * LPPTSZ;

// ---- CZoomWnd ----
class CZoomWnd : public CWnd
{
public:

    // --- Cursor Yönetimi ---
    HCURSOR m_hCursorZoomIn = nullptr;
    HCURSOR m_hCursorZoomOut = nullptr;
    HCURSOR m_hCursorOpen = nullptr;
    HCURSOR m_hCursorClosed = nullptr;

    enum MODE { MODE_PAN, MODE_ZOOMIN, MODE_ZOOMOUT };

    CZoomWnd();
    virtual ~CZoomWnd();


    void CreateToolbar();
    void OnInitialUpdate();

    // Pencere sınıf adı
    virtual LPCTSTR GetClassName() const { return _T("CZoomWnd"); }

    // Oluşturma
    BOOL Create(HWND hParent, DWORD style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
        DWORD exStyle = 0, const RECT& rc = CRect(0, 0, 0, 0));

    // Komutlar / API
    void ZoomIn();
    void ZoomOut();
    void ActualSize();
    void BestFit();

    void SetBitmap(HBITMAP hbmp);
    void SetPalette(HPALETTE hpal);
    void StatusUpdate(int iStatus);
    void Zoom(WPARAM wParam, LPARAM lParam);
    bool SetMode(MODE modeNew);
    int  QueryStatus() const { return m_iStrID; }
    void PreRegisterClass(WNDCLASS& wc);
    HCURSOR LoadAppCursor(UINT id, LPCTSTR sysFallback);

protected:
    // Ana mesaj yönlendirici
    virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnEraseBkgnd(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
    LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseDown(UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseUp(WPARAM wParam, LPARAM lParam);
    LRESULT OnSetCursor(UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnScroll(UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWheelTurn(WPARAM wParam, LPARAM lParam);
    LRESULT OnSetFocus();

    // Yardımcılar
    void AdjustRectPlacement();
    void SetScrollBars();

private:
    // Görsel boyutları
public:
    int m_cxImage = 1;
    int m_cyImage = 1;

private:
    // Durum
    bool    m_bBestFit = true;
    HBITMAP m_hbitmap = nullptr;

    int m_cxCenter = 1;
    int m_cyCenter = 1;

    int m_cxVScroll = 0;
    int m_cyHScroll = 0;
    int m_cxWindow = 0;
    int m_cyWindow = 0;

    int  m_xPosMouse = 0;
    int  m_yPosMouse = 0;

    MODE m_modeDefault = MODE_ZOOMIN;
    PTSZ m_ptszDest{ 0,0,1,1 };
    bool m_fPanning = false;
    bool m_fCtrlDown = false;
    bool m_fShiftDown = false;

    int m_iStrID = 0;      // durum/mesaj göstergesi (isterseniz IDS_* bağlayın)
    HPALETTE m_hpal = nullptr;
    ULONG_PTR m_gdiplusToken = 0;
    // İmleç ID’leri (kaynak yoksa sistem imleçlerine düşer)  
    // static HCURSOR LoadAppCursor(UINT id, LPCTSTR sysFallback);

};
