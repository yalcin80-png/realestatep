#include "stdafx.h"
#include "HomeFeaturesPage.h"
#include "FeaturesCodec.h"
#include <commctrl.h>

namespace
{
    constexpr int kPad = 10;
    constexpr int kGroupGap = 14;
    constexpr int kTitleHeight = 20;
    constexpr int kCheckHeight = 18;
    constexpr int kRowGap = 4;
    constexpr int kColGap = 8;

    inline int ClampInt(int v, int lo, int hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    HFONT GetUiFont(HWND hWnd)
    {
        HFONT f = (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0);
        if (f) return f;
        return (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
    }

    void SetChildFont(HWND hParent, HFONT hFont)
    {
        for (HWND h = ::GetWindow(hParent, GW_CHILD); h; h = ::GetWindow(h, GW_HWNDNEXT))
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    int CalcAutoColumns(int clientW)
    {
        // Responsive kolon: geniþlik arttýkça kolon artar
        // (checkbox label uzunluklarý için konservatif)
        if (clientW >= 980) return 4;
        if (clientW >= 720) return 3;
        return 2;
    }
}

BOOL CHomeFeaturesPage::Create(HWND hParent, const RECT& rc, UINT id, PageKind kind)
{
    m_kind = kind;

    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL;
    DWORD exStyle = WS_EX_CONTROLPARENT;

    HWND hCreated = CWnd::CreateEx(exStyle, _T("STATIC"), _T(""), style, rc, hParent, id);
    if (!hCreated) return FALSE;

    m_hFont = GetUiFont(hParent);
    ::SendMessage(*this, WM_SETFONT, (WPARAM)m_hFont, TRUE);

    BuildGroups();
    CreateControls();

    SetChildFont(*this, m_hFont);

    return TRUE;
}

void CHomeFeaturesPage::BuildGroups()
{
    m_groups.clear();

    if (m_kind == PageKind::Features1)
    {
        m_groups.push_back({ _T("Cephe"), _T("Facades"),
            { _T("Kuzey"), _T("Güney"), _T("Doðu"), _T("Batý") }, 4 });

        m_groups.push_back({ _T("Ýç Özellikler"), _T("FeaturesInterior"),
            {
                _T("ADSL"), _T("Akýllý Ev"), _T("Alaturka Tuvalet"), _T("Alüminyum Doðrama"),
                _T("Ankastre Fýrýn"), _T("Balkon"), _T("Barbekü"), _T("Beyaz Eþya"),
                _T("Boyalý"), _T("Bulaþýk Makinesi"), _T("Duþakabin"), _T("Ebeveyn Banyosu"),
                _T("Fiber Ýnternet"), _T("Fýrýn"), _T("Görüntülü Diafon"), _T("Giyinme Odasý"),
                _T("Hilton Banyo"), _T("Isýcam"), _T("Jakuzi"), _T("Kartonpiyer"),
                _T("Kiler"), _T("Klima"), _T("Laminat"), _T("Marley"),
                _T("Mobilya"), _T("Mutfak Doðalgazý"), _T("Mutfak (Ankastre)"), _T("Parke"),
                _T("PVC Doðrama"), _T("Seramik Zemin"), _T("Set Üstü Ocak"), _T("Þofben"),
                _T("Þömine"), _T("Teras"), _T("Vestiyer"), _T("Çamaþýr Makinesi"),
                _T("Çelik Kapý"), _T("Çift Cam"), _T("Gömme Dolap"), _T("Yerden Isýtma"),
                _T("Yüksek Tavan"), _T("Buzdolabý")
            }, 3 });
    }
    else
    {
        m_groups.push_back({ _T("Dýþ Özellikler"), _T("FeaturesExterior"),
            {
                _T("Asansör"), _T("Güvenlik"), _T("Isý Yalýtým"), _T("Kapalý Garaj"), _T("Kamera Sistemi"),
                _T("Kapýcý"), _T("Otopark"), _T("Oyun Parký"), _T("Ses Yalýtýmý"), _T("Siding"),
                _T("Spor Alaný"), _T("Su Deposu"), _T("Uydu"), _T("Yangýn Merdiveni")
            }, 3 });

        m_groups.push_back({ _T("Muhit"), _T("FeaturesNeighborhood"),
            {
                _T("Alýþveriþ Merkezi"), _T("Belediye"), _T("Cami"), _T("Denize Yakýn"), _T("Eczane"),
                _T("Eðlence Merkezi"), _T("Fuar"), _T("Hastane"), _T("Ýtfaiye"), _T("Lise"),
                _T("Market"), _T("Park"), _T("Polis Merkezi"), _T("Saðlýk Ocaðý"), _T("Spor Salonu"),
                _T("Þehir Merkezi"), _T("Üniversite"), _T("Ýlkokul-Ortaokul")
            }, 3 });

        m_groups.push_back({ _T("Ulaþým"), _T("FeaturesTransport"),
            { _T("Anayol"), _T("Dolmuþ"), _T("Havaalaný"), _T("Metro"), _T("Metrobüs"), _T("Otobüs"), _T("Minibüs"), _T("Tramvay") }, 4 });

        m_groups.push_back({ _T("Manzara"), _T("FeaturesView"),
            { _T("Deniz"), _T("Doða"), _T("Þehir"), _T("Havuz"), _T("Göl"), _T("Dað") }, 4 });

        m_groups.push_back({ _T("Konut Tipi"), _T("HousingType"),
            { _T("Ara Kat"), _T("Bahçe Katý"), _T("Çatý Dubleksi"), _T("Dubleks"), _T("En Üst Kat"), _T("Giriþ Kat"), _T("Kot 1"), _T("Kot 2") }, 4 });

        m_groups.push_back({ _T("Engelli ve Yaþlýya Uygun"), _T("FeaturesAccessibility"),
            { _T("Rampa"), _T("Geniþ Koridor"), _T("Engelli Asansörü"), _T("Tekerlekli Sandalye Uygun"), _T("Tutunma Barý") }, 3 });
    }
}

void CHomeFeaturesPage::CreateControls()
{
    // Destroy previous
    for (auto& c : m_checks) if (c.hWnd) ::DestroyWindow(c.hWnd);
    for (auto& t : m_titles) if (t) ::DestroyWindow(t);
    m_checks.clear();
    m_titles.clear();
    m_groupStartIndex.clear();

    m_titles.reserve(m_groups.size());
    m_groupStartIndex.reserve(m_groups.size());

    int idSeed = 2000;

    for (const auto& g : m_groups)
    {
        m_groupStartIndex.push_back((int)m_checks.size());

        // Title
        HWND hTitle = ::CreateWindowEx(
            0, _T("STATIC"), g.title,
            WS_CHILD | WS_VISIBLE,
            0, 0, 10, 10,
            *this, (HMENU)(INT_PTR)(idSeed++),
            GetApp()->GetInstanceHandle(), nullptr
        );
        m_titles.push_back(hTitle);

        // Checkbox items
        for (const auto& item : g.items)
        {
            HWND h = ::CreateWindowEx(
                0, _T("BUTTON"), item,
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                0, 0, 10, 10,
                *this, (HMENU)(INT_PTR)(idSeed++),
                GetApp()->GetInstanceHandle(), nullptr
            );
            m_checks.push_back({ g.key, item, h });
        }
    }

    if (m_hFont) SetChildFont(*this, m_hFont);

    RECT rc{};
    ::GetClientRect(*this, &rc);
    LayoutControls(rc.right - rc.left, rc.bottom - rc.top);
}

void CHomeFeaturesPage::ClampScroll(int cy)
{
    int maxPos = (m_contentHeight > cy) ? (m_contentHeight - cy) : 0;
    m_scrollY = ClampInt(m_scrollY, 0, maxPos);
}

void CHomeFeaturesPage::ScrollTo(int newPos, int cy, bool redraw)
{
    m_scrollY = newPos;
    ClampScroll(cy);

    ::SetScrollPos(*this, SB_VERT, m_scrollY, TRUE);

    RECT rc{};
    ::GetClientRect(*this, &rc);
    LayoutControls(rc.right - rc.left, rc.bottom - rc.top);

    if (redraw)
        ::InvalidateRect(*this, nullptr, TRUE);
}

void CHomeFeaturesPage::LayoutControls(int cx, int cy)
{
    if (cx <= 0 || cy <= 0)
        return;

    // Responsive columns per group (min:2)
    int autoCols = CalcAutoColumns(cx);

    int x = kPad;
    int y = kPad - m_scrollY;

    // scrollbar boþluðu (Windows kendi çizer ama client daralmasýný hesaplamak iyi olur)
    int sbW = ::GetSystemMetrics(SM_CXVSCROLL);
    int usableW = cx - 2 * kPad - sbW;
    if (usableW < 100) usableW = cx - 2 * kPad; // worst-case

    for (size_t gi = 0; gi < m_groups.size(); ++gi)
    {
        const auto& g = m_groups[gi];

        // Title
        if (gi < m_titles.size() && m_titles[gi])
            ::SetWindowPos(m_titles[gi], nullptr, x, y, usableW, kTitleHeight, SWP_NOZORDER | SWP_NOACTIVATE);

        y += kTitleHeight + 6;

        int cols = g.columns;
        if (cols <= 0) cols = autoCols;
        cols = ClampInt(cols, 2, 5); // güvenli aralýk

        int colW = (usableW - (cols - 1) * kColGap) / cols;
        if (colW < 160) // çok dar ise kolon azalt
        {
            cols = ClampInt(autoCols, 2, 4);
            colW = (usableW - (cols - 1) * kColGap) / cols;
        }

        int start = (gi < m_groupStartIndex.size()) ? m_groupStartIndex[gi] : 0;
        int count = (int)g.items.size();

        int col = 0;
        int rowY = y;

        for (int i = 0; i < count; ++i)
        {
            int idx = start + i;
            if (idx < 0 || idx >= (int)m_checks.size()) break;

            HWND h = m_checks[idx].hWnd;
            if (!h) continue;

            int xItem = x + col * (colW + kColGap);
            int wItem = colW;

            ::SetWindowPos(h, nullptr, xItem, rowY, wItem, kCheckHeight, SWP_NOZORDER | SWP_NOACTIVATE);

            col++;
            if (col >= cols)
            {
                col = 0;
                rowY += kCheckHeight + kRowGap;
            }
        }

        if (col != 0)
            rowY += kCheckHeight + kRowGap;

        y = rowY + kGroupGap;
    }

    // içerik yüksekliði hesapla
    m_contentHeight = (y + m_scrollY + kPad);

    UpdateScrollBar(cx, cy);
    ClampScroll(cy);
}

void CHomeFeaturesPage::UpdateScrollBar(int cx, int cy)
{
    (void)cx;

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    si.nMin = 0;
    si.nMax = (m_contentHeight > 0) ? m_contentHeight : 0;
    si.nPage = (cy > 0) ? (UINT)cy : 0;

    ClampScroll(cy);
    si.nPos = m_scrollY;

    ::SetScrollInfo(*this, SB_VERT, &si, TRUE);

    // içerik sýðarsa scroll'u kapat (görsel temizlik)
    bool need = (m_contentHeight > cy + 2);
    ::ShowScrollBar(*this, SB_VERT, need ? TRUE : FALSE);
}

std::set<CString> CHomeFeaturesPage::GetCheckedForKey(const CString& key) const
{
    std::set<CString> out;
    for (const auto& c : m_checks)
    {
        if (c.groupKey != key || !c.hWnd) continue;
        if (::SendMessage(c.hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
            out.insert(c.text);
    }
    return out;
}

void CHomeFeaturesPage::SetCheckedForKey(const CString& key, const std::set<CString>& values)
{
    for (auto& c : m_checks)
    {
        if (c.groupKey != key || !c.hWnd) continue;
        bool on = (values.find(c.text) != values.end());
        ::SendMessage(c.hWnd, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

void CHomeFeaturesPage::LoadFromMap(const std::map<CString, CString>& record)
{
    auto get = [&](const wchar_t* k) -> CString
        {
            auto it = record.find(k);
            return (it != record.end()) ? it->second : CString(_T(""));
        };

    SetCheckedForKey(_T("Facades"), FeaturesCodec::DecodeToSet(get(L"Facades")));
    SetCheckedForKey(_T("FeaturesInterior"), FeaturesCodec::DecodeToSet(get(L"FeaturesInterior")));
    SetCheckedForKey(_T("FeaturesExterior"), FeaturesCodec::DecodeToSet(get(L"FeaturesExterior")));
    SetCheckedForKey(_T("FeaturesNeighborhood"), FeaturesCodec::DecodeToSet(get(L"FeaturesNeighborhood")));
    SetCheckedForKey(_T("FeaturesTransport"), FeaturesCodec::DecodeToSet(get(L"FeaturesTransport")));
    SetCheckedForKey(_T("FeaturesView"), FeaturesCodec::DecodeToSet(get(L"FeaturesView")));
    SetCheckedForKey(_T("FeaturesAccessibility"), FeaturesCodec::DecodeToSet(get(L"FeaturesAccessibility")));

    CString housing = FeaturesCodec::NormalizeSingle(get(L"HousingType"));
    if (!housing.IsEmpty())
        SetCheckedForKey(_T("HousingType"), std::set<CString>{ housing });

    // Scroll'u en üste al
    RECT rc{};
    ::GetClientRect(*this, &rc);
    m_scrollY = 0;
    UpdateScrollBar(rc.right - rc.left, rc.bottom - rc.top);
}

void CHomeFeaturesPage::SaveToMap(std::map<CString, CString>& record) const
{
    auto putJson = [&](const wchar_t* k, const std::set<CString>& values)
        {
            record[k] = values.empty() ? CString("") : FeaturesCodec::EncodeJsonArray(values);
        };

    putJson(L"Facades", GetCheckedForKey(_T("Facades")));
    putJson(L"FeaturesInterior", GetCheckedForKey(_T("FeaturesInterior")));
    putJson(L"FeaturesExterior", GetCheckedForKey(_T("FeaturesExterior")));
    putJson(L"FeaturesNeighborhood", GetCheckedForKey(_T("FeaturesNeighborhood")));
    putJson(L"FeaturesTransport", GetCheckedForKey(_T("FeaturesTransport")));
    putJson(L"FeaturesView", GetCheckedForKey(_T("FeaturesView")));
    putJson(L"FeaturesAccessibility", GetCheckedForKey(_T("FeaturesAccessibility")));

    auto housingSet = GetCheckedForKey(_T("HousingType"));
    record[L"HousingType"] = housingSet.empty() ? CString("") : *housingSet.begin();
}

LRESULT CHomeFeaturesPage::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        // Flicker azaltmak için: default erase yeterli olur ama TRUE dönmek daha sakin.
        return TRUE;

    case WM_SIZE:
    {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        LayoutControls(cx, cy);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        int z = GET_WHEEL_DELTA_WPARAM(wParam);
        RECT rc{};
        ::GetClientRect(*this, &rc);
        int cy = rc.bottom - rc.top;

        // 1 notch = 120. Adým: 36px
        int step = 36;
        int delta = (z > 0) ? -step : step;
        ScrollTo(m_scrollY + delta, cy, true);
        return 0;
    }

    case WM_VSCROLL:
    {
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        ::GetScrollInfo(*this, SB_VERT, &si);

        RECT rc{};
        ::GetClientRect(*this, &rc);
        int cy = rc.bottom - rc.top;

        int pos = si.nPos;

        switch (LOWORD(wParam))
        {
        case SB_LINEUP:      pos -= 24; break;
        case SB_LINEDOWN:    pos += 24; break;
        case SB_PAGEUP:      pos -= (int)si.nPage; break;
        case SB_PAGEDOWN:    pos += (int)si.nPage; break;
        case SB_THUMBTRACK:  pos = si.nTrackPos; break;
        default: break;
        }

        ScrollTo(pos, cy, false);
        return 0;
    }
    }

    return CWnd::WndProc(uMsg, wParam, lParam);
}
