/////////////////////////////
// RibbonApp.h

#ifndef RIBBONAPP_H
#define RIBBONAPP_H

#include "Mainfrm.h"
#include "LoginManager.h"

/////////////////////////////////////////////////////////////////
// CRibbonFrameApp manages the application. It initializes the
// Win32++ framework when it is constructed, and creates the main
// frame window when it runs.
class CRibbonFrameApp : public CWinApp
{
public:
    LoginManager loginManager;
    CRibbonFrameApp() = default;
    virtual ~CRibbonFrameApp() override = default;
    CMainFrame& GetMainFrame() { return m_frame; }
protected:
    virtual BOOL InitInstance() override;

private:
    CRibbonFrameApp(const CRibbonFrameApp&) = delete;
    CRibbonFrameApp& operator=(const CRibbonFrameApp&) = delete;

    CMainFrame m_frame;
};


inline CRibbonFrameApp* GetContainerApp() { return static_cast<CRibbonFrameApp*>(GetApp()); }
#endif
