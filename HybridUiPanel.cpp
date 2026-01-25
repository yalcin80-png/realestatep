#include "stdafx.h"
#include "HybridUiPanel.h"
#include "FeaturesCodec2.h"

#include "SchemaManager.h"

#include <windows.h>

using Win32xx::CString;

namespace
{
    int GetDpi(HWND h)
    {
        HMODULE hUser = ::GetModuleHandle(_T("user32.dll"));
        if (hUser)
        {
            typedef UINT (WINAPI* GetDpiForWindow_t)(HWND);
            auto p = (GetDpiForWindow_t)::GetProcAddress(hUser, "GetDpiForWindow");
            if (p) return (int)p(h);
        }
        HDC dc = ::GetDC(h);
        int dpi = dc ? ::GetDeviceCaps(dc, LOGPIXELSY) : 96;
        if (dc) ::ReleaseDC(h, dc);
        return dpi > 0 ? dpi : 96;
    }

    int Scale(HWND h, int px) { return MulDiv(px, GetDpi(h), 96); }

    // Hard-coded option sets for multi/single fields that we know.
    struct OptSet { std::vector<CString> opts; bool single; };

    OptSet GetStandardOptions(const CString& dbName)
    {
        if (dbName.CompareNoCase(_T("Facades")) == 0)
            return { { _T("Batı"), _T("Doğu"), _T("Güney"), _T("Kuzey") }, false };

        if (dbName.CompareNoCase(_T("FeaturesInterior")) == 0)
            return { { _T("Akıllı Ev"), _T("Amerikan Mutfak"), _T("Balkon"), _T("Duşakabin"), _T("Ebeveyn Banyosu"), _T("Gömme Dolap"), _T("Görüntülü Diafon"), _T("Hilton Banyo"), _T("Kartonpiyer"), _T("Kiler"), _T("Klima"), _T("Panjur"), _T("Seramik Zemin"), _T("Set Üstü Ocak"), _T("Spot Aydınlatma"), _T("Vestiyer"), _T("Wi-Fi"), _T("Fiber İnternet") }, false };

        if (dbName.CompareNoCase(_T("FeaturesExterior")) == 0)
            return { { _T("Isı Yalıtım"), _T("Ses Yalıtım"), _T("Mantolama"), _T("Jeneratör"), _T("Güvenlik"), _T("Kamera Sistemi"), _T("Otopark"), _T("Kapalı Otopark"), _T("Açık Otopark"), _T("Asansör"), _T("Yangın Merdiveni") }, false };

        if (dbName.CompareNoCase(_T("FeaturesNeighborhood")) == 0)
            return { { _T("Alışveriş Merkezi"), _T("Belediye"), _T("Cami"), _T("Eczane"), _T("Hastane"), _T("Okul"), _T("Park"), _T("Semt Pazarı"), _T("Spor Salonu") }, false };

        if (dbName.CompareNoCase(_T("FeaturesTransport")) == 0)
            return { { _T("Otobüs"), _T("Dolmuş"), _T("Metro"), _T("Tramvay"), _T("Minibüs"), _T("Metrobüs") }, false };

        if (dbName.CompareNoCase(_T("FeaturesView")) == 0)
            return { { _T("Şehir"), _T("Doğa"), _T("Deniz"), _T("Göl"), _T("Dağ") }, false };

        if (dbName.CompareNoCase(_T("FeaturesAccessibility")) == 0)
            return { { _T("Engelli Rampası"), _T("Asansör Uygun"), _T("Tekerlekli Sandalye Uygun"), _T("Yaşlı Dostu") }, false };

        if (dbName.CompareNoCase(_T("HousingType")) == 0)
            return { { _T("Ara Kat"), _T("Bahçe Katı"), _T("Çatı Dubleksi"), _T("Dubleks"), _T("Ters Dubleks"), _T("Müstakil") }, true };

        // Default: no known options.
        return { {}, false };
    }

    int DpiScale(HWND hWnd, int px)
    {
        UINT dpi = 96;
        HDC hdc = ::GetDC(hWnd);
        if (hdc) { dpi = (UINT)GetDeviceCaps(hdc, LOGPIXELSY); ::ReleaseDC(hWnd, hdc); }
        return MulDiv(px, (int)dpi, 96);
    }
}

bool HybridUiPanel::CreatePanel(Win32xx::CWnd& parent, DatabaseManager& db, const CString& tableName, int dockWidth)
{
    m_db = &db;
    m_table = tableName;
    m_dockWidth = dockWidth;

    // Create this window (panel)
    RECT rc{};
    ::GetClientRect(parent, &rc);
    int w = dockWidth;
    RECT rcPanel{ rc.right - w - 8, 8, rc.right - 8, rc.bottom - 8 };

    HWND h = CreateEx(0, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, rcPanel, parent, 0);
    if (!h) return false;

    // Create scroll host
    RECT rcHost{ 0, 0, rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top };
    m_host.CreateEx(WS_EX_CLIENTEDGE, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL, rcHost, *this, 1);

    // Font inherits later via SetUiFont
    BuildUI();
    LayoutUI();
    return true;
}

void HybridUiPanel::LayoutDock(const RECT& parentClient, int top, int bottom, int right)
{
    if (!IsWindow()) return;
    RECT rcPanel{ parentClient.right - m_dockWidth - right, top, parentClient.right - right, parentClient.bottom - bottom };
    ::SetWindowPos(*this, nullptr, rcPanel.left, rcPanel.top, rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::SetWindowPos(m_host, nullptr, 0, 0, rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    LayoutUI();
}

void HybridUiPanel::SetUiFont(HFONT hFont)
{
    m_hFont = hFont;
    if (!m_hFont) m_hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
    ApplyFontRecursive(*this, m_hFont);
}

void HybridUiPanel::ClearUI()
{
    for (auto& f : m_fields)
    {
        if (f.hLabel) ::DestroyWindow(f.hLabel);
        if (f.hEdit)  ::DestroyWindow(f.hEdit);
        for (auto h : f.hChecks) ::DestroyWindow(h);
    }
    m_fields.clear();
}

void HybridUiPanel::BuildUI()
{
    ClearUI();
    if (!m_db) return;

    const auto& schema = SchemaManager::Get().GetSchema(m_table);

    // We only build fields that do NOT have a valid RC control.
    for (const auto& fd : schema)
    {
        bool hasRc = false;
        if (fd.resID != 0)
        {
            HWND h = ::GetDlgItem(::GetParent(GetHwnd()), fd.resID);
            if (h) hasRc = true;
        }

        if (hasRc) continue;

        // Skip technical columns that user should not edit
        if (fd.dbName.CompareNoCase(_T("sync_id")) == 0 ||
            fd.dbName.CompareNoCase(_T("Updated_At")) == 0 ||
            fd.dbName.CompareNoCase(_T("Deleted")) == 0)
            continue;

        DynField df;
        df.dbName = fd.dbName;
        df.type = fd.type;

        auto opt = GetStandardOptions(fd.dbName);
        if (!opt.opts.empty())
        {
            df.uiKind = opt.single ? PropertyMap<void>::UiKind::Single : PropertyMap<void>::UiKind::Multi;
            df.singleChoice = opt.single;
        }
        else
        {
            df.uiKind = (fd.type == FT_BOOL) ? PropertyMap<void>::UiKind::Bool : PropertyMap<void>::UiKind::Text;
        }

        // Create label
        df.hLabel = ::CreateWindowEx(0, _T("STATIC"), fd.label,
                                     WS_CHILD | WS_VISIBLE,
                                     0, 0, 10, 10,
                                     m_host, nullptr, ::GetModuleHandle(nullptr), nullptr);

        if (df.uiKind == PropertyMap<void>::UiKind::Text)
        {
            DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
            if (fd.type == FT_MEMO) style |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
            df.hEdit = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), style,
                                        0, 0, 10, 10,
                                        m_host, nullptr, ::GetModuleHandle(nullptr), nullptr);
        }
        else if (df.uiKind == PropertyMap<void>::UiKind::Bool)
        {
            df.hEdit = ::CreateWindowEx(0, _T("BUTTON"), _T("Evet"),
                                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                                        0, 0, 10, 10,
                                        m_host, nullptr, ::GetModuleHandle(nullptr), nullptr);
        }
        else
        {
            // Multi / Single: checkbox grid
            int idx = 0;
            for (const auto& t : opt.opts)
            {
                HWND hChk = ::CreateWindowEx(0, _T("BUTTON"), t,
                                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                                             0, 0, 10, 10,
                                             m_host, nullptr, ::GetModuleHandle(nullptr), nullptr);
                df.hChecks.push_back(hChk);
                (void)idx;
            }
        }

        m_fields.push_back(std::move(df));
    }

    if (m_hFont) ApplyFontRecursive(*this, m_hFont);
}

void HybridUiPanel::LayoutUI()
{
    if (!m_host.IsWindow()) return;

    RECT rc{}; ::GetClientRect(m_host, &rc);
    int pad = DpiScale(m_host, 10);
    int gapY = DpiScale(m_host, 8);
    int rowH = DpiScale(m_host, 22);
    int titleH = DpiScale(m_host, 18);

    int x = pad;
    int y = pad - m_scrollY;
    int w = (rc.right - rc.left) - 2 * pad;

    int colGap = DpiScale(m_host, 10);
    int cols = (w >= DpiScale(m_host, 520)) ? 2 : 1;
    int colW = (cols == 2) ? (w - colGap) / 2 : w;

    for (auto& f : m_fields)
    {
        // label
        ::SetWindowPos(f.hLabel, nullptr, x, y, w, titleH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        y += titleH + DpiScale(m_host, 4);

        if (f.uiKind == PropertyMap<void>::UiKind::Text)
        {
            int h = rowH;
            if (f.type == FT_MEMO) h = DpiScale(m_host, 70);
            ::SetWindowPos(f.hEdit, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            y += h + gapY;
        }
        else if (f.uiKind == PropertyMap<void>::UiKind::Bool)
        {
            ::SetWindowPos(f.hEdit, nullptr, x, y, w, rowH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            y += rowH + gapY;
        }
        else
        {
            // checkbox grid
            int i = 0;
            for (HWND hChk : f.hChecks)
            {
                int cx = x + (i % cols) * (colW + colGap);
                int cy = y + (i / cols) * rowH;
                ::SetWindowPos(hChk, nullptr, cx, cy, colW, rowH, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                i++;
            }
            int rows = (int)((f.hChecks.size() + cols - 1) / cols);
            y += rows * rowH + gapY;
        }
    }

    m_contentH = y + m_scrollY + pad;
    UpdateScrollInfo();
}

void HybridUiPanel::UpdateScrollInfo()
{
    if (!m_host.IsWindow()) return;
    RECT rc{}; ::GetClientRect(m_host, &rc);
    int viewH = rc.bottom - rc.top;
    int maxY = (m_contentH > viewH) ? (m_contentH - viewH) : 0;

    SCROLLINFO si{}; si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin = 0; si.nMax = maxY; si.nPage = viewH; si.nPos = m_scrollY;
    ::SetScrollInfo(m_host, SB_VERT, &si, TRUE);
}

void HybridUiPanel::ScrollTo(int y)
{
    if (!m_host.IsWindow()) return;
    RECT rc{}; ::GetClientRect(m_host, &rc);
    int viewH = rc.bottom - rc.top;
    int maxY = (m_contentH > viewH) ? (m_contentH - viewH) : 0;
    if (y < 0) y = 0;
    if (y > maxY) y = maxY;
    if (y == m_scrollY) return;
    m_scrollY = y;
    LayoutUI();
    ::InvalidateRect(m_host, nullptr, TRUE);
}

void HybridUiPanel::LoadFromMap(const std::map<CString, CString>& data)
{
    for (auto& f : m_fields)
    {
        auto it = data.find(f.dbName);
        if (it == data.end()) continue;

        if (f.uiKind == PropertyMap<void>::UiKind::Text)
        {
            SetWindowTextC(f.hEdit, it->second);
        }
        else if (f.uiKind == PropertyMap<void>::UiKind::Bool)
        {
            CString v = it->second; v.Trim();
            bool on = (v == _T("1") || v.CompareNoCase(_T("true")) == 0 || v.CompareNoCase(_T("evet")) == 0);
            ::SendMessage(f.hEdit, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        else
        {
            auto selected = FeaturesCodec2::DecodeToSet(it->second);
            for (HWND hChk : f.hChecks)
            {
                CString t = GetWindowTextC(hChk);
                bool on = selected.count(t) > 0;
                ::SendMessage(hChk, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
            }
        }
    }
}

void HybridUiPanel::SaveToMap(std::map<CString, CString>& data) const
{
    for (const auto& f : m_fields)
    {
        if (f.uiKind == PropertyMap<void>::UiKind::Text)
        {
            data[f.dbName] = GetWindowTextC(f.hEdit);
        }
        else if (f.uiKind == PropertyMap<void>::UiKind::Bool)
        {
            LRESULT chk = ::SendMessage(f.hEdit, BM_GETCHECK, 0, 0);
            data[f.dbName] = (chk == BST_CHECKED) ? _T("1") : _T("0");
        }
        else
        {
            std::set<CString> sel;
            for (HWND hChk : f.hChecks)
                if (::SendMessage(hChk, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    sel.insert(GetWindowTextC(hChk));

            if (f.singleChoice)
            {
                CString one;
                if (!sel.empty()) one = *sel.begin();
                data[f.dbName] = FeaturesCodec2::NormalizeSingle(one);
            }
            else
            {
                data[f.dbName] = FeaturesCodec2::EncodeJsonArray(sel);
            }
        }
    }
}

CString HybridUiPanel::GetWindowTextC(HWND h)
{
    CString s;
    if (!h) return s;
    int len = ::GetWindowTextLength(h);
    if (len <= 0) return s;
    std::vector<TCHAR> buf((size_t)len + 1);
    ::GetWindowText(h, buf.data(), len + 1);
    s = buf.data();
    return s;
}

void HybridUiPanel::SetWindowTextC(HWND h, const CString& s)
{
    if (h) ::SetWindowText(h, s);
}

void HybridUiPanel::ApplyFontRecursive(HWND h, HFONT f)
{
    if (!h || !f) return;
    ::SendMessage(h, WM_SETFONT, (WPARAM)f, TRUE);
    for (HWND c = ::GetWindow(h, GW_CHILD); c; c = ::GetWindow(c, GW_HWNDNEXT))
        ApplyFontRecursive(c, f);
}

LRESULT HybridUiPanel::WndProc(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_SIZE:
        LayoutUI();
        break;
    case WM_VSCROLL:
    {
        int code = LOWORD(wp);
        SCROLLINFO si{}; si.cbSize = sizeof(si); si.fMask = SIF_ALL;
        ::GetScrollInfo(m_host, SB_VERT, &si);
        int y = m_scrollY;
        int line = Scale(m_host, 24);
        if (code == SB_LINEUP) y -= line;
        else if (code == SB_LINEDOWN) y += line;
        else if (code == SB_PAGEUP) y -= (int)si.nPage;
        else if (code == SB_PAGEDOWN) y += (int)si.nPage;
        else if (code == SB_THUMBTRACK) y = (int)si.nTrackPos;
        ScrollTo(y);
        return 0;
    }
    }
    return WndProcDefault(msg, wp, lp);
}
