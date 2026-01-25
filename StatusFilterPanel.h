
#pragma once

// ============================================================================
//  StatusFilterPanel.h  (Win32++)
//  --------------------------------------------------------------------------
//  Purpose
//    ListView üstünde yatay, scroll edilebilir "kategori/durum" sekmeleri.
//    AlphaFilterPanel gibi hızlı seçim sağlar.
//
//  Contract
//    - Parent'a UWM_STATUS_FILTER mesajı yollar.
//      WPARAM = statusID (0 -> HEPSI)
//
//  UX
//    - Tek satır, sağa/sola kaydırmalı (mouse wheel ile de kayar)
//    - Tab başlıklarının yanında sayı: "Randevu (12)"
//    - Seçili tab: modern pill + ince alt çizgi
//
//  Dependencies
//    - Win32++ (wxx_wincore.h)
// ============================================================================

#include "wxx_wincore.h"
#include <vector>
#include <algorithm>

#ifndef UWM_STATUS_FILTER
  #define UWM_STATUS_FILTER 9520
#endif
#ifndef UWM_STATUS_COUNTS
  #define UWM_STATUS_COUNTS 9521
#endif

struct StatusTabItem
{
    int                 id = 0;        // 0 => HEPSI
    Win32xx::CString    text;          // base text
    int                 count = -1;    // -1 => gizle
};

class CStatusFilterBarTop : public Win32xx::CWnd
{
public:
    CStatusFilterBarTop() = default;
    virtual ~CStatusFilterBarTop() override = default;

    BOOL Create(HWND parent, UINT id = 0U)
    {
        DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP;
        return (CWnd::CreateEx(0, nullptr, _T(""), style, 0, 0, 10, 10, parent, (HMENU)(INT_PTR)id) != nullptr);
    }

    void SetTabs(const std::vector<StatusTabItem>& tabs)
    {
        m_tabs = tabs;
        if (m_selectedIndex >= (int)m_tabs.size()) m_selectedIndex = 0;
        m_scrollX = 0;
        RecalcMetrics();
        Invalidate();
    }

    void UpdateCounts(const std::vector<std::pair<int,int>>& counts)
    {
        // counts: (id,count)
        for (auto& t : m_tabs)
            t.count = -1;

        for (const auto& kv : counts)
        {
            for (auto& t : m_tabs)
            {
                if (t.id == kv.first)
                {
                    t.count = kv.second;
                    break;
                }
            }
        }
        RecalcMetrics();
        Invalidate();
    }

    void SetSelectedById(int statusId)
    {
        for (int i = 0; i < (int)m_tabs.size(); ++i)
        {
            if (m_tabs[i].id == statusId)
            {
                m_selectedIndex = i;
                EnsureTabVisible(i);
                Invalidate();
                return;
            }
        }
        m_selectedIndex = 0;
        EnsureTabVisible(0);
        Invalidate();
    }

    int GetDesiredHeight() const
    {
        return DpiScaleInt(m_height);
    }

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override
    {
        switch (msg)
        {
        case WM_SIZE:
            RecalcMetrics();
            Invalidate();
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEWHEEL:
        {
            // wheel -> horizontal scroll (Explorer-like)
            const int z = GET_WHEEL_DELTA_WPARAM(wparam);
            const int step = DpiScaleInt(80);
            ScrollBy((z > 0) ? -step : step);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            int oldHot = m_hotIndex;
            m_hotIndex = HitTest(pt);
            if (oldHot != m_hotIndex)
            {
                Invalidate();
                TRACKMOUSEEVENT tme{ sizeof(tme), TME_LEAVE, GetHwnd(), 0 };
                ::TrackMouseEvent(&tme);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            m_hotIndex = -1;
            Invalidate();
            return 0;
        case WM_LBUTTONDOWN:
        {
            POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            if (m_showArrows)
            {
                if (m_rcLeftArrow.PtInRect(pt))
                {
                    ScrollBy(-DpiScaleInt(180));
                    return 0;
                }
                if (m_rcRightArrow.PtInRect(pt))
                {
                    ScrollBy(DpiScaleInt(180));
                    return 0;
                }
            }

            const int idx = HitTest(pt);
            if (idx >= 0 && idx < (int)m_tabs.size())
            {
                m_selectedIndex = idx;
                EnsureTabVisible(idx);
                Invalidate();
                GetParent().PostMessage(UWM_STATUS_FILTER, (WPARAM)m_tabs[idx].id, 0);
            }
            return 0;
        }
        case WM_KEYDOWN:
        {
            if (m_tabs.empty()) break;
            if (wparam == VK_LEFT)
            {
                if (m_selectedIndex > 0) SetSelectedById(m_tabs[m_selectedIndex - 1].id);
                return 0;
            }
            if (wparam == VK_RIGHT)
            {
                if (m_selectedIndex + 1 < (int)m_tabs.size()) SetSelectedById(m_tabs[m_selectedIndex + 1].id);
                return 0;
            }
            if (wparam == VK_HOME)
            {
                SetSelectedById(m_tabs.front().id);
                return 0;
            }
            if (wparam == VK_END)
            {
                SetSelectedById(m_tabs.back().id);
                return 0;
            }
            break;
        }
        case WM_PAINT:
            return OnPaint();
        }
        return WndProcDefault(msg, wparam, lparam);
    }

private:
    std::vector<StatusTabItem> m_tabs;
    std::vector<CRect>         m_itemRects;   // client coords (scroll uygulanmamis "content" rects)

    int  m_hotIndex = -1;
    int  m_selectedIndex = 0;

    // Scroll
    int  m_scrollX = 0;
    int  m_totalContentW = 0;
    bool m_showArrows = false;
    CRect m_rcLeftArrow;
    CRect m_rcRightArrow;

    // Style metrics (DPI scaled in code)
    int m_height = 40;       // control height
    int m_padX   = 10;
    int m_padY   = 6;
    int m_gapX   = 8;
    int m_arrowW = 26;
    int m_round  = 12;

    int DpiScaleInt(int v) const
    {
        // Win32++ has GetWindowDpi? Not always. Use system DPI (96 base) via GetDeviceCaps.
        HDC hdc = ::GetDC(GetHwnd());
        int dpi = (hdc ? GetDeviceCaps(hdc, LOGPIXELSX) : 96);
        if (hdc) ::ReleaseDC(GetHwnd(), hdc);
        return MulDiv(v, dpi, 96);
    }

    // NOTE: We no longer render counts as "(12)".
    // We render a small circular badge (like modern tabs).
    Win32xx::CString MakeCaption(const StatusTabItem& t) const { return t.text; }

    int MeasureTextWidth(HDC hdc, const Win32xx::CString& s) const
    {
        if (!hdc || s.IsEmpty()) return 0;
        SIZE sz{ 0,0 };
        ::GetTextExtentPoint32(hdc, s, s.GetLength(), &sz);
        return sz.cx;
    }

    int MeasureBadgeWidth(HDC hdc, int count, int badgeMinDiamPx) const
    {
        if (count < 0) return 0;
        Win32xx::CString s; s.Format(_T("%d"), count);
        const int txt = MeasureTextWidth(hdc, s);
        // circle-ish badge: keep it at least a diameter, but allow extra width for 3+ digits
        const int pad = DpiScaleInt(8);
        return std::max(badgeMinDiamPx, txt + pad);
    }

    int MeasureTabWidth(HDC hdc, const StatusTabItem& t) const
    {
        const int badgeDiam = DpiScaleInt(18);
        const int textW = MeasureTextWidth(hdc, t.text);
        const int badgeW = (t.count >= 0) ? MeasureBadgeWidth(hdc, t.count, badgeDiam) : 0;
        const int gap = (t.count >= 0) ? DpiScaleInt(6) : 0;
        return textW + badgeW + gap;
    }

    void RecalcMetrics()
    {
        m_itemRects.clear();
        m_totalContentW = 0;
        m_showArrows = false;

        CRect rc = GetClientRect();
        if (rc.IsRectEmpty() || m_tabs.empty()) return;

        // Measure text with current font
        HDC hdc = ::GetDC(GetHwnd());
        HFONT hFontOld = nullptr;
        HFONT hFont = (HFONT)SendMessage(WM_GETFONT, 0, 0);
        if (hdc && hFont) hFontOld = (HFONT)::SelectObject(hdc, hFont);

        int x = 0;
        for (const auto& t : m_tabs)
        {
            const int contentW = MeasureTabWidth(hdc, t);
            const int w = DpiScaleInt(m_padX) * 2 + contentW;
            const int h = rc.Height() - DpiScaleInt(m_padY) * 2;
            CRect r(x, DpiScaleInt(m_padY), x + w, DpiScaleInt(m_padY) + h);
            m_itemRects.push_back(r);
            x += w + DpiScaleInt(m_gapX);
        }

        if (hdc && hFontOld) ::SelectObject(hdc, hFontOld);
        if (hdc) ::ReleaseDC(GetHwnd(), hdc);

        m_totalContentW = (x > 0) ? (x - DpiScaleInt(m_gapX)) : 0;

        // Arrows
        const int availW = rc.Width();
        m_showArrows = (m_totalContentW > availW);

        const int arrowW = DpiScaleInt(m_arrowW);
        m_rcLeftArrow  = CRect(rc.left, rc.top, rc.left + arrowW, rc.bottom);
        m_rcRightArrow = CRect(rc.right - arrowW, rc.top, rc.right, rc.bottom);

        ClampScroll();
    }

    void ClampScroll()
    {
        CRect rc = GetClientRect();
        int avail = rc.Width();
        if (m_showArrows)
            avail = std::max(0, avail - DpiScaleInt(m_arrowW) * 2);

        const int maxScroll = std::max(0, m_totalContentW - avail);
        if (m_scrollX < 0) m_scrollX = 0;
        if (m_scrollX > maxScroll) m_scrollX = maxScroll;
    }

    void ScrollBy(int dx)
    {
        m_scrollX += dx;
        ClampScroll();
        Invalidate();
    }

    int ContentLeft() const
    {
        CRect rc = GetClientRect();
        return m_showArrows ? (rc.left + DpiScaleInt(m_arrowW)) : rc.left;
    }

    CRect ContentRect() const
    {
        CRect rc = GetClientRect();
        if (m_showArrows)
        {
            rc.left += DpiScaleInt(m_arrowW);
            rc.right -= DpiScaleInt(m_arrowW);
        }
        return rc;
    }

    void EnsureTabVisible(int idx)
    {
        if (idx < 0 || idx >= (int)m_itemRects.size()) return;

        CRect vis = ContentRect();
        const int left = m_itemRects[idx].left;
        const int right = m_itemRects[idx].right;

        const int avail = vis.Width();
        if (avail <= 0) return;

        // Current visible content range in content coords
        int visL = m_scrollX;
        int visR = m_scrollX + avail;

        if (left < visL)
            m_scrollX = left;
        else if (right > visR)
            m_scrollX = right - avail;

        ClampScroll();
    }

    int HitTest(const POINT& pt) const
    {
        CRect content = ContentRect();
        if (!content.PtInRect(pt)) return -1;

        // translate point to content coords
        int xContent = pt.x - content.left + m_scrollX;
        int y = pt.y;

        for (int i = 0; i < (int)m_itemRects.size(); ++i)
        {
            CRect r = m_itemRects[i];
            if (xContent >= r.left && xContent < r.right && y >= r.top && y < r.bottom)
                return i;
        }
        return -1;
    }

    void DrawArrow(HDC hdc, const CRect& rc, bool left)
    {
        // minimalist triangle
        const int midY = (rc.top + rc.bottom) / 2;
        const int midX = (rc.left + rc.right) / 2;
        const int s = DpiScaleInt(5);

        POINT pts[3];
        if (left)
        {
            pts[0] = { midX + s, midY - s };
            pts[1] = { midX - s, midY };
            pts[2] = { midX + s, midY + s };
        }
        else
        {
            pts[0] = { midX - s, midY - s };
            pts[1] = { midX + s, midY };
            pts[2] = { midX - s, midY + s };
        }

        HBRUSH b = ::CreateSolidBrush(::GetSysColor(COLOR_3DDKSHADOW));
        HPEN p = ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
        HGDIOBJ oldB = ::SelectObject(hdc, b);
        HGDIOBJ oldP = ::SelectObject(hdc, p);
        ::Polygon(hdc, pts, 3);
        ::SelectObject(hdc, oldB);
        ::SelectObject(hdc, oldP);
        ::DeleteObject(b);
        ::DeleteObject(p);
    }

    LRESULT OnPaint()
    {
        PAINTSTRUCT ps{};
        HDC hdc = ::BeginPaint(GetHwnd(), &ps);
        CRect rc = GetClientRect();

        // Background
        HBRUSH bg = ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
        ::FillRect(hdc, &rc, bg);
        ::DeleteObject(bg);

        HFONT hFont = (HFONT)SendMessage(WM_GETFONT, 0, 0);
        HFONT hOldFont = nullptr;
        if (hFont) hOldFont = (HFONT)::SelectObject(hdc, hFont);

        ::SetBkMode(hdc, TRANSPARENT);

        // Arrows regions
        if (m_showArrows)
        {
            // arrow backplates
            HBRUSH ab = ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
            ::FillRect(hdc, &m_rcLeftArrow, ab);
            ::FillRect(hdc, &m_rcRightArrow, ab);
            ::DeleteObject(ab);

            DrawArrow(hdc, m_rcLeftArrow, true);
            DrawArrow(hdc, m_rcRightArrow, false);
        }

        CRect content = ContentRect();

        // Clip to content area
        HRGN rgn = ::CreateRectRgn(content.left, content.top, content.right, content.bottom);
        ::SelectClipRgn(hdc, rgn);
        ::DeleteObject(rgn);

        // Draw tabs
        const COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
        const COLORREF clrHi   = ::GetSysColor(COLOR_HIGHLIGHT);
        const COLORREF clrWin  = ::GetSysColor(COLOR_WINDOW);

        for (int i = 0; i < (int)m_tabs.size(); ++i)
        {
            CRect r = m_itemRects[i];
            // translate to client
            r.OffsetRect(content.left - m_scrollX, 0);

            // skip if not visible
            if (r.right < content.left || r.left > content.right)
                continue;

            const bool selected = (i == m_selectedIndex);
            const bool hot = (i == m_hotIndex);

            // pill background
            COLORREF fill = clrWin;
            COLORREF border = ::GetSysColor(COLOR_3DSHADOW);
            if (hot) border = ::GetSysColor(COLOR_3DDKSHADOW);
            if (selected) border = clrHi;

            HBRUSH hb = ::CreateSolidBrush(fill);
            HPEN hp = ::CreatePen(PS_SOLID, 1, border);
            HGDIOBJ oldB = ::SelectObject(hdc, hb);
            HGDIOBJ oldP = ::SelectObject(hdc, hp);

            const int rr = DpiScaleInt(m_round);
            ::RoundRect(hdc, r.left, r.top, r.right, r.bottom, rr, rr);

            ::SelectObject(hdc, oldB);
            ::SelectObject(hdc, oldP);
            ::DeleteObject(hb);
            ::DeleteObject(hp);

            // underline for selected
            if (selected)
            {
                const int ulH = DpiScaleInt(2);
                RECT ul{ r.left + DpiScaleInt(10), r.bottom - ulH, r.right - DpiScaleInt(10), r.bottom };
                HBRUSH hul = ::CreateSolidBrush(clrHi);
                ::FillRect(hdc, &ul, hul);
                ::DeleteObject(hul);
            }

            // text + badge
            const StatusTabItem& t = m_tabs[i];
            Win32xx::CString label = t.text;

            RECT tr = r;
            tr.left += DpiScaleInt(m_padX);
            tr.right -= DpiScaleInt(m_padX);

            // Badge rect (right side)
            CRect badgeRc;
            const bool hasBadge = (t.count >= 0);
            if (hasBadge)
            {
                const int diam = DpiScaleInt(18);
                const int bw = MeasureBadgeWidth(hdc, t.count, diam);
                const int bh = diam;
                const int gap = DpiScaleInt(6);

                badgeRc = CRect(tr.right - bw, (r.top + r.bottom - bh) / 2, tr.right, (r.top + r.bottom - bh) / 2 + bh);
                tr.right = badgeRc.left - gap;
            }

            ::SetTextColor(hdc, clrText);
            ::DrawText(hdc, label, label.GetLength(), &tr, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

            if (hasBadge)
            {
                Win32xx::CString badgeText;
                badgeText.Format(_T("%d"), t.count);

                const COLORREF badgeFill = selected ? clrHi : ::GetSysColor(COLOR_3DSHADOW);
                const COLORREF badgeBorder = selected ? clrHi : ::GetSysColor(COLOR_3DDKSHADOW);

                HBRUSH bb = ::CreateSolidBrush(badgeFill);
                HPEN bp = ::CreatePen(PS_SOLID, 1, badgeBorder);
                HGDIOBJ oldB2 = ::SelectObject(hdc, bb);
                HGDIOBJ oldP2 = ::SelectObject(hdc, bp);
                ::Ellipse(hdc, badgeRc.left, badgeRc.top, badgeRc.right, badgeRc.bottom);
                ::SelectObject(hdc, oldB2);
                ::SelectObject(hdc, oldP2);
                ::DeleteObject(bb);
                ::DeleteObject(bp);

                RECT br = badgeRc;
                ::SetTextColor(hdc, RGB(255, 255, 255));
                ::DrawText(hdc, badgeText, badgeText.GetLength(), &br, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            }
        }

        // reset clip
        ::SelectClipRgn(hdc, nullptr);

        if (hOldFont) ::SelectObject(hdc, hOldFont);
        ::EndPaint(GetHwnd(), &ps);
        return 0;
    }
};

