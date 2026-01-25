#include "stdafx.h"
#include "FilterPanel.h"

CFilterPanel::CFilterPanel(UINT nResID) : CDialog(nResID)
{
}

CFilterPanel::~CFilterPanel()
{
}

BOOL CFilterPanel::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 1. Varsayılan olarak hepsini seçili getirebilirsin veya boş bırakabilirsin
    // CheckDlgButton(IDC_CHK_ALICI, BST_CHECKED);

    // 2. İlçe Combobox'ını doldur (Örnek)
    //SendDlgItemMessage(IDC_COMBO_ILCE, CB_ADDSTRING, 0, (LPARAM)L"Tümü");
    //SendDlgItemMessage(IDC_COMBO_ILCE, CB_ADDSTRING, 0, (LPARAM)L"Şahinbey");
    //SendDlgItemMessage(IDC_COMBO_ILCE, CB_ADDSTRING, 0, (LPARAM)L"Şehitkamil");
    //SendDlgItemMessage(IDC_COMBO_ILCE, CB_SETCURSEL, 0, 0); // "Tümü" seçili

    return TRUE;
}

BOOL CFilterPanel::OnCommand(WPARAM wparam, LPARAM lparam)
{
    UINT id = LOWORD(wparam);
    UINT code = HIWORD(wparam);

    // Checkbox tıklandıysa VEYA ComboBox değiştiyse
    if (code == BN_CLICKED || code == CBN_SELCHANGE)
    {
        // Parent pencereye (CContainFiles) haber ver
        // PostMessage kullanıyoruz ki UI takılmasın
        //GetParent().PostMessage(UWM_FILTER_CHANGED, 0, 0);
    }

    return CDialog::OnCommand(wparam, lparam);
}