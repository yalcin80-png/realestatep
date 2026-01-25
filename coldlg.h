#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"
#include "wxx_treelistview.h"
// ColumnManagerDlg.h

class CColumnManagerDlg : public Win32xx::CDialog
{
public:
    // Sadece kolon listesini referans alıyoruz
    CColumnManagerDlg(std::vector<Win32xx::PropertyColumnInfo>& cols)
        : CDialog(IDD_COLUMN_MANAGER), m_columns(cols)
    {
    }

protected:
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual void OnOK() override; // Win32xx'de OnOK sanal olabilir veya Command içinde çağrılır

private:
    // Referans olduğu için burada yapılan değişiklik orijinal listeyi etkiler
    std::vector<Win32xx::PropertyColumnInfo>& m_columns;
    Win32xx::CListView m_list;
};
