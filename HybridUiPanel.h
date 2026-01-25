#pragma once
#include "stdafx.h"
#include "SchemaManager.h"
#include "wxx_wincore.h"
#include "wxx_cstring.h"
#include <map>
#include <vector>

// ============================================================================
//  HybridUiPanel - RC-first, Dynamic-fallback panel for any dialog
//  ---------------------------------------------------------------------------
//  - If a schema field has resID and dialog already has that control -> DO NOTHING (RC layout wins)
//  - Otherwise creates a control automatically inside a docked right panel
//  - Supports: FT_TEXT/FT_INT/FT_REAL (Edit), FT_MEMO (Multiline Edit), FT_BOOL (Checkbox)
//  - LoadFromMap / SaveToMap integrate with existing Bind_Data_To_UI / Bind_UI_To_Data
// ============================================================================

class HybridUiPanel : public Win32xx::CWnd
{
public:
    bool CreatePanel(Win32xx::CWnd& parentDlg, const CString& tableName, int dockWidth = 320)
    {
        m_parentDlg = &parentDlg;
        m_tableName = tableName;
        m_dockWidth = dockWidth;

        RECT rc{}; ::GetClientRect(parentDlg, &rc);
        RECT rcPanel{ rc.right - m_dockWidth, 6, rc.right - 6, rc.bottom - 6 };

        HWND h = CreateEx(0, _T("STATIC"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL,
            rcPanel, parentDlg, 0);

        if (!h) return false;

        BuildRows();
        LayoutRows();
        UpdateScroll();
        return true;
    }

    void DockToRight(const RECT& parentClient, int bottomLimitY = -1)
    {
        RECT rcPanel{
            parentClient.right - m_dockWidth,
            6,
            parentClient.right - 6,
            (bottomLimitY > 0 ? bottomLimitY - 6 : parentClient.bottom - 6)
        };

        ::SetWindowPos(*this, nullptr,
            rcPanel.left, rcPanel.top,
            rcPanel.right - rcPanel.left,
            rcPanel.bottom - rcPanel.top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        LayoutRows();
        UpdateScroll();
    }

    void ApplyDialogFont(HFONT hFont)
    {
        if (!hFont) hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
        m_hFont = hFont;
        ApplyFontRecursive(*this, hFont);
    }

    void LoadFromMap(const std::map<CString, CString>& data)
    {
        for (auto& r : m_rows)
        {
            auto it = data.find(r.def.dbName);
            if (it == data.end()) continue;

            if (r.isCheckbox)
                SetCtrlCheck(r.hCtrl, (it->second == _T("1") || it->second.CompareNoCase(_T("true")) == 0));
            else
                SetCtrlText(r.hCtrl, it->second);
        }
    }

    void SaveToMap(std::map<CString, CString>& data) const
    {
        for (const auto& r : m_rows)
        {
            if (r.isCheckbox)
                data[r.def.dbName] = GetCtrlCheck(r.hCtrl) ? _T("1") : _T("0");
            else
                data[r.def.dbName] = GetCtrlText(r.hCtrl);
        }
    }

protected:
    virtual LRESULT WndProc(UINT msg, WPARAM wp, LPARAM lp) override
    {
        switch (msg)
        {
        case WM_SIZE:
            LayoutRows();
            UpdateScroll();
            break;

        case WM_VSCROLL:
        {
            SCROLLINFO si{}; si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            ::GetScrollInfo(*this, SB_VERT, &si);

            int pos = si.nPos;
            switch (LOWORD(wp))
            {
            case SB_LINEUP:     pos -= 20; break;
            case SB_LINEDOWN:   pos += 20; break;
            case SB_PAGEUP:     pos -= (int)si.nPage; break;
            case SB_PAGEDOWN:   pos += (int)si.nPage; break;
            case SB_THUMBTRACK: pos = si.nTrackPos; break;
            default: break;
            }

            int maxPos = si.nMax - (int)si.nPage;
            if (maxPos < 0) maxPos = 0;
            if (pos < si.nMin) pos = si.nMin;
            if (pos > maxPos) pos = maxPos;

            m_scrollPos = pos;
            ::SetScrollPos(*this, SB_VERT, m_scrollPos, TRUE);

            LayoutRows();
            return 0;
        }
        }

        return WndProcDefault(msg, wp, lp);
    }

private:
    struct Row
    {
        FieldDef def;             // no default ctor -> we always construct with FieldDef
        HWND hLabel = nullptr;
        HWND hCtrl  = nullptr;
        bool isCheckbox = false;
        bool isMemo = false;

        Row(const FieldDef& f) : def(f) {}
    };

    void BuildRows()
    {
        m_rows.clear();

        const auto& schema = SchemaManager::Get().GetSchema(m_tableName);

        for (const auto& f : schema)
        {
            // RC-first: if dialog has the control already, skip
            if (m_parentDlg && f.resID != 0)
            {
                HWND h = ::GetDlgItem(*m_parentDlg, (int)f.resID);
                if (h) continue;
            }

            m_rows.emplace_back(f);
            Row& r = m_rows.back();

            // Label window (hidden for checkbox)
            r.hLabel = ::CreateWindowEx(0, _T("STATIC"), f.label,
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, *this, nullptr, GetApp()->GetInstanceHandle(), nullptr);

            // Unique id for dynamic controls
            static UINT g_dynId = 45000;
            UINT id = g_dynId++;

            if (f.type == FT_BOOL)
            {
                r.isCheckbox = true;
                r.hCtrl = ::CreateWindowEx(0, _T("BUTTON"), f.label,
                    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                    0, 0, 0, 0, *this, (HMENU)(UINT_PTR)id, GetApp()->GetInstanceHandle(), nullptr);

                ::ShowWindow(r.hLabel, SW_HIDE);
            }
            else if (f.type == FT_MEMO)
            {
                r.isMemo = true;
                r.hCtrl = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
                    WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                    0, 0, 0, 0, *this, (HMENU)(UINT_PTR)id, GetApp()->GetInstanceHandle(), nullptr);
            }
            else
            {
                r.hCtrl = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                    0, 0, 0, 0, *this, (HMENU)(UINT_PTR)id, GetApp()->GetInstanceHandle(), nullptr);
            }
        }

        if (m_hFont)
            ApplyFontRecursive(*this, m_hFont);
    }

    void LayoutRows()
    {
        constexpr int kPad = 10;
        constexpr int kRowGap = 8;
        constexpr int kLabelW = 130;
        constexpr int kCtrlH = 22;
        constexpr int kMemoH = 90;

        RECT rc{}; ::GetClientRect(*this, &rc);
        int w = rc.right - rc.left;
        int y = kPad - m_scrollPos;

        int ctrlW = (w - (kPad * 2) - kLabelW - 8);
        if (ctrlW < 80) ctrlW = 80;

        for (auto& r : m_rows)
        {
            if (r.isCheckbox)
            {
                ::SetWindowPos(r.hCtrl, nullptr,
                    kPad, y, w - (kPad * 2), kCtrlH,
                    SWP_NOZORDER | SWP_NOACTIVATE);
                y += kCtrlH + kRowGap;
                continue;
            }

            ::SetWindowPos(r.hLabel, nullptr,
                kPad, y + 3, kLabelW, kCtrlH,
                SWP_NOZORDER | SWP_NOACTIVATE);

            int h = r.isMemo ? kMemoH : kCtrlH;
            ::SetWindowPos(r.hCtrl, nullptr,
                kPad + kLabelW + 8, y, ctrlW, h,
                SWP_NOZORDER | SWP_NOACTIVATE);

            y += h + kRowGap;
        }

        m_totalHeight = y + m_scrollPos + kPad;
    }

    void UpdateScroll()
    {
        RECT rc{}; ::GetClientRect(*this, &rc);
        int viewH = rc.bottom - rc.top;

        SCROLLINFO si{}; si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = (m_totalHeight > 0 ? m_totalHeight : 0);
        si.nPage = (UINT)viewH;
        si.nPos = m_scrollPos;

        ::SetScrollInfo(*this, SB_VERT, &si, TRUE);
    }

    CString GetCtrlText(HWND hWnd) const
    {
        if (!hWnd) return _T("");
        int len = ::GetWindowTextLength(hWnd);
        CString s;
        ::GetWindowText(hWnd, s.GetBuffer(len + 1), len + 1);
        s.ReleaseBuffer();
        return s;
    }

    void SetCtrlText(HWND hWnd, const CString& text) const
    {
        if (!hWnd) return;
        ::SetWindowText(hWnd, text);
    }

    bool GetCtrlCheck(HWND hWnd) const
    {
        return (::SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
    }

    void SetCtrlCheck(HWND hWnd, bool checked) const
    {
        ::SendMessage(hWnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    static void ApplyFontRecursive(HWND hWnd, HFONT hFont)
    {
        if (!hWnd || !hFont) return;
        ::SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

        for (HWND child = ::GetWindow(hWnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
            ApplyFontRecursive(child, hFont);
    }

private:
    Win32xx::CWnd* m_parentDlg = nullptr;
    CString m_tableName;
    int m_dockWidth = 320;

    std::vector<Row> m_rows;

    int m_scrollPos = 0;
    int m_totalHeight = 0;
    HFONT m_hFont = nullptr;
};
