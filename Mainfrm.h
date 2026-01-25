/////////////////////////////
// Mainfrm.h
//

#ifndef MAINFRM_H
#define MAINFRM_H

// --- Includes ---
#include "View.h"
#include "Files.h"
#include "Doc.h"
#include "dataIsMe.h"
#include "vHomeDlg.h"
#include "Customer.h"
#include "CTreeListVDlg.h"
#include "PreviewPanel.h"
#include "LoginManager.h"
#include "OAuthLoginWindow.h"
#include "FirestoreProvider.h"
#include "AdoLocalAdapter.h"
#include "SyncManager.h"
#include "SyncBridge.h"
#include "CloudDBManager.h"
#include "CloudSyncManager.h"

#include <memory>

// --- Constants ---
#define UWM_CHANGEVIEW 9182

class CMainFrame : public CDockFrame
{
public:
    // =========================================================================
    // 🎯 ENUMS & TYPES
    // =========================================================================
    enum FilterType {
        FILTER_ALL = 0,         // Tüm müşteriler
        FILTER_WORKING = 1,     // Çalışan müşteriler
        FILTER_NOT_WORKING = 2, // Çalışmayan müşteriler
        FILTER_THINKING = 3     // Düşünen müşteriler
    };

    enum ViewType {
        VIEW_TREE,
        VIEW_HOME
    };

    // =========================================================================
    // 🏗️ CONSTRUCTOR & DESTRUCTOR
    // =========================================================================
    CMainFrame();
    virtual ~CMainFrame() override = default;
    virtual HWND Create(HWND parent = nullptr) override;

    // Başlangıçta Sync Progress göster/gizle ayarı (kalıcı)
    bool IsStartupSyncProgressEnabled() const { return m_showStartupSyncProgress; }

    // =========================================================================
    // 🖥️ UI & VIEW MANAGEMENT
    // =========================================================================
    // Ana Docker (Sol Panel - Müşteri Listesi)
    CDockFiles m_MainDocker;

    // Sağ Panel (Mülk Listesi / TreeList)
    CDockAllRestate* GetRightDock() const { return m_pDockAllRestate; }

    // Ana Liste Erişim
    CListCustomerView& GetMainListView() {
        return m_MainDocker.GetContainer().GetListView();
    }

    // Görünüm Değiştirme (Preview / Normal)
    BOOL SetPreviewView();
    BOOL SetNormalView();

    // Preview Paneli Erişimi
    CPreviewPanel& GetPreviewDlg() { return m_previewDlg; }

    // Ribbon & Toolbar Kontrolü
    void ShowToolbarButton(UINT nID, BOOL bShow);
    void UpdateToolbarContext();
    void SetRibbonContext(int modeIndex);

    // Docker Göster/Gizle
    void ShowAllDockers();
    void HideAllDockers();

    // =========================================================================
    // ☁️ CLOUD & SYNC OPERATIONS
    // =========================================================================
    void OnCloudSyncTest();
    void OnCloudUploadTest(); // PUSH Handler
    void OnLogoutGoogle();
    void OnEditDelete();

    // =========================================================================
    // ⚙️ SYSTEM & HELPERS
    // =========================================================================
    DatabaseManager& GetDBManager() { return DatabaseManager::GetInstance(); }
    CDoc& GetDoc() { return m_doc; }

    // =========================================================================
    // 🖱️ COMMAND HANDLERS (Public Erişilebilir Olanlar)
    // =========================================================================
    void OnCompanySettings();
    void OnExportExcel();
    std::shared_ptr<LoginManager> m_login;
    std::unique_ptr<FirestoreProvider> m_cloud;
    std::unique_ptr<AdoLocalAdapter>   m_local;
    std::unique_ptr<SyncManager>       m_syncMgr;
    SyncBridge* m_syncBridge = nullptr;

protected:
    // =========================================================================
    // 🛡️ RIBBON INTERFACE IMPLEMENTATION (IUICommandHandler)
    // =========================================================================
    virtual STDMETHODIMP Execute(UINT32, UI_EXECUTIONVERB, const PROPERTYKEY*, const PROPVARIANT*, IUISimplePropertySet*) ;
    virtual STDMETHODIMP OnViewChanged(UINT32, UI_VIEWTYPE, IUnknown*, UI_VIEWVERB, INT32) ;
    virtual STDMETHODIMP UpdateProperty(UINT32, __in REFPROPERTYKEY, __in_opt const PROPVARIANT*, __out PROPVARIANT*) ;

    // =========================================================================
    // 🛡️ FRAMEWORK OVERRIDES
    // =========================================================================
    virtual BOOL    OnCommand(WPARAM wparam, LPARAM lparam) override;
    virtual void    OnInitialUpdate() override;
    virtual void    SetupToolBar() override;
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

    int OnCreate(CREATESTRUCT& cs);

private:
    // =========================================================================
    // 🔒 PRIVATE MEMBERS (Variables)
    // =========================================================================

    // --- Document & Data ---
    CDoc m_doc;
    CString m_pathName;
    ViewType m_currentView = VIEW_HOME;

    // --- UI Elements ---
    CToolBar m_ToolBar;
    CToolBar m_dbToolbar;
    CToolBar m_customerToolbar;
    CToolBar m_printToolbar;
    CToolBar m_viewToolbar;
    Win32xx::CImageList m_mImageList;

    // --- Docker Pointers ---
    CDocker* m_pTreeDocker = nullptr;
    CDocker* m_docCustomers = nullptr;
    CDocker* m_pNormalViewDocker = nullptr;
    CDockAllRestate* m_pDockAllRestate = nullptr;

    // --- Views & Dialogs ---
    CListCustomerView m_view; // Yedek view (Gerekirse)
    CPreviewPanel m_previewDlg;
    CCustomerDialog customerDlg;

    // --- State Flags ---
    bool m_bPreviewActive = false;

    // 1=Customer (ListView), 2=Property (TreeListView)
    int m_contextKind = 1;
    CWnd* m_pLastView = nullptr;

    // TreeListView doldurma sırasında (deferred navigation) toolbar/context
    // güncellemelerinin re-entrancy yaratmasını engellemek için.
    bool m_inDeferredCustomerNav = false;

    // Başlangıç sync progress toggle komut ID (Sistem menüsü)
    static constexpr UINT IDM_SYS_TOGGLE_STARTUP_SYNC_PROGRESS = 0x1F10;

    // --- Startup Settings ---
    bool m_showStartupSyncProgress = true;

    // --- Cloud / Sync Components ---

    // =========================================================================
    // 🔒 PRIVATE METHODS (Helpers & Handlers)
    // =========================================================================

    // --- Sync Helpers ---
    void EnsureSyncSetup();
    void SyncTableAsync(const std::string& localTable, const std::string& cloudCollection);

    // --- Deferred navigation handlers ---
    LRESULT OnOpenCustomerPropertiesMessage(WPARAM wparam, LPARAM lparam);

    // --- File Operations ---
    void OnFileExit();
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
    void OnFileSaveAs();
    void OnFilePrint();
    void LoadFile(LPCWSTR fileName);
    void MRUFileOpen(UINT mruIndex);
    void OnMRUList(const PROPERTYKEY* key, const PROPVARIANT* ppropvarValue);

    // --- Message Handlers ---
    LRESULT OnDropFile(WPARAM wparam);
    LRESULT OnGetAllPoints();
    LRESULT OnGetMinMaxInfo(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnSendPoint(WPARAM wparam);

    // --- Prevent Copy ---
    CMainFrame(const CMainFrame&) = delete;
    CMainFrame& operator=(const CMainFrame&) = delete;

    // --- Settings helpers ---
    CString GetAppSettingsIniPath() const;
    void LoadAppSettings();
    void SaveAppSettings() const;
    void UpdateSystemMenuCheck() const;
    void ToggleStartupSyncProgress();

    // Menu enable/disable based on selection/context
    void UpdateMenuEnableStates(HMENU hMenu);
};

#endif //MAINFRM_H