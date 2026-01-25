#pragma once

#include <memory>
#include "LoginManager.h"
#include "GoogleServices.h"

class CContactsSyncDlg : public CDialog
{
public:
    CContactsSyncDlg(std::shared_ptr<LoginManager> loginMgr);
    virtual ~CContactsSyncDlg();
    
    BOOL OnInitDialog() override;
    LRESULT OnCommand(WPARAM wparam, LPARAM lparam) override;

    // Sync iþlemi tamamlandýktan sonra bu metodu çaðýr
    int GetUserChoice() const { return m_userChoice; }

private:
    std::shared_ptr<LoginManager> m_loginManager;
    int m_userChoice = IDCANCEL;  // IDYES = Senkronize Et, IDNO = Atla, IDCANCEL = Ýptal
};


