#include "stdafx.h"
#include "MatrixMenu.h"
#include "resource.h"
#include <cmath>

#pragma comment (lib,"Gdiplus.lib")

// Statik Üyeler
ULONG_PTR CMatrixMenu::g_gdiplusToken = 0;
int CMatrixMenu::g_gdiplusRefCount = 0;

CMatrixMenu::CMatrixMenu() : 
    m_cols(3), 
    m_itemSize(100), 
    m_gap(10), 
    m_cornerRadius(15), 
    m_headerHeight(40),
    m_fontName(_T("Segoe UI")), 
    m_fontSize(12.0f),
    m_animAlpha(0.0f),
    m_isClosing(false),
    m_hParent(NULL)
{
    // Varsayılan Renkler (Modern Dark/Gray Tema)
    m_cBg = Gdiplus::Color(240, 30, 30, 30);
    m_cItemBg = Gdiplus::Color(255, 50, 50, 55);
    m_cItemHover = Gdiplus::Color(255, 0, 120, 215); // Windows Mavisi
    m_cText = Gdiplus::Color(255, 255, 255, 255);

    m_pCurrentItems = &m_rootItems; // Başlangıçta kök dizindeyiz

    if (g_gdiplusRefCount++ == 0) {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&g_gdiplusToken, &input, NULL);
    }
}

CMatrixMenu::~CMatrixMenu() {
    if (--g_gdiplusRefCount == 0) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
    }
}

void CMatrixMenu::PreRegisterClass(WNDCLASS& wc) {
    CWnd::PreRegisterClass(wc);
    wc.lpszClassName = _T("MatrixMenuClass");
    wc.hbrBackground = NULL;
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
}

// --- YAPILANDIRMA ---
CMatrixMenu& CMatrixMenu::SetGrid(int cols, int itemSize, int gap) {
    m_cols = cols; m_itemSize = itemSize; m_gap = gap; return *this;
}
CMatrixMenu& CMatrixMenu::SetColors(COLORREF bg, COLORREF itemBg, COLORREF itemHover, COLORREF text) {
    m_cBg.SetFromCOLORREF(bg); m_cBg = Gdiplus::Color(240, m_cBg.GetR(), m_cBg.GetG(), m_cBg.GetB());
    m_cItemBg.SetFromCOLORREF(itemBg); m_cItemBg = Gdiplus::Color(255, m_cItemBg.GetR(), m_cItemBg.GetG(), m_cItemBg.GetB());
    m_cItemHover.SetFromCOLORREF(itemHover); m_cItemHover = Gdiplus::Color(255, m_cItemHover.GetR(), m_cItemHover.GetG(), m_cItemHover.GetB());
    m_cText.SetFromCOLORREF(text); m_cText = Gdiplus::Color(255, m_cText.GetR(), m_cText.GetG(), m_cText.GetB());
    return *this;
}
CMatrixMenu& CMatrixMenu::SetFont(const CString& fontName, float fontSize) {
    m_fontName = fontName; m_fontSize = fontSize; return *this;
}
CMatrixMenu& CMatrixMenu::SetCornerRadius(int radius) {
    m_cornerRadius = radius; return *this;
}

// --- VERİ EKLEME ---
void CMatrixMenu::AddItem(UINT cmdID, LPCTSTR text, UINT iconID) {
    MatrixItem item;
    item.cmdID = cmdID;
    item.text = text;
    item.iconID = iconID;
    m_rootItems.push_back(item);
}

void CMatrixMenu::AddSubItem(UINT parentCmdID, UINT cmdID, LPCTSTR text, UINT iconID) {
    // Basit bir recursive arama ile parent'ı bul
    std::function<bool(std::vector<MatrixItem>&)> finder = 
        [&](std::vector<MatrixItem>& list) -> bool {
        for (auto& item : list) {
            if (item.cmdID == parentCmdID) {
                MatrixItem sub;
                sub.cmdID = cmdID; sub.text = text; sub.iconID = iconID;
                item.subItems.push_back(sub);
                return true;
            }
            if (!item.subItems.empty()) {
                if (finder(item.subItems)) return true;
            }
        }
        return false;
    };
    finder(m_rootItems);
}

// --- GÖSTER ---
void CMatrixMenu::Show(POINT screenPt, HWND hParent) {
    m_hParent = hParent;
    m_animAlpha = 0.0f;
    m_isClosing = false;
    m_pCurrentItems = &m_rootItems; // Reset
    while(!m_history.empty()) m_history.pop(); // Reset history

    // Pencere Boyutu Hesapla
    // Genişlik = (Sütun * Boyut) + ((Sütun+1) * Boşluk)
    // Yükseklik = Header + (Satır * Boyut) + ((Satır+1) * Boşluk)
    // Başlangıçta Max 3 satır gibi düşünelim, dinamik çizimde sığdıracağız.
    
    int totalWidth = (m_cols * m_itemSize) + ((m_cols + 1) * m_gap);
    // Yükseklik şimdilik tahmini, Render'da dinamik olsa iyi olur ama sabit verelim
    // Maksimum 4 satırlık yer ayıralım
    int totalHeight = m_headerHeight + (4 * m_itemSize) + (5 * m_gap);

    m_wndSize.cx = totalWidth;
    m_wndSize.cy = totalHeight;
    m_screenPos = screenPt;

    // Ekrandan taşmayı engelle
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (m_screenPos.x + m_wndSize.cx > screenW) m_screenPos.x = screenW - m_wndSize.cx;
    if (m_screenPos.y + m_wndSize.cy > screenH) m_screenPos.y = screenH - m_wndSize.cy;

    CreateEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
             NULL, _T("MatrixMenu"), WS_POPUP, 
             m_screenPos.x, m_screenPos.y, m_wndSize.cx, m_wndSize.cy, hParent, NULL);

    RenderFrame();
    ShowWindow(SW_SHOW);
    SetTimer(1, 15, NULL); // Fade In Animasyonu
    SetFocus();
}

void CMatrixMenu::RenderFrame() {
    if (!IsWindow()) return;

    HDC hScreenDC = ::GetDC(NULL);
    HDC hMemDC = ::CreateCompatibleDC(hScreenDC);
    
    BITMAPINFO bmi = {0};
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

        // --- ARKA PLAN (Kavisli ve Yarı Saydam) ---
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color((BYTE)(m_animAlpha * m_cBg.GetA()), m_cBg.GetR(), m_cBg.GetG(), m_cBg.GetB()));
        Gdiplus::GraphicsPath path;
        CRect rc(0, 0, m_wndSize.cx, m_wndSize.cy);
        
        // Kavisli Dikdörtgen Yolu
        int d = 20; // Ana çerçeve kavisi
        path.AddArc(rc.left, rc.top, d, d, 180, 90);
        path.AddArc(rc.right - d, rc.top, d, d, 270, 90);
        path.AddArc(rc.right - d, rc.bottom - d, d, d, 0, 90);
        path.AddArc(rc.left, rc.bottom - d, d, d, 90, 90);
        path.CloseFigure();
        g.FillPath(&bgBrush, &path);

        // --- HEADER (GERİ BUTONU) ---
        DrawHeader(g);

        // --- MENÜ ÖĞELERİ ---
        int count = (int)m_pCurrentItems->size();
        for (int i = 0; i < count; ++i) {
            int col = i % m_cols;
            int row = i / m_cols;
            
            int x = m_gap + (col * (m_itemSize + m_gap));
            int y = m_headerHeight + m_gap + (row * (m_itemSize + m_gap));

            // Çizim alanını aşarsa çizme
            if (y + m_itemSize > m_wndSize.cy) break;

            DrawItem(g, (*m_pCurrentItems)[i], x, y);
        }
    }

    POINT ptSrc = {0, 0};
    BLENDFUNCTION blend = {0};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    ::UpdateLayeredWindow(*this, hScreenDC, &m_screenPos, &m_wndSize, hMemDC, &ptSrc, 0, &blend, ULW_ALPHA);

    ::SelectObject(hMemDC, hOldBitmap);
    ::DeleteObject(hBitmap);
    ::DeleteDC(hMemDC);
    ::ReleaseDC(NULL, hScreenDC);
}

void CMatrixMenu::DrawHeader(Gdiplus::Graphics& g) {
    if (m_history.empty()) {
        // Ana Menüdeysek sadece başlık veya boş
        Gdiplus::SolidBrush textBrush(m_cText);
        Gdiplus::Font font(m_fontName, m_fontSize * 1.2f, Gdiplus::FontStyleBold);
        g.DrawString(L"Ana Menü", -1, &font, Gdiplus::PointF((float)m_gap, (float)m_gap), NULL);
        return;
    }

    // Geri Butonu Çiz (< Geri)
    Gdiplus::RectF btnRect(0, 0, (float)m_wndSize.cx, (float)m_headerHeight);
    
    // Hover efekti için basit kontrol (Mouse takibi burada yapılabilir ama basit tutalım)
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(50, 255, 255, 255)); // Hafif beyaz
    g.FillRectangle(&bgBrush, btnRect);

    Gdiplus::SolidBrush textBrush(m_cText);
    Gdiplus::Font font(m_fontName, m_fontSize, Gdiplus::FontStyleBold);
    
    // Geri Oku ve Yazısı
// Hatalı Satır:
// CString title = m_titleHistory.empty() ? _T("Geri") : m_titleHistory.top();

// ✅ DÜZELTİLMİŞ HALİ:
    CString title = m_titleHistory.empty() ? CString(_T("Geri")) : m_titleHistory.top();    CString btnText = _T("🡠 ") + title; // Unicode ok
    
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    
    g.DrawString(btnText, -1, &font, btnRect, &format, &textBrush);
}

void CMatrixMenu::DrawItem(Gdiplus::Graphics& g, const MatrixItem& item, int x, int y) {
    Gdiplus::RectF rect((float)x, (float)y, (float)m_itemSize, (float)m_itemSize);
    
    // Arka Plan (Hover ise renk değişir)
    Gdiplus::SolidBrush brush(item.hovered ? m_cItemHover : m_cItemBg);
    
    // Yuvarlak Köşeli Kare
    Gdiplus::GraphicsPath path;
    float fd = (float)(m_cornerRadius * 2); // 'd' değerini float yapıyoruz

    // Tüm parametrelerin float olduğundan emin oluyoruz
    path.AddArc(rect.X, rect.Y, fd, fd, 180.0f, 90.0f);
    path.AddArc(rect.X + rect.Width - fd, rect.Y, fd, fd, 270.0f, 90.0f);
    path.AddArc(rect.X + rect.Width - fd, rect.Y + rect.Height - fd, fd, fd, 0.0f, 90.0f);
    path.AddArc(rect.X, rect.Y + rect.Height - fd, fd, fd, 90.0f, 90.0f);

    path.CloseFigure();
    
    g.FillPath(&brush, &path);

    // İkon Çiz (Varsa)
    if (item.iconID > 0) {
        // Basit bitmap yükleme (GDI+ Bitmap)
        // (Gerçek projede HICON -> Gdiplus::Bitmap dönüşümü eklenmeli)
        // Şimdilik temsili daire
        /*Gdiplus::SolidBrush iconBrush(Gdiplus::Color(100, 255, 255, 255));
        g.FillEllipse(&iconBrush, x + m_itemSize/2 - 10, y + 20, 20, 20);*/
    }

    // Metin Çiz
    Gdiplus::SolidBrush textBrush(m_cText);
    Gdiplus::Font font(m_fontName, m_fontSize, Gdiplus::FontStyleRegular);
    
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentFar); // Altta

    Gdiplus::RectF textRect(rect.X + 5, rect.Y, rect.Width - 10, rect.Height - 10);
    g.DrawString(item.text, -1, &font, textRect, &format, &textBrush);
    
    // Alt menü işareti
    if (!item.subItems.empty()) {
        Gdiplus::Font arrowFont(m_fontName, 8, Gdiplus::FontStyleRegular);
        Gdiplus::StringFormat arrowFmt;
        arrowFmt.SetAlignment(Gdiplus::StringAlignmentFar);
        g.DrawString(L"▶", -1, &arrowFont, Gdiplus::PointF(rect.GetRight()-5, rect.GetTop()+5), &arrowFmt, &textBrush);
    }
}

int CMatrixMenu::GetItemIndexAt(POINT pt) {
    // Header alanı hariç
    if (pt.y < m_headerHeight) return -1;

    int relY = pt.y - m_headerHeight - m_gap;
    if (relY < 0) return -1;

    int row = relY / (m_itemSize + m_gap);
    int col = (pt.x - m_gap) / (m_itemSize + m_gap);

    // Sınır kontrolleri
    if (col < 0 || col >= m_cols) return -1;
    
    int index = (row * m_cols) + col;
    if (index >= 0 && index < (int)m_pCurrentItems->size()) {
        return index;
    }
    return -1;
}

bool CMatrixMenu::IsBackButtonAt(POINT pt) {
    return (!m_history.empty() && pt.y >= 0 && pt.y < m_headerHeight);
}

LRESULT CMatrixMenu::WndProc(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_TIMER:
        if (wparam == 1) { // Fade In/Out
            if (!m_isClosing) {
                if (m_animAlpha < 1.0f) {
                    m_animAlpha += 0.1f;
                    if (m_animAlpha > 1.0f) m_animAlpha = 1.0f;
                    RenderFrame();
                } else KillTimer(1);
            } else {
                if (m_animAlpha > 0.0f) {
                    m_animAlpha -= 0.2f;
                    RenderFrame();
                } else {
                    KillTimer(1);
                    Destroy();
                }
            }
        }
        return 0;

    case WM_MOUSEMOVE: {
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        int idx = GetItemIndexAt(pt);
        bool changed = false;
        
        for (size_t i = 0; i < m_pCurrentItems->size(); ++i) {
            if ((*m_pCurrentItems)[i].hovered != (i == idx)) {
                (*m_pCurrentItems)[i].hovered = (i == idx);
                changed = true;
            }
        }
        if (changed) RenderFrame();
        return 0;
    }

    case WM_LBUTTONUP: {
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        
        // Geri Butonu Kontrolü
        if (IsBackButtonAt(pt)) {
            if (!m_history.empty()) {
                m_pCurrentItems = m_history.top(); m_history.pop();
                m_titleHistory.pop();
                RenderFrame(); // Sayfa değişti, yeniden çiz
            }
            return 0;
        }

        // Öğe Seçimi
        int idx = GetItemIndexAt(pt);
        if (idx != -1) {
            auto& item = (*m_pCurrentItems)[idx];
            
            // 1. Alt Menü varsa oraya dal
            if (!item.subItems.empty()) {
                m_history.push(m_pCurrentItems); // Mevcut sayfayı kaydet
                m_titleHistory.push(item.text);  // Başlığı kaydet
                m_pCurrentItems = &item.subItems; // Yeni sayfaya geç
                RenderFrame();
            }
            // 2. Komut ise çalıştır ve kapat
            else if (item.cmdID != 0) {
                if (OnCommandSelected) OnCommandSelected(item.cmdID);
                m_isClosing = true; SetTimer(1, 15, NULL);
            }
        } else {
            // Boşa tıklandıysa kapatma, belki header'a basmıştır.
             if (pt.y > m_wndSize.cy) { // Pencere dışı
                 m_isClosing = true; SetTimer(1, 15, NULL);
             }
        }
        return 0;
    }

    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE) { m_isClosing = true; SetTimer(1, 15, NULL); }
        return 0;
    
    case WM_KILLFOCUS:
        m_isClosing = true; SetTimer(1, 15, NULL);
        return 0;
    }
    return WndProcDefault(msg, wparam, lparam);
}