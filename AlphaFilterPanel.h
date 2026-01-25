#pragma once
#include "wxx_wincore.h"
#include <vector>

#ifndef UWM_ALPHA_FILTER
#define UWM_ALPHA_FILTER 9510
#endif

class CAlphaFilterBarTop : public Win32xx::CWnd
{
public:
    CAlphaFilterBarTop() : m_selectedIndex(0), m_hotIndex(-1), m_isVertical(false) {}
    virtual ~CAlphaFilterBarTop() = default;

    BOOL Create(HWND parent, UINT id = 0U) {
        DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
        return (CWnd::CreateEx(0, nullptr, _T(""), style, 0, 0, 10, 10, parent, (HMENU)(INT_PTR)id) != nullptr);
    }

    // ------------------------------------------------------------
    // Turkish alphabet preset (touch-friendly)
    // ------------------------------------------------------------
    // If the caller doesn't provide a letter set, we can auto-fill
    // the Turkish alphabet so users can filter by "Ç,Ğ,İ,Ö,Ş,Ü".
    static std::vector<Win32xx::CString> BuildTurkishAlphabet(bool includeAll = true)
    {
        // NOTE: Use explicit order for TR.
        static const wchar_t* kLetters[] = {
            L"A", L"B", L"C", L"Ç", L"D", L"E", L"F", L"G", L"Ğ", L"H",
            L"I", L"İ", L"J", L"K", L"L", L"M", L"N", L"O", L"Ö", L"P",
            L"R", L"S", L"Ş", L"T", L"U", L"Ü", L"V", L"Y", L"Z"
        };
        std::vector<Win32xx::CString> out;
        out.reserve(_countof(kLetters) + 1);
        if (includeAll)
            out.push_back(_T("HEPSİ"));
        for (auto s : kLetters)
            out.push_back(Win32xx::CString(s));
        return out;
    }

    void EnsureTurkishAlphabetIfEmpty()
    {
        if (!m_letters.empty())
            return;
        SetLetters(BuildTurkishAlphabet(true));
        SetSelectedState(_T("HEPSİ"));
    }

    void SetLetters(const std::vector<Win32xx::CString>& letters) {
        m_letters = letters;
        RecalcItemRects();
        Invalidate();
    }

    void SetSelectedState(const Win32xx::CString& s) {
        for (int i = 0; i < (int)m_letters.size(); ++i) {
            if (m_letters[i].CompareNoCase(s) == 0) { m_selectedIndex = i; break; }
        }
        Invalidate();
    }

    void SetVertical(bool isVertical) {
        if (m_isVertical != isVertical) {
            m_isVertical = isVertical;
            RecalcItemRects();
            Invalidate();
        }
    }

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override {
        switch (msg) {
        case WM_SIZE: RecalcItemRects(); Invalidate(); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: return OnPaint();
        case WM_CREATE:
            EnsureTurkishAlphabetIfEmpty();
            return 0;
        case WM_MOUSEMOVE: {
            POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            int oldHot = m_hotIndex; m_hotIndex = -1;
            for (int i = 0; i < (int)m_itemRects.size(); ++i) {
                if (m_itemRects[i].PtInRect(pt)) { m_hotIndex = i; break; }
            }
            if (oldHot != m_hotIndex) {
                Invalidate();
                TRACKMOUSEEVENT tme{ sizeof(tme), TME_LEAVE, GetHwnd(), 0 };
                ::TrackMouseEvent(&tme);
            }
            return 0;
        }
        case WM_MOUSELEAVE: m_hotIndex = -1; Invalidate(); return 0;
        case WM_LBUTTONDOWN: {
            POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            for (int i = 0; i < (int)m_itemRects.size(); ++i) {
                if (m_itemRects[i].PtInRect(pt)) {
                    m_selectedIndex = i; Invalidate();
                    const bool isAll = (m_letters[i].CompareNoCase(_T("HEPSİ")) == 0) ||
                                       (m_letters[i].CompareNoCase(_T("HEPSI")) == 0);
                    wchar_t ch = isAll ? 0 : m_letters[i][0];
                    GetParent().SendMessage(UWM_ALPHA_FILTER, (WPARAM)ch, 0);
                    break;
                }
            }
            return 0;
        }
        }
        return WndProcDefault(msg, wparam, lparam);
    }

private:
    std::vector<Win32xx::CString> m_letters;
    std::vector<CRect> m_itemRects;
    int m_hotIndex, m_selectedIndex;
    bool m_isVertical;

    void RecalcItemRects() {
        m_itemRects.clear();
        CRect rc = GetClientRect();
        if (rc.IsRectEmpty() || m_letters.empty()) return;
        int n = (int)m_letters.size();
        for (int i = 0; i < n; ++i) {
            if (m_isVertical) {
                float h = (float)rc.Height() / n;
                m_itemRects.push_back(CRect(0, (int)(i * h), rc.Width(), (int)((i + 1) * h)));
            }
            else {
                float w = (float)rc.Width() / n;
                m_itemRects.push_back(CRect((int)(i * w), 0, (int)((i + 1) * w), rc.Height()));
            }
        }
    }

    LRESULT OnPaint() {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(GetHwnd(), &ps);
        CRect rc = GetClientRect();

        // --- Double Buffering ---
        HDC memDC = ::CreateCompatibleDC(hdc);
        HBITMAP bmp = ::CreateCompatibleBitmap(hdc, rc.Width(), rc.Height());
        HGDIOBJ oldBmp = ::SelectObject(memDC, bmp);

        // Arka Planı Temizle (modern, neutral)
        HBRUSH bgBr = ::CreateSolidBrush(RGB(248, 248, 248));
        ::FillRect(memDC, &rc, bgBr);
        ::DeleteObject(bgBr);

        // Fontu Ayarla
        HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
        ::SelectObject(memDC, hFont);
        ::SetBkMode(memDC, TRANSPARENT);

        // Visual constants
        const int radius = DpiScaleInt(10);
        const COLORREF clrText = RGB(30, 30, 30);
        const COLORREF clrBorder = RGB(210, 210, 210);
        const COLORREF clrSel = RGB(64, 115, 255);

        for (int i = 0; i < (int)m_letters.size(); ++i) {
            if (i >= (int)m_itemRects.size()) break;
            CRect r = m_itemRects[i];

            // Padding (touch)
            r.DeflateRect(DpiScaleInt(3), DpiScaleInt(6));

            const bool selected = (i == m_selectedIndex);
            const bool hot = (i == m_hotIndex);

            // Pill background
            COLORREF fill = RGB(248, 248, 248);
            COLORREF border = clrBorder;
            if (hot) { fill = RGB(240, 246, 255); border = RGB(180, 210, 255); }
            if (selected) { fill = RGB(232, 242, 255); border = clrSel; }

            HBRUSH brFill = ::CreateSolidBrush(fill);
            HPEN   penBrd = ::CreatePen(PS_SOLID, 1, border);
            HGDIOBJ oldBr = ::SelectObject(memDC, brFill);
            HGDIOBJ oldPn = ::SelectObject(memDC, penBrd);
            ::RoundRect(memDC, r.left, r.top, r.right, r.bottom, radius, radius);
            ::SelectObject(memDC, oldBr);
            ::SelectObject(memDC, oldPn);
            ::DeleteObject(brFill);
            ::DeleteObject(penBrd);

            // Text
            ::SetTextColor(memDC, selected ? clrSel : clrText);
            ::DrawText(memDC, m_letters[i], -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        // Ekrana Aktar
        ::BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), memDC, 0, 0, SRCCOPY);

        // Temizlik
        ::SelectObject(memDC, oldBmp);
        ::DeleteObject(bmp);
        ::DeleteDC(memDC);
        ::EndPaint(GetHwnd(), &ps);
        return 0;
    }
};