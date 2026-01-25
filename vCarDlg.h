#pragma once

// ============================================================================
//  vCarDlg.h - ULTRA PROFESYONEL VEKTREL EKSPERTZ EDTR (Win32++)
//  --------------------------------------------------------------------------
//  zellikler:
//  - %100 Vektrel izim (Resim dosyas gerektirmez, her boyutta nettir)
//  - GDI+ High Quality Rendering (Przsz izgiler)
//  - Direkler (Podye), Marpiyeller ve Tamponlar dahil tam para listesi
//  - Responsive Tasarm (Pencere bydke araba da byr)
//  - JSON Veritaban Entegrasyonu
// ============================================================================

#include "stdafx.h"
#include "resource.h"
#include "dataIsMe.h"
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

// GDI+ Ktphanesi
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// ---------------------------------------------------------------------------
// ENUM: Para Durumlar (Renk Kodlar)
// ---------------------------------------------------------------------------
enum CarPaintState
{
    CPS_ORJINAL = 0, // Temiz (Beyaz/Gri)
    CPS_BOYALI = 1, // Tam Boyal
    CPS_LOKAL_BOYA = 2, // Lokal Boyal
    CPS_DEGISEN = 3, // Deien
    CPS_SOKTAK = 4, // Sk-Tak (Vida oynanm)
    CPS_PLASTIK = 5, // Plastik Aksam
    CPS_EZIK_CIZIK = 6  // Ezik/izik
};

// ---------------------------------------------------------------------------
// STRUCT: Araba Paras Tanm
// ---------------------------------------------------------------------------
struct CarPaintPart
{
    CString key;            // Veritaban anahtar (rn: "KAPUT")
    CString labelUI;        // izim zerindeki etiket (rn: "K")
    CString labelList;      // Sa listedeki tam ad (rn: "Motor Kaputu")

    std::vector<Gdiplus::Point> pts1000; // 1000x1000 sanal koordinat sistemi
    HRGN hRgn = nullptr;             // Tklama alglama blgesi (Pixel space)
    int state = CPS_ORJINAL;         // Mevcut durum

    void Destroy()
    {
        if (hRgn) { ::DeleteObject(hRgn); hRgn = nullptr; }
    }
};

// ---------------------------------------------------------------------------
// CLASS: CCarPaintCanvas (Vektrel izim Alan)
// ---------------------------------------------------------------------------
class CCarPaintCanvas : public Win32xx::CWnd
{
public:
    CCarPaintCanvas() = default;
    ~CCarPaintCanvas() override { ClearResources(); ShutdownGdiplus(); }

    bool Create(HWND hParent, const CRect& rc, int ctrlId)
    {
        RegisterOnce();
        m_hParent = hParent;

        m_hWnd = ::CreateWindowEx(
            0, kClassName, nullptr,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
            rc.left, rc.top, rc.Width(), rc.Height(),
            hParent, (HMENU)(INT_PTR)ctrlId, ::GetModuleHandle(nullptr), this);

        if (!m_hWnd) return false;

        StartupGdiplus();
        InitVectorParts(); // Vektrel paralar ykle
        return true;
    }

    // --- VER YNETM ---

    // JSON verisini ykler (DB -> UI)
    void LoadFromJson(const CString& jsonStr)
    {
        // nce sfrla
        for (auto& p : m_parts) p.state = CPS_ORJINAL;

        // JSON parse (Basit string aramas - Hz ve bamllk olmamas iin)
        // Format: {"KAPUT":1, "TAVAN":2}
        for (auto& p : m_parts)
        {
            CString keyFind;
            keyFind.Format(_T("\"%s\":"), p.key.c_str());

            int pos = jsonStr.Find(keyFind);
            if (pos >= 0)
            {
                // Deerin yerini bul (key'den sonraki rakam)
                int valStart = pos + keyFind.GetLength();
                // Boluklar atla
                while (valStart < jsonStr.GetLength() && !isdigit(jsonStr[valStart]) && jsonStr[valStart] != '-') valStart++;

                if (valStart < jsonStr.GetLength())
                {
                    int val = _ttoi(jsonStr.Mid(valStart, 1));
                    if (val >= 0 && val <= 6) p.state = val;
                }
            }
        }
        Invalidate();
    }

    // JSON verisi retir (UI -> DB)
    CString BuildJson() const
    {
        CString s = _T("{");
        bool first = true;
        for (const auto& p : m_parts)
        {
            if (p.state == CPS_ORJINAL) continue; // Orjinalleri kaydetme (yer tasarrufu)

            if (!first) s += _T(",");
            CString item;
            item.Format(_T("\"%s\":%d"), p.key.c_str(), p.state);
            s += item;
            first = false;
        }
        s += _T("}");
        return s;
    }

    void ClearAll()
    {
        for (auto& p : m_parts) p.state = CPS_ORJINAL;
        Invalidate();
    }

private:
    static constexpr LPCTSTR kClassName = _T("RW_VectorExpertise_V2");

    HWND m_hWnd{ nullptr };
    HWND m_hParent{ nullptr };

    std::vector<CarPaintPart> m_parts;

    // Layout Cache
    CRect m_rcClient{ 0,0,0,0 };
    CRect m_rcCar{ 0,0,0,0 };   // Arabann izilecei alan
    CRect m_rcLegend{ 0,0,0,0 }; // Alt bilgi alan
    CRect m_rcRight{ 0,0,0,0 };  // Sa liste alan

    // GDI+ Token
    ULONG_PTR m_gdiplusToken{ 0 };

private:
    // --- GDI+ Balatma/Kapatma ---
    void StartupGdiplus()
    {
        if (m_gdiplusToken) return;
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&m_gdiplusToken, &input, nullptr);
    }

    void ShutdownGdiplus()
    {
        if (m_gdiplusToken)
        {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
            m_gdiplusToken = 0;
        }
    }

    // --- Win32 Kayt ---
    static void RegisterOnce()
    {
        static bool s_registered = false;
        if (s_registered) return;

        WNDCLASSEX wc{ sizeof(wc) };
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = &CCarPaintCanvas::WndProc;
        wc.hInstance = ::GetModuleHandle(nullptr);
        wc.hCursor = ::LoadCursor(nullptr, IDC_HAND);
        wc.hbrBackground = nullptr; // OnPaint iinde izeceiz
        wc.lpszClassName = kClassName;
        ::RegisterClassEx(&wc);

        s_registered = true;
    }

    void ClearResources()
    {
        for (auto& p : m_parts) p.Destroy();
    }

    // -----------------------------------------------------------------------
    // PARA TANIMLARI (1000x1000 Vektrel Dzlem)
    // Sahibinden stili simetrik Sedan/Hatchback karm ema
    // -----------------------------------------------------------------------
    void InitVectorParts()
    {
        m_parts.clear();

        // Helper Lambda
        auto Add = [&](LPCTSTR key, LPCTSTR lblUI, LPCTSTR lblList, std::initializer_list<long> xy) {
            CarPaintPart p;
            p.key = key;
            p.labelUI = lblUI;
            p.labelList = lblList;
            auto it = xy.begin();
            while (it != xy.end()) {
                int x = (int)*it++;
                int y = (int)*it++;
                p.pts1000.emplace_back(x, y);
            }
            m_parts.push_back(std::move(p));
            };

        // --- ORTA HAT (Gvde) ---
        // n Tampon (Kavisli)
        Add(_T("ON_TAMPON"), _T(""), _T("n Tampon"),
            { 200,60, 800,60, 820,120, 180,120 });

        // Kaput (Trapez)
        Add(_T("KAPUT"), _T("K"), _T("Motor Kaputu"),
            { 180,125, 820,125, 780,330, 220,330 });

        // Tavan (Dikdrtgen)
        Add(_T("TAVAN"), _T("T"), _T("Tavan"),
            { 240,380, 760,380, 760,630, 240,630 });

        // Bagaj (Trapez)
        Add(_T("BAGAJ"), _T("B"), _T("Bagaj Kapa"),
            { 230,680, 770,680, 800,860, 200,860 });

        // Arka Tampon (Kavisli)
        Add(_T("ARKA_TAMPON"), _T(""), _T("Arka Tampon"),
            { 200,865, 800,865, 820,930, 180,930 });

        // --- SOL TARAF ---
        // Sol n amurluk
        Add(_T("SOL_ON_CAM"), _T("S.."), _T("Sol n amurluk"),
            { 175,125, 215,330, 100,330, 120,135 });

        // Sol n Kap
        Add(_T("SOL_ON_KAPI"), _T("S..K"), _T("Sol n Kap"),
            { 95,335, 235,360, 235,500, 95,500 });

        // Sol Arka Kap
        Add(_T("SOL_ARKA_KAPI"), _T("S.A.K"), _T("Sol Arka Kap"),
            { 95,505, 235,505, 235,650, 95,650 });

        // Sol Arka amurluk
        Add(_T("SOL_ARKA_CAM"), _T("S.A."), _T("Sol Arka amurluk"),
            { 95,655, 225,660, 195,860, 120,850 });

        // Sol Marpiyel (nce d erit)
        Add(_T("SOL_MARSPIYEL"), _T(""), _T("Sol Marpiyel"),
            { 60,340, 90,340, 90,645, 60,645 });

        // Sol Podye/Direk ( ince erit)
        Add(_T("SOL_DIREK"), _T(""), _T("Sol Direk/Podye"),
            { 240,380, 260,380, 260,630, 240,630 });


        // --- SA TARAF ---
        // Sa n amurluk
        Add(_T("SAG_ON_CAM"), _T("S.."), _T("Sa n amurluk"),
            { 825,125, 880,135, 900,330, 785,330 });

        // Sa n Kap
        Add(_T("SAG_ON_KAPI"), _T("S..K"), _T("Sa n Kap"),
            { 905,335, 765,360, 765,500, 905,500 });

        // Sa Arka Kap
        Add(_T("SAG_ARKA_KAPI"), _T("S.A.K"), _T("Sa Arka Kap"),
            { 905,505, 765,505, 765,650, 905,650 });

        // Sa Arka amurluk
        Add(_T("SAG_ARKA_CAM"), _T("S.A."), _T("Sa Arka amurluk"),
            { 905,655, 775,660, 805,860, 880,850 });

        // Sa Marpiyel
        Add(_T("SAG_MARSPIYEL"), _T(""), _T("Sa Marpiyel"),
            { 910,340, 940,340, 940,645, 910,645 });

        // Sa Podye/Direk
        Add(_T("SAG_DIREK"), _T(""), _T("Sa Direk/Podye"),
            { 740,380, 760,380, 760,630, 740,630 });
    }

    // --- Renk Paleti ---
    Gdiplus::Color GetGdiColor(int st, bool isFill) const
    {
        using namespace Gdiplus;
        // Dolgu renkleri (hafif effaf) / Kenar renkleri (tam mat)
        int alpha = isFill ? 210 : 255;

        switch (st)
        {
        case CPS_LOKAL_BOYA: return Color(alpha, 255, 165, 0);   // Turuncu
        case CPS_BOYALI:     return Color(alpha, 65, 105, 225);  // Royal Blue
        case CPS_DEGISEN:    return Color(alpha, 220, 20, 60);   // Crimson Krmz
        case CPS_SOKTAK:     return Color(alpha, 30, 144, 255);  // Dodger Blue
        case CPS_PLASTIK:    return Color(alpha, 169, 169, 169); // Gri
        case CPS_EZIK_CIZIK: return Color(alpha, 138, 43, 226);  // Mor
        case CPS_ORJINAL:
        default:
            return isFill ? Color(255, 245, 245, 245) : Color(255, 120, 120, 120); // Kirli Beyaz / Koyu Gri Border
        }
    }

    CString GetShortCode(int st) const
    {
        switch (st) {
        case CPS_LOKAL_BOYA: return _T("L");
        case CPS_BOYALI: return _T("B");
        case CPS_DEGISEN: return _T("D");
        case CPS_SOKTAK: return _T("S");
        case CPS_PLASTIK: return _T("P");
        default: return _T("");
        }
    }

    CString GetStatusText(int st) const
    {
        switch (st) {
        case CPS_ORJINAL: return _T("Orjinal");
        case CPS_LOKAL_BOYA: return _T("Lokal Boyal");
        case CPS_BOYALI: return _T("Boyal");
        case CPS_DEGISEN: return _T("Deien");
        case CPS_SOKTAK: return _T("Sk-Tak");
        case CPS_PLASTIK: return _T("Plastik Aksam");
        case CPS_EZIK_CIZIK: return _T("Ezik / izik");
        default: return _T("");
        }
    }

    // --- Layout Hesaplama ---
    void CalcLayout(const CRect& rc)
    {
        m_rcClient = rc;
        const int padding = 10;

        // 1. Alt Bilgi (Legend) Alan
        m_rcLegend = rc;
        m_rcLegend.top = m_rcLegend.bottom - 40; // Alttan 40px

        // 2. Ana Gvde
        CRect rcBody = rc;
        rcBody.bottom = m_rcLegend.top - padding;
        rcBody.DeflateRect(padding, padding);

        // 3. Sa Liste (Responsive Genilik)
        int listWidth = std::min(300, rcBody.Width() / 3);
        m_rcRight = rcBody;
        m_rcRight.left = m_rcRight.right - listWidth;

        // 4. Araba izim Alan (Kalan sol taraf)
        m_rcCar = rcBody;
        m_rcCar.right = m_rcRight.left - padding;

        // Arabay ortala ve orann koru (Kareye yakn dikey dikdrtgen)
        int w = m_rcCar.Width();
        int h = m_rcCar.Height();
        int carSize = std::min(w, h);

        int offX = (w - carSize) / 2;
        int offY = (h - carSize) / 2;

        m_rcCar.SetRect(m_rcCar.left + offX, m_rcCar.top + offY,
            m_rcCar.left + offX + carSize, m_rcCar.top + offY + carSize);
    }

    // GDI Region'lar gncelle (Tklama alglama iin)
    void UpdateHitRegions()
    {
        for (auto& p : m_parts) p.Destroy();

        for (auto& p : m_parts)
        {
            std::vector<POINT> poly;
            for (const auto& pt : p.pts1000)
            {
                POINT np;
                np.x = m_rcCar.left + (pt.X * m_rcCar.Width() / 1000);
                np.y = m_rcCar.top + (pt.Y * m_rcCar.Height() / 1000);
                poly.push_back(np);
            }
            if (!poly.empty())
                p.hRgn = ::CreatePolygonRgn(poly.data(), (int)poly.size(), WINDING);
        }
    }

    // --- Etkileim ---
    void OnLButtonDown(int x, int y)
    {
        POINT pt{ x, y };
        for (auto& p : m_parts)
        {
            if (p.hRgn && ::PtInRegion(p.hRgn, pt.x, pt.y))
            {
                // Durum dngs
                p.state++;
                if (p.state > 6) p.state = 0;

                Invalidate(); // Yeniden iz
                break;
            }
        }
    }

    void Invalidate()
    {
        if (m_hWnd) ::InvalidateRect(m_hWnd, nullptr, FALSE);
    }

    // --- izim Fonksiyonlar ---

    void DrawCar(Gdiplus::Graphics& g)
    {
        using namespace Gdiplus;

        // Anti-aliasing a
        g.SetSmoothingMode(SmoothingModeHighQuality);

        // Fontlar
        FontFamily ff(L"Segoe UI");
        Gdiplus::Font fontLabel(&ff, 14.0f, FontStyleBold, UnitPixel);
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);

        for (const auto& p : m_parts)
        {
            // Koordinatlar ekrana evir
            std::vector<Point> pts;
            for (const auto& pt : p.pts1000)
            {
                int px = m_rcCar.left + (pt.X * m_rcCar.Width() / 1000);
                int py = m_rcCar.top + (pt.Y * m_rcCar.Height() / 1000);
                pts.emplace_back(px, py);
            }

            // 1. Dolgu
            Color colorFill = GetGdiColor(p.state, true);
            SolidBrush brFill(colorFill);
            g.FillPolygon(&brFill, pts.data(), (INT)pts.size());

            // 2. Kenarlk
            Color colorStroke = GetGdiColor(p.state, false);
            // Seiliyse kenarlk kalnlasn
            float penWidth = (p.state != CPS_ORJINAL) ? 2.0f : 1.0f;
            Pen penStroke(colorStroke, penWidth);
            g.DrawPolygon(&penStroke, pts.data(), (INT)pts.size());

            // 3. Etiket (Sadece seiliyse veya byk paraysa)
            if (p.state != CPS_ORJINAL && !p.labelUI.IsEmpty())
            {
                // Blgenin merkezini bul (basit bounding box merkezi)
                int minX = 99999, maxX = -99999, minY = 99999, maxY = -99999;
                for (auto& pt : pts) {
                    if (pt.X < minX) minX = pt.X; if (pt.X > maxX) maxX = pt.X;
                    if (pt.Y < minY) minY = pt.Y; if (pt.Y > maxY) maxY = pt.Y;
                }
                RectF rTxt((REAL)minX, (REAL)minY, (REAL)(maxX - minX), (REAL)(maxY - minY));

                // Durum Kodu (B, D, L vs.)
                CString code = GetShortCode(p.state);
                std::wstring wCode = code.c_str(); // CString -> wstring

                SolidBrush brText(Color::White);
                g.DrawString(wCode.c_str(), -1, &fontLabel, rTxt, &sf, &brText);
            }
        }
    }

    void DrawLegend(Gdiplus::Graphics& g)
    {
        using namespace Gdiplus;
        FontFamily ff(L"Segoe UI");
        Gdiplus::Font font(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush brText(Color::Black);

        // Alt tarafa yan yana kutucuklar iz
        int x = m_rcLegend.left + 10;
        int y = m_rcLegend.top + 10;
        int boxSize = 14;

        int states[] = { CPS_ORJINAL, CPS_LOKAL_BOYA, CPS_BOYALI, CPS_DEGISEN, CPS_SOKTAK, CPS_PLASTIK, CPS_EZIK_CIZIK };

        for (int st : states)
        {
            SolidBrush brBox(GetGdiColor(st, true));
            Pen penBox(Color::Gray, 1.0f);

            g.FillRectangle(&brBox, x, y, boxSize, boxSize);
            g.DrawRectangle(&penBox, x, y, boxSize, boxSize);

            std::wstring txt = GetStatusText(st).c_str();
            g.DrawString(txt.c_str(), -1, &font, PointF((REAL)(x + boxSize + 5), (REAL)y), &brText);

            x += 90; // Sonraki eye kaydr
        }
    }

    void DrawRightList(Gdiplus::Graphics& g)
    {
        using namespace Gdiplus;
        FontFamily ff(L"Segoe UI");
        Gdiplus::Font fontHeader(&ff, 12.0f, FontStyleBold, UnitPixel);
        Gdiplus::Font fontItem(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush brText(Color::Black);
        SolidBrush brDim(Color::Gray);

        int x = m_rcRight.left + 10;
        int y = m_rcRight.top + 10;

        // Balk
        g.DrawString(L"Hasar Kayd zeti", -1, &fontHeader, PointF((REAL)x, (REAL)y), &brText);
        y += 25;

        // Listeleme (Sadece hasarllar yaz)
        bool hasDamage = false;
        for (const auto& p : m_parts)
        {
            if (p.state == CPS_ORJINAL) continue;
            hasDamage = true;

            CString line;
            line.Format(_T("%s: %s"), p.labelList.c_str(), GetStatusText(p.state).c_str());
            std::wstring wLine = line.c_str();

            // Renkli nokta
            SolidBrush brDot(GetGdiColor(p.state, true));
            g.FillEllipse(&brDot, x, y + 3, 8, 8);

            g.DrawString(wLine.c_str(), -1, &fontItem, PointF((REAL)(x + 15), (REAL)y), &brText);
            y += 20;
        }

        if (!hasDamage)
        {
            g.DrawString(L"Hatasz / Boyasz", -1, &fontItem, PointF((REAL)x, (REAL)y), &brDim);
        }
    }

    void OnPaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(m_hWnd, &ps);
        RECT rc; ::GetClientRect(m_hWnd, &rc);

        // Double Buffering
        HDC hMem = ::CreateCompatibleDC(hdc);
        HBITMAP hBmp = ::CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOld = (HBITMAP)::SelectObject(hMem, hBmp);

        // Arkaplan Temizle
        ::FillRect(hMem, &rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

        // Hesaplamalar
        CalcLayout(CRect(rc));
        UpdateHitRegions();

        // GDI+ izim Balat
        {
            Gdiplus::Graphics g(hMem);
            DrawCar(g);
            DrawLegend(g);
            DrawRightList(g);
        }

        // Ekrana Aktar
        ::BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMem, 0, 0, SRCCOPY);

        ::SelectObject(hMem, hOld);
        ::DeleteObject(hBmp);
        ::DeleteDC(hMem);
        ::EndPaint(m_hWnd, &ps);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        CCarPaintCanvas* self = (CCarPaintCanvas*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (msg == WM_NCCREATE) {
            auto cs = (CREATESTRUCT*)lParam;
            self = (CCarPaintCanvas*)cs->lpCreateParams;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
            self->m_hWnd = hWnd;
        }

        if (self) {
            switch (msg) {
            case WM_PAINT: self->OnPaint(); return 0;
            case WM_LBUTTONDOWN: self->OnLButtonDown(LOWORD(lParam), HIWORD(lParam)); return 0;
            case WM_SIZE: self->Invalidate(); return 0;
            case WM_ERASEBKGND: return 1;
            }
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }
};

// ---------------------------------------------------------------------------
// CLASS: CCarPage (Tab Sayfalar iin Base Class)
// ---------------------------------------------------------------------------
class CCarPage : public Win32xx::CDialog
{
public:
    CCarPage(UINT nID) : CDialog(nID) {}
    virtual ~CCarPage() override = default;
protected:
    virtual void OnOK() override {}
    virtual void OnCancel() override {}
    virtual BOOL OnInitDialog() override {
        EnableThemeDialogTexture(GetHwnd(), ETDT_ENABLETAB);
        return TRUE;
    }
};

// ---------------------------------------------------------------------------
// CLASS: CCarEkspertizPage (Ekspertiz Sekmesi)
// ---------------------------------------------------------------------------
class CCarEkspertizPage : public CCarPage
{
public:
    CCarEkspertizPage(UINT nID) : CCarPage(nID) {}

    CString GetJsonData() const { return m_canvas.BuildJson(); }
    void SetJsonData(const CString& json) { m_canvas.LoadFromJson(json); }
    void ClearData() { m_canvas.ClearAll(); }

protected:
    CCarPaintCanvas m_canvas;

    virtual BOOL OnInitDialog() override
    {
        CCarPage::OnInitDialog();

        HWND hPlace = ::GetDlgItem(GetHwnd(), IDC_CAR_PAINT_CANVAS);
        if (hPlace)
        {
            CRect rc; ::GetWindowRect(hPlace, &rc);
            ::MapWindowPoints(nullptr, GetHwnd(), (LPPOINT)&rc, 2);
            ::ShowWindow(hPlace, SW_HIDE);

            m_canvas.Create(GetHwnd(), rc, IDC_CAR_PAINT_CANVAS);
        }
        return TRUE;
    }

    virtual INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override
    {
        if (msg == WM_COMMAND) {
            if (LOWORD(wp) == IDC_BTN_CAR_PAINT_CLEAR && HIWORD(wp) == BN_CLICKED) {
                ClearData();
                return TRUE;
            }
        }
        return CCarPage::DialogProc(msg, wp, lp);
    }
};

// ---------------------------------------------------------------------------
// CLASS: CCarDialog (Ana Pencere)
// ---------------------------------------------------------------------------
class CCarDialog : public Win32xx::CDialog
{
public:
    CCarDialog(DatabaseManager& db, int mode, const CString& cariKod, const CString& carCode = _T(""))
        : CDialog(IDD_CAR_DIALOG), m_db(db), m_mode(mode), m_cariKod(cariKod), m_carCode(carCode),
        m_pageGenel(IDD_PAGE_GENEL), m_pageTeknik(IDD_PAGE_TEKNIK), m_pageIlan(IDD_PAGE_ILAN),
        m_pageNotlar(IDD_PAGE_NOTLAR), m_pageAciklama(IDD_PAGE_ACIKLAMA), m_pageDonanim(IDD_PAGE_DONANIM),
        m_pageEkspertiz(IDD_PAGE_EKSPERTIZ)
    {
    }

protected:
    DatabaseManager& m_db;
    int m_mode;
    CString m_cariKod, m_carCode;
    Win32xx::CFont m_font;
    Win32xx::CTab m_tab;

    CCarPage m_pageGenel, m_pageTeknik, m_pageIlan, m_pageNotlar, m_pageAciklama, m_pageDonanim;
    CCarEkspertizPage m_pageEkspertiz;

    virtual BOOL OnInitDialog() override
    {
        LOGFONT lf{}; SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
        _tcscpy_s(lf.lfFaceName, _T("Segoe UI")); lf.lfHeight = -MulDiv(9, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72);
        m_font.CreateFontIndirect(lf); SetFont(m_font, TRUE);
        EnableThemeDialogTexture(GetHwnd(), ETDT_ENABLETAB);

        m_tab.AttachDlgItem(IDC_TAB_CAR, *this); m_tab.SetFont(m_font, TRUE);
        auto AddTab = [&](int i, LPCTSTR t) { TCITEM ti{ TCIF_TEXT, 0, 0, (LPTSTR)t }; m_tab.InsertItem(i, &ti); };
        AddTab(0, _T("Genel Bilgiler")); AddTab(1, _T("Fiyat ve Teknik")); AddTab(2, _T("lan Detaylar"));
        AddTab(3, _T("Notlar")); AddTab(4, _T("Aklama")); AddTab(5, _T("Donanm")); AddTab(6, _T("Ekspertiz"));

        CCarPage* pages[] = { &m_pageGenel, &m_pageTeknik, &m_pageIlan, &m_pageNotlar, &m_pageAciklama, &m_pageDonanim, &m_pageEkspertiz };
        for (auto p : pages) { p->Create(m_tab); p->SetFont(m_font, TRUE); }

        RecalcLayout(); ShowPage(0);

        if (m_mode == INEWUSER) {
            m_carCode = m_db.GenerateNextCarCode();
            m_pageGenel.SetDlgItemText(IDC_EDIT_CAR_CODE, m_carCode);
        }
        else {
            m_pageGenel.SetDlgItemText(IDC_EDIT_CAR_CODE, m_carCode);
            LoadDataFromDB();
        }
        return TRUE;
    }

    virtual INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override
    {
        if (msg == WM_NOTIFY) {
            LPNMHDR pnm = (LPNMHDR)lp;
            if (pnm->idFrom == IDC_TAB_CAR && pnm->code == TCN_SELCHANGE) {
                ShowPage(m_tab.GetCurSel()); return TRUE;
            }
        }
        return DialogProcDefault(msg, wp, lp);
    }

    virtual void OnOK() override
    {
        std::map<CString, CString> uiData;
        auto Col = [&](CWnd* p) { m_db.Bind_UI_To_Data(p, TABLE_NAME_CAR, uiData); };

        Col(&m_pageGenel); Col(&m_pageTeknik); Col(&m_pageIlan);
        Col(&m_pageNotlar); Col(&m_pageAciklama); Col(&m_pageDonanim);
        uiData[_T("PaintInfo")] = m_pageEkspertiz.GetJsonData();

        uiData[_T("Cari_Kod")] = m_cariKod;
        uiData[_T("Car_Code")] = m_carCode;
        uiData[_T("Updated_At")] = m_db.GetCurrentIsoUtc();

        Car_cstr c;
        for (const auto& kv : uiData) DatabaseManager::SetFieldByStringName(c, kv.first, kv.second);

        if (m_mode == INEWUSER) m_db.InsertGlobal(c); else m_db.UpdateGlobal(c);
        CDialog::OnOK();
    }

    void RecalcLayout()
    {
        if (!m_tab.IsWindow()) return;
        CRect rc = m_tab.GetClientRect(); m_tab.AdjustRect(FALSE, rc);
        CCarPage* pages[] = { &m_pageGenel, &m_pageTeknik, &m_pageIlan, &m_pageNotlar, &m_pageAciklama, &m_pageDonanim, &m_pageEkspertiz };
        for (auto p : pages) p->MoveWindow(rc);
    }

    void ShowPage(int page)
    {
        CCarPage* pages[] = { &m_pageGenel, &m_pageTeknik, &m_pageIlan, &m_pageNotlar, &m_pageAciklama, &m_pageDonanim, &m_pageEkspertiz };
        for (int i = 0; i < 7; i++) pages[i]->ShowWindow(i == page ? SW_SHOW : SW_HIDE);
    }

    void LoadDataFromDB()
    {
        auto map = m_db.FetchRecordMap(TABLE_NAME_CAR, _T("Car_Code"), m_carCode);
        if (map.empty()) return;

        m_db.Bind_Data_To_UI(&m_pageGenel, TABLE_NAME_CAR, map);
        m_db.Bind_Data_To_UI(&m_pageTeknik, TABLE_NAME_CAR, map);
        m_db.Bind_Data_To_UI(&m_pageIlan, TABLE_NAME_CAR, map);
        m_db.Bind_Data_To_UI(&m_pageNotlar, TABLE_NAME_CAR, map);
        m_db.Bind_Data_To_UI(&m_pageAciklama, TABLE_NAME_CAR, map);
        m_db.Bind_Data_To_UI(&m_pageDonanim, TABLE_NAME_CAR, map);

        if (map.count(_T("PaintInfo")))
            m_pageEkspertiz.SetJsonData(map[_T("PaintInfo")]);
    }
};