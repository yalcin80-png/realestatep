#pragma once
#include "resource.h"

class CFilterPanel : public CDialog
{
public:
    CFilterPanel(UINT nResID);
    virtual ~CFilterPanel();

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);

private:
    void InitControls(); // Başlangıç ayarları
};