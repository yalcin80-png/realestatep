#include "stdafx.h"
#include "coldlg.h"
#include "dataIsMe.h"
#include "RibbonApp.h"
#include "resource.h"
#include "CTreeListVDlg.h"

BOOL CColumnManagerDlg::OnInitDialog()
{
    AttachItem(IDC_COL_LIST, m_list);

    // Checkbox'lı liste stili
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
    m_list.InsertColumn(0, _T("Kolon Adı"), LVCFMT_LEFT, 220);

    // Kolonları listeye ekle ve check durumunu ayarla
    int i = 0;
    for (const auto& c : m_columns)
    {
        m_list.InsertItem(i, c.title);
        m_list.SetCheckState(i, c.visible); // Mevcut görünürlük durumunu işaretle
        i++;
    }

    CenterWindow();
    return TRUE;
}

BOOL CColumnManagerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDOK:
        OnOK(); // Değişiklikleri uygula
        //EndDialog(IDOK);
        return TRUE;

    case IDCANCEL:
        EndDialog(IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

void CColumnManagerDlg::OnOK()
{
    // 1. ListView'daki seçimleri vektöre aktar
    int count = m_list.GetItemCount();
    // Güvenlik: Vektör boyutuyla liste boyutu eşleşmeli
    if (count > (int)m_columns.size()) count = (int)m_columns.size();

    for (int i = 0; i < count; ++i)
    {
        BOOL checked = m_list.GetCheckState(i);
        m_columns[i].visible = (checked != FALSE);
    }

    // 2. Diyaloğu Kapat (IDOK koduyla)
    // CDialog::OnOK() çağırmak yerine direkt EndDialog kullanmak daha güvenlidir.
    EndDialog(IDOK);
}