#include "stdafx.h"
#include "ColorMenu.h"
#include "resource.h" 

CColorMenu::CColorMenu() {}

CColorMenu::~CColorMenu() {
    for (auto& pair : m_itemMap) delete pair.second;
    m_itemMap.clear();
}

void CColorMenu::AddIcon(UINT cmdID, UINT iconID) {
    m_iconMap[cmdID] = iconID;
}

void CColorMenu::ConvertToOwnerDraw(HMENU hMenu) {
    int count = GetMenuItemCount(hMenu);
    for (int i = 0; i < count; i++) {
        UINT id = GetMenuItemID(hMenu, i);
        
        MyMenuItemData* pData = new MyMenuItemData();
        
        // Ayırıcı (Separator) Kontrolü
        if (id == 0) { 
            pData->isSeparator = true; 
        } else {
            // Metni al
            TCHAR buffer[256];
            GetMenuString(hMenu, i, buffer, 256, MF_BYPOSITION);
            pData->text = buffer;
            
            // İkon var mı?
            if (m_iconMap.find(id) != m_iconMap.end()) {
                pData->iconID = m_iconMap[id];
            }
        }

        // Menü stilini OWNERDRAW yap ve veriyi iliştir
        ModifyMenu(hMenu, i, MF_BYPOSITION | MF_OWNERDRAW, id, (LPCTSTR)pData);
        
        // Alt menü varsa (Recursive)
        HMENU hSub = GetSubMenu(hMenu, i);
        if (hSub) ConvertToOwnerDraw(hSub);
        
        m_itemMap[id] = pData; // Temizlik için sakla
    }
}

void CColorMenu::OnMeasureItem(MEASUREITEMSTRUCT* lpMIS) {
    // Menü elemanının boyutunu belirle
    lpMIS->itemHeight = 32; // Satır yüksekliği
    lpMIS->itemWidth = 200; // Genişlik (Otomatik hesaplanabilir ama sabit şık durur)
}

void CColorMenu::OnDrawItem(DRAWITEMSTRUCT* lpDIS) {
    if (lpDIS->CtlType != ODT_MENU) return;

    MyMenuItemData* pData = (MyMenuItemData*)lpDIS->itemData;
    if (!pData) return;

    HDC hDC = lpDIS->hDC;
    RECT rc = lpDIS->rcItem;
    bool isSelected = (lpDIS->itemState & ODS_SELECTED);
    bool isDisabled = (lpDIS->itemState & ODS_DISABLED);
    bool isChecked = (lpDIS->itemState & ODS_CHECKED);

    // 1. Arka Planı Çiz
    COLORREF bgCol = isSelected ? RGB(220, 235, 255) : RGB(255, 255, 255);
    if (isDisabled) bgCol = RGB(245, 245, 245);

    HBRUSH hBr = CreateSolidBrush(bgCol);
    FillRect(hDC, &rc, hBr);
    DeleteObject(hBr);

    // 2. Sol Şeridi Çiz
    RECT rcGutter = rc; rcGutter.right = rc.left + 30;
    HBRUSH hBrGutter = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hDC, &rcGutter, hBrGutter);
    DeleteObject(hBrGutter);

    if (pData->isSeparator) {
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        HPEN hOld = (HPEN)SelectObject(hDC, hPen);
        MoveToEx(hDC, rc.left + 35, (rc.top + rc.bottom) / 2, NULL);
        LineTo(hDC, rc.right - 5, (rc.top + rc.bottom) / 2);
        SelectObject(hDC, hOld);
        DeleteObject(hPen);
        return;
    }

    // 3. İKON VEYA BITMAP ÇİZİMİ (DÜZELTİLEN KISIM) ---------------------------
    if (pData->iconID > 0) {
        int iconX = rc.left + 7;
        int iconY = rc.top + 6; // Ortalamak için ayarla

        // A) Önce ICON Olarak Yüklemeyi Dene
        HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(pData->iconID), IMAGE_ICON, 16, 16, 0);

        if (hIcon) {
            // İkon bulundu, çiz
            DrawIconEx(hDC, iconX, iconY, hIcon, 16, 16, 0, NULL, DI_NORMAL);
            DestroyIcon(hIcon);
        }
        else {
            // B) İkon değilse BITMAP Olarak Yüklemeyi Dene
            HBITMAP hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(pData->iconID), IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION);

            if (hBmp) {
                // Bitmap bulundu, çiz
                HDC hMemDC = CreateCompatibleDC(hDC);
                HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);

                // TransparentBlt yerine basit BitBlt kullanıyoruz (daha güvenli)
                // Eğer şeffaflık gerekirse arka plan rengini beyaza boyamak gerekebilir.
                BitBlt(hDC, iconX, iconY, 16, 16, hMemDC, 0, 0, SRCCOPY);

                SelectObject(hMemDC, hOldBmp);
                DeleteDC(hMemDC);
                DeleteObject(hBmp);
            }
        }
    }
    // -------------------------------------------------------------------------

    // 4. Check İşareti
    if (isChecked) {
        HBRUSH hCheckBr = CreateSolidBrush(RGB(180, 210, 255));
        FrameRect(hDC, &rcGutter, hCheckBr);
        DeleteObject(hCheckBr);
    }

    // 5. Metin
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, isDisabled ? RGB(150, 150, 150) : RGB(0, 0, 0));

    RECT rcText = rc; rcText.left += 35;
    DrawText(hDC, pData->text, -1, &rcText, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}