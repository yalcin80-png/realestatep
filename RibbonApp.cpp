/////////////////////////////
// RibbonApp.cpp
//

#include "stdafx.h"
#include "RibbonApp.h"
#include "SyncProgressDlg.h"
///////////////////////////////////////
// CRibbonFrameApp function definitions
//

BOOL CRibbonFrameApp::InitInstance()
{
    // This function is called automatically when the application starts
    // NOTE:
    //  - Açılış senkronizasyon progress penceresini burada açmıyoruz.
    //  - Çünkü burada oluşturulan local provider/adapter/sync nesneleri InitInstance bitince yok olur.
    //  - Startup progress kontrolü + toggle ayarı MainFrame tarafında yönetiliyor.

    // Create the Window
    m_frame.Create();   // throws a CWinException on failure

    return TRUE;
}
