#include "stdafx.h"
#include "RadialMenu.h"
#include "resource.h"
#include <cmath>

#pragma comment (lib,"Gdiplus.lib")

#define PI 3.14159265358979323846

// Statik Üyeler
ULONG_PTR CRadialMenu::g_gdiplusToken = 0;
int CRadialMenu::g_gdiplusRefCount = 0;

CRadialMenu::CRadialMenu() :
    m_currentRadius(0),
    m_targetRadius(100),    // İç Halka Çapı
    m_outerRadius(160),     // Dış Halka Çapı
    m_animStep(20),
    m_isClosing(false),
    m_hParent(NULL),
    m_hoveredMainIndex(-1),
    m_hoveredSubIndex(-1)
{
    if (g_gdiplusRefCount++ == 0) {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    }
}

CRadialMenu::~CRadialMenu()
{
    if (--g_gdiplusRefCount == 0) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
    }
}

void CRadialMenu::PreRegisterClass(WNDCLASS& wc)
{
    CWnd::PreRegisterClass(wc);
    wc.lpszClassName = _T("RadialMenuClass");
    wc.hbrBackground = NULL;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
}

void CRadialMenu::AddItem(UINT cmdID, LPCTSTR text, UINT iconID)
{
    RadialItem item;
    item.cmdID = cmdID;
    item.text = text;
    item.iconID = iconID;
    item.hovered = false;
    m_items.push_back(item);
}

void CRadialMenu::AddSubItem(UINT parentCmdID, UINT cmdID, LPCTSTR text, UINT iconID)
{
    for (auto& item : m_items) {
        if (item.cmdID == parentCmdID) {
            RadialItem sub;
            sub.cmdID = cmdID;
            sub.text = text;
            sub.iconID = iconID;
            sub.hovered = false;
            item.subItems.push_back(sub);
            return;
        }
    }
}

void CRadialMenu::Show(POINT screenPt, HWND hParent)
{
    m_hParent = hParent;
    m_currentRadius = 0;
    m_isClosing = false;
    m_hoveredMainIndex = -1;
    m_hoveredSubIndex = -1;

    // Pencere boyutunu maksimum genişliğe (Dış halka) göre ayarla
    int size = m_outerRadius * 2 + 50;
    m_wndSize.cx = size;
    m_wndSize.cy = size;

    m_screenPos.x = screenPt.x - size / 2;
    m_screenPos.y = screenPt.y - size / 2;

    CreateEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        NULL, _T("RadialMenu"), WS_POPUP,
        m_screenPos.x, m_screenPos.y, m_wndSize.cx, m_wndSize.cy, hParent, NULL);

    RenderFrame();
    ShowWindow(SW_SHOW);
    SetTimer(1, 15, NULL);
    SetFocus();
}

void CRadialMenu::RenderFrame()
{
    if (!IsWindow()) return;

    HDC hScreenDC = ::GetDC(NULL);
    HDC hMemDC = ::CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_wndSize.cx;
    bmi.bmiHeader.biHeight = m_wndSize.cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = NULL;
    HBITMAP hBitmap = ::CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);

    {
        Gdiplus::Graphics g(hMemDC);
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
        g.Clear(Gdiplus::Color(0, 0, 0, 0));

        DrawRadialItems(g, m_wndSize.cx / 2, m_wndSize.cy / 2);
    }

    POINT ptSrc = { 0, 0 };
    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    ::UpdateLayeredWindow(*this, hScreenDC, &m_screenPos, &m_wndSize, hMemDC, &ptSrc, 0, &blend, ULW_ALPHA);

    ::SelectObject(hMemDC, hOldBitmap);
    ::DeleteObject(hBitmap);
    ::DeleteDC(hMemDC);
    ::ReleaseDC(NULL, hScreenDC);
}

LRESULT CRadialMenu::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_TIMER:
        if (wparam == 1) {
            if (!m_isClosing) {
                if (m_currentRadius < m_targetRadius) {
                    m_currentRadius += m_animStep;
                    if (m_currentRadius > m_targetRadius) m_currentRadius = m_targetRadius;
                    RenderFrame();
                }
                else {
                    KillTimer(1);
                }
            }
            else {
                if (m_currentRadius > 0) {
                    m_currentRadius -= m_animStep;
                    RenderFrame();
                }
                else {
                    KillTimer(1);
                    Destroy();
                    return 0;
                }
            }
        }
        return 0;

    case WM_MOUSEMOVE:
    {
        POINT pt; GetCursorPos(&pt);

        // Yerel değişkenler kullanarak hesapla
        int newMain = -1;
        int newSub = -1;
        GetIndicesAt(pt, newMain, newSub);

        // Sadece değişiklik varsa yeniden çiz
        if (newMain != m_hoveredMainIndex || newSub != m_hoveredSubIndex) {
            m_hoveredMainIndex = newMain;
            m_hoveredSubIndex = newSub;
            RenderFrame();
        }
        return 0;
    }

    case WM_LBUTTONUP:
    {
        POINT pt; GetCursorPos(&pt);

        int mainIdx = -1;
        int subIdx = -1;
        GetIndicesAt(pt, mainIdx, subIdx);

        UINT cmdToExec = 0;

        if (mainIdx != -1) {
            // 1. Eğer Alt Menü seçildiyse onu çalıştır
            if (subIdx != -1) {
                if (subIdx < (int)m_items[mainIdx].subItems.size())
                    cmdToExec = m_items[mainIdx].subItems[subIdx].cmdID;
            }
            // 2. Eğer Ana Menü seçildiyse ve Alt Menüsü YOKSA çalıştır (Varsa çalıştırma, sadece menü açar)
            else if (m_items[mainIdx].subItems.empty()) {
                cmdToExec = m_items[mainIdx].cmdID;
            }
        }

        if (cmdToExec != 0) {
            if (OnCommandSelected) OnCommandSelected(cmdToExec);
            m_isClosing = true;
            SetTimer(1, 15, NULL);
        }
        else {
            // Eğer tamamen dışarıya veya boşluğa tıklandıysa kapat
            // Ama eğer alt menüsü olan bir ana başlığa tıklandıysa kapatma (bekle)
            if (mainIdx == -1) {
                m_isClosing = true;
                SetTimer(1, 15, NULL);
            }
        }
        return 0;
    }

    case WM_PAINT:
    { CPaintDC dc(*this); } return 0;

    case WM_RBUTTONDOWN:
    case WM_KILLFOCUS:
        if (!m_isClosing) { m_isClosing = true; SetTimer(1, 15, NULL); }
        return 0;

    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE) { m_isClosing = true; SetTimer(1, 15, NULL); }
        return 0;
    }

    return WndProcDefault(msg, wparam, lparam);
}

void CRadialMenu::DrawRadialItems(Gdiplus::Graphics& g, int cx, int cy)
{
    if (m_items.empty()) return;

    int count = (int)m_items.size();
    float angleStep = 360.0f / count;
    float startAngle = -90.0f;

    float innerR = m_currentRadius * 0.35f;
    float outerR = (float)m_currentRadius;

    // --- 1. ANA HALKA ---
    for (int i = 0; i < count; ++i)
    {
        float angle = startAngle + (i * angleStep);
        bool isHovered = (i == m_hoveredMainIndex);

        // Eğer alt menü açıldıysa ana menü "Seçili" gibi kalsın
        Gdiplus::Color fillColor = isHovered
            ? Gdiplus::Color(220, 0, 122, 204)  // Mavi (Hover/Aktif)
            : Gdiplus::Color(200, 45, 45, 48);  // Gri

        Gdiplus::SolidBrush brush(fillColor);
        Gdiplus::Pen pen(Gdiplus::Color(100, 255, 255, 255), 2.0f);

        Gdiplus::GraphicsPath path;
        path.AddArc(cx - outerR, cy - outerR, outerR * 2, outerR * 2, angle, angleStep);
        path.AddArc(cx - innerR, cy - innerR, innerR * 2, innerR * 2, angle + angleStep, -angleStep);
        path.CloseFigure();

        g.FillPath(&brush, &path);
        g.DrawPath(&pen, &path);

        // Metin
        float midAngle = angle + angleStep / 2.0f;
        float rad = midAngle * (PI / 180.0f);
        float textR = (innerR + outerR) / 2.0f;
        float tx = cx + textR * cos(rad);
        float ty = cy + textR * sin(rad);

        Gdiplus::FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font font(&fontFamily, 10, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255));
        Gdiplus::StringFormat format;
        format.SetAlignment(Gdiplus::StringAlignmentCenter);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        g.DrawString(m_items[i].text.c_str(), -1, &font, Gdiplus::PointF(tx, ty), &format, &textBrush);
    }

    // --- 2. DIŞ HALKA (ALT MENÜ) ---
    if (m_hoveredMainIndex != -1 && m_hoveredMainIndex < count)
    {
        const auto& parentItem = m_items[m_hoveredMainIndex];
        if (!parentItem.subItems.empty())
        {
            int subCount = (int)parentItem.subItems.size();
            float subStep = 360.0f / subCount; // Tam daireye yay

            // Dış halka parametreleri
            float subInnerR = outerR + 2.0f; // Hafif boşluk
            float subOuterR = (float)m_outerRadius; // Maksimum genişlik

            for (int j = 0; j < subCount; ++j)
            {
                float subAngle = startAngle + (j * subStep);
                bool isSubHover = (j == m_hoveredSubIndex);

                Gdiplus::Color subColor = isSubHover
                    ? Gdiplus::Color(240, 50, 200, 50)  // Yeşil (Hover)
                    : Gdiplus::Color(220, 70, 70, 70);  // Açık Gri

                Gdiplus::SolidBrush sBrush(subColor);
                Gdiplus::Pen sPen(Gdiplus::Color(100, 255, 255, 255), 1.5f);

                Gdiplus::GraphicsPath sPath;
                sPath.AddArc(cx - subOuterR, cy - subOuterR, subOuterR * 2, subOuterR * 2, subAngle, subStep);
                sPath.AddArc(cx - subInnerR, cy - subInnerR, subInnerR * 2, subInnerR * 2, subAngle + subStep, -subStep);
                sPath.CloseFigure();

                g.FillPath(&sBrush, &sPath);
                g.DrawPath(&sPen, &sPath);

                // Alt Metin
                float sMidAngle = subAngle + subStep / 2.0f;
                float sRad = sMidAngle * (PI / 180.0f);
                float sTextR = (subInnerR + subOuterR) / 2.0f;
                float sx = cx + sTextR * cos(sRad);
                float sy = cy + sTextR * sin(sRad);
                Gdiplus::FontFamily fontFamily(L"Segoe UI");
                Gdiplus::Font font(&fontFamily, 10, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
                Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255));
                Gdiplus::StringFormat format;

                Gdiplus::Font subFont(&fontFamily, 9, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
                Gdiplus::SolidBrush subTextBrush(Gdiplus::Color(255, 255, 255));
                g.DrawString(parentItem.subItems[j].text.c_str(), -1, &subFont, Gdiplus::PointF(sx, sy), &format, &subTextBrush);
            }
        }
    }

    // Kapat Düğmesi (Orta)
    Gdiplus::SolidBrush centerBrush(Gdiplus::Color(255, 200, 60, 60));
    g.FillEllipse(&centerBrush, cx - innerR + 5, cy - innerR + 5, (innerR - 5) * 2, (innerR - 5) * 2);
}

void CRadialMenu::GetIndicesAt(POINT screenPt, int& outMain, int& outSub)
{
    int resMain = -1;
    int resSub = -1;

    int cx = m_wndSize.cx / 2;
    int cy = m_wndSize.cy / 2;
    int localX = screenPt.x - m_screenPos.x;
    int localY = screenPt.y - m_screenPos.y;

    double dx = localX - cx;
    double dy = localY - cy;
    double dist = sqrt(dx * dx + dy * dy);

    double angle = atan2(dy, dx) * (180.0 / PI);
    if (angle < 0) angle += 360.0;
    angle += 90.0;
    if (angle >= 360.0) angle -= 360.0;

    // 1. İÇ HALKA (Ana Menü) KONTROLÜ
    if (dist >= m_currentRadius * 0.35f && dist <= m_currentRadius)
    {
        int count = (int)m_items.size();
        if (count > 0) {
            resMain = (int)(angle / (360.0f / count));
            if (resMain >= count) resMain = 0;
        }
        // Sub seçim yok
    }
    // 2. DIŞ HALKA (Alt Menü) KONTROLÜ
    else if (dist > m_currentRadius && dist <= m_outerRadius)
    {
        // 🔥 KRİTİK DÜZELTME: Burada "Şu anki" durumdan kopya çekiyoruz
        // Fare dışa kaydıysa, hangi ana menüdeydik? Onu koru.
        if (m_hoveredMainIndex != -1 && m_hoveredMainIndex < (int)m_items.size())
        {
            const auto& item = m_items[m_hoveredMainIndex];
            if (!item.subItems.empty())
            {
                resMain = m_hoveredMainIndex; // ANA SEÇİMİ KORU!

                // Alt menü indeksini hesapla
                int subCount = (int)item.subItems.size();
                resSub = (int)(angle / (360.0f / subCount));
                if (resSub >= subCount) resSub = 0;
            }
        }
    }

    outMain = resMain;
    outSub = resSub;
}