#pragma once
#include "stdafx.h"
#include <vector>
#include <functional>
#include <gdiplus.h>

// Menü Öğesi Yapısı (Alt öğeleri de tutacak şekilde)
struct RadialItem {
    UINT cmdID;
    CString text;
    UINT iconID;
    std::vector<RadialItem> subItems; // Alt Menüler
    bool hovered;
};

class CRadialMenu : public CWnd
{
public:
    CRadialMenu();
    virtual ~CRadialMenu();

    CRadialMenu(const CRadialMenu&) = delete;
    CRadialMenu& operator=(const CRadialMenu&) = delete;

    // Ana Öğeyi Ekle
    void AddItem(UINT cmdID, LPCTSTR text, UINT iconID = 0);

    // Alt Öğeyi Ekle (Hangi ana komuta bağlanacağını belirt)
    void AddSubItem(UINT parentCmdID, UINT cmdID, LPCTSTR text, UINT iconID = 0);

    void Show(POINT screenPt, HWND hParent);
    std::function<void(UINT)> OnCommandSelected;

protected:
    void RenderFrame();
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;
    virtual void PreRegisterClass(WNDCLASS& wc) override;

private:
    std::vector<RadialItem> m_items;
    HWND m_hParent;

    int m_currentRadius;
    int m_targetRadius;  // İç Halka Hedef Çapı
    int m_outerRadius;   // Dış Halka Hedef Çapı (Alt menü açılınca)
    int m_animStep;
    bool m_isClosing;

    POINT m_screenPos;
    SIZE m_wndSize;

    // Durum Takibi
    int m_hoveredMainIndex; // Hangi ana öğe aktif?
    int m_hoveredSubIndex;  // Hangi alt öğe aktif?

    void DrawRadialItems(Gdiplus::Graphics& g, int centerX, int centerY);

    // Yardımcılar
    void GetIndicesAt(POINT screenPt, int& outMain, int& outSub);

    static ULONG_PTR g_gdiplusToken;
    static int g_gdiplusRefCount;
};