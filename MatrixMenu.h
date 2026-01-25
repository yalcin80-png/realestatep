#pragma once
#include "stdafx.h"
#include <vector>
#include <stack>
#include <functional>
#include <gdiplus.h>

// Menü Öğesi
struct MatrixItem {
    UINT cmdID;           // Komut ID (0 ise sadece klasördür)
    CString text;         // Görünen İsim
    UINT iconID;          // İkon Kaynağı
    std::vector<MatrixItem> subItems; // Alt Menüler
    bool hovered;         // Fare üzerinde mi?
    
    MatrixItem() : cmdID(0), iconID(0), hovered(false) {}
};

class CMatrixMenu : public CWnd
{
public:
    CMatrixMenu();
    virtual ~CMatrixMenu();

    // Kopyalama yasağı
    CMatrixMenu(const CMatrixMenu&) = delete;
    CMatrixMenu& operator=(const CMatrixMenu&) = delete;

    // --- YAPILANDIRMA (Fluent Interface) ---
    CMatrixMenu& SetGrid(int cols, int itemSize, int gap);
    CMatrixMenu& SetColors(COLORREF bg, COLORREF itemBg, COLORREF itemHover, COLORREF text);
    CMatrixMenu& SetFont(const CString& fontName, float fontSize);
    CMatrixMenu& SetCornerRadius(int radius);

    // --- İÇERİK EKLEME ---
    void AddItem(UINT cmdID, LPCTSTR text, UINT iconID = 0);
    void AddSubItem(UINT parentCmdID, UINT cmdID, LPCTSTR text, UINT iconID = 0);

    // --- GÖSTER ---
    void Show(POINT screenPt, HWND hParent);

    // Callback
    std::function<void(UINT)> OnCommandSelected;

protected:
    void RenderFrame();
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual void PreRegisterClass(WNDCLASS& wc) override;

private:
    // Veri Yapısı
    std::vector<MatrixItem> m_rootItems;         // Kök Menü
    std::vector<MatrixItem>* m_pCurrentItems;    // Şu an gösterilen liste pointer'ı
    std::stack<std::vector<MatrixItem>*> m_history; // Geri gitmek için yığın
    std::stack<CString> m_titleHistory;          // Başlık geçmişi

    HWND m_hParent;
    POINT m_screenPos;
    SIZE m_wndSize;
    
    // Görsel Ayarlar
    int m_cols;
    int m_itemSize;
    int m_gap;
    int m_cornerRadius;
    int m_headerHeight;
    
    // Renkler (GDI+ Color)
    Gdiplus::Color m_cBg;
    Gdiplus::Color m_cItemBg;
    Gdiplus::Color m_cItemHover;
    Gdiplus::Color m_cText;
    
    // Font
    CString m_fontName;
    float m_fontSize;

    // Animasyon
    float m_animAlpha; // 0.0 - 1.0 arası (Fade In)
    bool m_isClosing;

    // Yardımcılar
    void DrawItem(Gdiplus::Graphics& g, const MatrixItem& item, int x, int y);
    void DrawHeader(Gdiplus::Graphics& g); // Geri butonu ve başlık
    int GetItemIndexAt(POINT pt);
    bool IsBackButtonAt(POINT pt); // Geri butonuna mı tıklandı?

    static ULONG_PTR g_gdiplusToken;
    static int g_gdiplusRefCount;
};