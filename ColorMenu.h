
#pragma once

#ifndef MENUCOLORTER
#define MENUCOLORTER


#include "stdafx.h"
#include <map>
#include <vector>

// Menü Öğesi Verisi
struct MyMenuItemData {
    CString text;
    UINT iconID;
    COLORREF textColor;
    COLORREF bgColor;
    bool isSeparator;
    
    MyMenuItemData() : iconID(0), textColor(RGB(0,0,0)), bgColor(RGB(255,255,255)), isSeparator(false) {}
};

class CColorMenu
{
public:
    CColorMenu();
    ~CColorMenu();

    // Menüyü "Owner Drawn" moduna çevirir
    void ConvertToOwnerDraw(HMENU hMenu);

    // İkon ve Renk Tanımları
    void AddIcon(UINT cmdID, UINT iconID); // Komut ID'sine ikon ata
    void SetColor(UINT cmdID, COLORREF text, COLORREF bg); // Özel renk ata

    // Win32 Mesajlarını İşleyen Fonksiyonlar (Dialog'dan çağrılacak)
    void OnMeasureItem(MEASUREITEMSTRUCT* lpMIS);
    void OnDrawItem(DRAWITEMSTRUCT* lpDIS);

private:
    // HMENU -> Veri eşleşmesi için
    std::map<UINT, MyMenuItemData*> m_itemMap;
    std::map<UINT, UINT> m_iconMap; // CommandID -> IconResourceID

    // GDI+ veya GDI ile ikon çizmek için yardımcı
    void DrawIcon(HDC hDC, UINT iconID, int x, int y, bool disabled);
};



#endif // !MENUCOLORTER
