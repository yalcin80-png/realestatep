/////////////////////////////
// Mainfrm.cpp
//

#include "stdafx.h"
#include "DebugLog.h"

#include <UIRibbonPropertyHelpers.h>
#include "RibbonUI.h"
#include "RibbonApp.h"
#include "Mainfrm.h"
#include "Files.h"
#include "Resource.h"
#include "UserMessages.h"
#include "GoogleServices.h"
#include "SahibindenImportDlg.h"
#include "SahibindenBulkWindow.h"

// Registry settings
#include <windows.h>

constexpr COLORREF red   = RGB(255, 125, 0);
constexpr COLORREF green = RGB(0, 196, 0);
constexpr COLORREF blue  = RGB(0, 0, 255);
constexpr COLORREF black = RGB(0, 0, 0);




//CMyTreeListView* Gettreelistiew()
//{
//
//    CDockAllRestate* mFiles = (CDockAllRestate*)GetContainerApp()->GetMainFrame().GetDockFromID(ID_TREEDOCKER);
//    return &mFiles->GetCDockFiles().GetDockAllRestateTreeListView();
//
//
//}
//
CMyTreeListView* Gettreelistiew()
{
    // 1. Uygulama ve MainFrame işaretçilerini güvenli alalım
    CRibbonFrameApp* pApp = GetContainerApp();
    if (!pApp) return nullptr;

    CMainFrame* pMainFrame = &pApp->GetMainFrame();
    if (!pMainFrame) return nullptr;

    // 2. ID_TREEDOCKER kimlikli Docker'ı isteyelim
    // DİKKAT: Program açılışında bu henüz oluşmamış olabilir, NULL dönebilir.
    CDocker* pDocker = pMainFrame->GetDockFromID(ID_TREEDOCKER);

    // 3. Eğer Docker bulunamadıysa (henüz oluşmadıysa) hemen çık.
    if (!pDocker)
        return nullptr;

    // 4. Pointer'ı güvenli şekilde cast edelim
    CDockAllRestate* mFiles = static_cast<CDockAllRestate*>(pDocker);
    if (!mFiles)
        return nullptr;

    // 5. Artık güvenle erişebiliriz
    return &mFiles->GetCDockFiles().GetDockAllRestateTreeListView();
}

CMainFrame::CMainFrame() 
{
}

// Mainfrm.cpp içinde

int CMainFrame::OnCreate(CREATESTRUCT& cs)
{

    // Önce temel frame oluşturulsun
    int ret = CDockFrame::OnCreate(cs);

    // Login manager (shared_ptr) - gerekince EnsureSyncSetup içinde tekrar da kurulabilir
    if (!m_login)
        m_login = std::make_shared<LoginManager>();

    // Uygulama ayarlarını yükle (ini)
    LoadAppSettings();

    // Sistem menüsüne (pencere ikon menüsü) toggle ekle
    HMENU hSys = ::GetSystemMenu(GetHwnd(), FALSE);
    if (hSys)
    {
        ::AppendMenu(hSys, MF_SEPARATOR, 0, nullptr);
        ::AppendMenu(hSys, MF_STRING, IDM_SYS_TOGGLE_STARTUP_SYNC_PROGRESS, L"Başlangıçta Sync Progress Göster");
        UpdateSystemMenuCheck();
    }

    return ret;
}


//void CMainFrame::CreatePreviewPanelToolbar()
//{
//    // 1. Komut ID'lerinin Tanımı (Bitmap'e Göre)
//    const UINT buttons[] = {
//
//        IDC_BACKTOLISTVIEW,
//        IDC_ZOOMIN,              // 1. İkon: Büyüteç (+)
//        IDC_ZOOMOUT,             // 2. İkon: Büyüteç (-)
//        IDC_FIT,                 // 3. İkon: Çapraz Oklar (Fit to Window)
//        IDC_BESTFIT,             // 4. İkon: İç içe Oklar (Best Fit/Actual Size)
//        IDC_NORMAL,              // 5. İkon: Sayfa Simge (Normal Size/Actual Size)
//        0,                       // Ayırıcı
//        IDC_PRINT                // 6. İkon: Yazıcı (Print)
//    };
//
//    // 2. CToolBar nesnesini oluştur ve butonu ekle
//    if (!m_ToolBar.IsWindow())
//    {
//        // CToolBar'ı CPreviewDlg'ye alt pencere olarak oluştur
//        m_ToolBar.Create(*this);
//    }
//
//    // 3. CImageList'i yükle ve CToolBar'a ata
//
//    // Resource.rc'deki ID'yi kullanarak Bitmap'i yükle
//    Win32xx::CBitmap bmToolBar(IDB_TOOLBAR);
//
//    // Toolbar.bmp'deki ikonların standart boyutu (Örn: 16x16)
//    // Bitmap'iniz 48 piksel yüksekliğinde, bu yüzden 16x16'lık alanları kullanıyoruz.
//    const int ICON_WIDTH = 32;
//    const int ICON_HEIGHT = 32;
//
//    // CImageList'i oluştur
//    m_mImageList.Create(ICON_WIDTH, ICON_HEIGHT, ILC_COLOR32 | ILC_MASK, 0, 0);
//
//    // Bitmap'i ImageList'e ekle. Pembe arka planı (Magenta: RGB(255, 0, 255)) şeffaf maske olarak kullan.
//    m_mImageList.Add(bmToolBar, RGB(255, 0, 255));
//
//    // CToolBar'a ImageList'i ata
//    m_ToolBar.SetImageList(m_mImageList);
//
//    // 4. Butonları CToolBar'a ekle
//
//    // TBBUTTON yapılarını oluştur (Win32++'da AddButtons için genellikle bu formatta kullanılır)
//    std::vector<TBBUTTON> buttonInfos(ARRAYSIZE(buttons));
//
//    for (size_t i = 0; i < ARRAYSIZE(buttons); ++i)
//    {
//        buttonInfos[i].iBitmap = (buttons[i] == 0) ? 0 : static_cast<int>(i); // İkon sırası (0'dan başlar)
//        buttonInfos[i].idCommand = buttons[i];
//        buttonInfos[i].fsState = TBSTATE_ENABLED;
///        buttonInfos[i].fsStyle = (buttons[i] == 0) ? TBSTYLE_SEP : TBSTYLE_BUTTON;
//        buttonInfos[i].dwData = 0;
//        buttonInfos[i].iString = 0;
//    }
//
//    // TB_ADDBUTTONS mesajını CToolBar::AddButtons ile gönder
//    m_ToolBar.AddButtons(static_cast<UINT>(buttonInfos.size()), buttonInfos.data());
//
//    // 5. ToolBar yerleşimi
//    m_ToolBar.Autosize();
//    m_ToolBar.ShowWindow(SW_SHOW);
//}





BOOL CMainFrame::SetPreviewView()
{
    if (m_bPreviewActive)
        return TRUE;

    TRACE(_T("🔸 Preview Mode Aktif Ediliyor...\n"));

    // --- 1️⃣ Mevcut layout'u kaydet ---
    SaveDockRegistrySettings(_T("LayoutBeforePreview"));

    // 
    // --- 3️⃣ Mevcut view'i kaydet ---
    m_pLastView = &GetMainListView();
    m_bPreviewActive = true;



    // --- 5️⃣ PreviewDlg'yi ana view olarak set et ---
    SetView(m_previewDlg);
   // RecalcLayout();

    return TRUE;
}

// --- Preview Kapat ---
BOOL CMainFrame::SetNormalView()
{
    if (!m_bPreviewActive)
        return TRUE;


    // --- 1️⃣ PreviewDlg'yi kapat ---


    if (m_previewDlg.IsWindow())
    {
        // KRİTİK TEMİZLİK: CPreviewDlg'nin yıkıcısı çağrılmadığı için alt pencereleri
        // manuel olarak yok etmeliyiz.
        if (m_previewDlg.GetZoomWnd().IsWindow())
        {
            m_previewDlg.GetZoomWnd().Destroy();
        }
        m_ToolBar.Destroy();
        // CPreviewDlg'nin ana HWND'sini yok et
        m_previewDlg.Destroy();
    }



    // --- 2️⃣ Eski view'i geri getir ---
    if (m_pLastView)
        SetView(m_MainDocker);

    // --- 3️⃣ Layout'u geri yükle ---
   // LoadDockRegistrySettings(_T("LayoutBeforePreview"));
    //DWORD style = DS_CLIENTEDGE  | DS_NO_CAPTION | DS_NO_CLOSE;

    //CDocker* m_pDockRight = AddDockedChild(new CDockAllRestate, DS_DOCKED_BOTTOM | style, 400, ID_TREEDOCKER);
   //// GetDockAncestor()->GetActiveDocker()->GetDockFromID(ID_TREEDOCKER)->Hide();
   // ShowAllDockers();
    RecalcLayout();
    m_bPreviewActive = false;

    // --- 5️⃣ TreeListView ve ConsumerListView refresh ---


    if (CListCustomerView* pConsumer = &GetMainListView())
    {
        pConsumer->Invalidate();
        pConsumer->UpdateWindow();
    }

    return TRUE;
}



// Mainfrm.cpp

void CMainFrame::ShowAllDockers()
{
    // Docking sistemine erişim için bu metot, CRibbonDockFrame'den (CDockFrame)
    // miras alınan metotları kullanır.

    // 1. En üstteki Docker'ı (DockAncestor) al
    CDocker* pAncestor = GetDockAncestor();

    if (pAncestor && pAncestor->IsWindow())
    {
        // 2. DockAncestor'ın tuttuğı tüm Docker listesinde döngü yap
        // GetAllDockers() metodu CDocker sınıfında tanımlıdır.
        const auto& allDockers = pAncestor->GetAllDockers();

        for (CDocker* pDocker : allDockers)
        {
            if (pDocker && pDocker->IsWindow() && !pDocker->IsWindowVisible())
            {
                // Gizlenmiş pencereleri göster
                pDocker->ShowWindow(SW_SHOW);

                // Dock Bar'ı (splitter'ı) göster (Eğer varsa ve gizliyse)
                if (pDocker->IsDocked() && pDocker->GetDockBar().IsWindow() && !pDocker->GetDockBar().IsWindowVisible())
                {
                    pDocker->GetDockBar().ShowWindow(SW_SHOW);
                }
            }
        }

        // DockAncestor'ın kendisini de gösterebiliriz (genellikle zaten görünürdür)
        if (!pAncestor->IsWindowVisible())
        {
            pAncestor->ShowWindow(SW_SHOW);
        }
    }
}

void CMainFrame::HideAllDockers()
{
    CDocker* pAncestor = GetDockAncestor();

    if (pAncestor && pAncestor->IsWindow())
    {
        // DockAncestor'ın tuttuğı tüm Docker listesinde döngü yap
        const auto& allDockers = pAncestor->GetAllDockers();

        for (CDocker* pDocker : allDockers)
        {
            if (pDocker && pDocker->IsWindow() && pDocker->IsWindowVisible())
            {
                // Gizlenmiş pencereleri göster
                pDocker->ShowWindow(SW_HIDE);

                // Dock Bar'ı (splitter'ı) da gizle
                if (pDocker->IsDocked() && pDocker->GetDockBar().IsWindow() && pDocker->GetDockBar().IsWindowVisible())
                {
                    pDocker->GetDockBar().ShowWindow(SW_HIDE);
                }
            }
        }
    }
}




// Create the frame window.
HWND CMainFrame::Create(HWND parent)
{


    SetView(m_MainDocker);
    //SetView(m_view);

    GetDBManager().InitializeDatabase();
    // Set m_view as the view window of the frame.

    // Set the registry key name, and load the initial window position.
    // Use a registry key name like "CompanyName\\Application".
    LoadRegistrySettings(L"Win32++\\Ribbon Frame");

    // Load the settings from the registry with 4 MRU entries
    LoadRegistryMRUSettings(4);
    return CDockFrame::Create(parent);
}

 //This function is called when a ribbon button is pressed.
 //Refer to IUICommandHandler::Execute in the Windows 7 SDK documentation.
STDMETHODIMP CMainFrame::Execute(UINT32 cmdID, UI_EXECUTIONVERB verb, const PROPERTYKEY* key, const PROPVARIANT* ppropvarValue, IUISimplePropertySet* pCmdExProp)
{
	int vievmode = 0;
    if (UI_EXECUTIONVERB_EXECUTE == verb)
    {
        switch(cmdID)
        {
        case IDC_CMD_NEW:       OnFileNew();        break;
        case IDC_CMD_OPEN:      OnFileOpen();       break;
        case IDC_CMD_SAVE:      OnFileSave();       break;
        case IDC_CMD_SAVE_AS:   OnFileSaveAs();     break;
        case IDC_CMD_PRINT:     OnFilePrint();      break;
        case IDC_CMD_COPY:      TRACE("Copy\n");    break;
        case IDC_CMD_CUT:       TRACE("Cut\n");     break;
        case IDC_CMD_PASTE:     TRACE("Paste\n");   break;
        case IDC_CMD_ABOUT:     OnHelp();           break;
        case IDC_CMD_EXIT:      OnFileExit();       break;
        case IDC_RICHFONT:      TRACE("Font dialog\n");     break;
        case IDC_RIBBONHELP:    OnHelp();                   break;
        case IDC_MRULIST:    OnMRUList(key, ppropvarValue);      break;
        case IDC_CMD_VIEW_CARD:
            //OnUploadToCloud();
       break;
      case IDC_PEN_COLOR:
        {
            SetRedraw(FALSE);

            if (m_currentView == VIEW_HOME) 
            {
            }
            else 
            {
                SetView(m_view);
                m_currentView = VIEW_HOME;
            }

            ShowMenu(TRUE);
            ShowToolBar(TRUE);
            SetRedraw(TRUE);
            RedrawWindow();
            break;
        }
        case IDC_CUSTOMIZE_QAT: TRACE("Customize Quick Access ToolBar\n");  break;
        case IDM_FLT_HEPSI:
        case IDM_FLT_SICAK_ALICI:
        case IDM_FLT_SICAK_SATICI:
        case IDM_FLT_POT_ALICI:
        case IDM_FLT_POT_SATICI:
        case IDM_FLT_YATIRIMCI:
        case IDM_FLT_KIRACI:
        case IDM_FLT_EV_SAHIBI:
        case IDM_FLT_YENI:
        case IDM_FLT_TAKIP:
        case IDM_FLT_RANDEVU:
        case IDM_FLT_TEKLIF:
        case IDM_FLT_TAMAMLANDI:
        {
            // 1. Enum dönüşümü yap (Eğer Resource ID'ler Enum ile birebir aynı değilse)
            // Eğer Resource.h ve Enum'daki değerler aynıysa direkt cast edebilirsin.
            // Güvenli yöntem maplemektir:

            int categoryEnum = 0;
            if (cmdID == IDM_FLT_HEPSI) categoryEnum = CAT_HEPSI;
            else if (cmdID == IDM_FLT_SICAK_ALICI) categoryEnum = CAT_SICAK_ALICI;
            else if (cmdID == IDM_FLT_SICAK_SATICI) categoryEnum = CAT_SICAK_SATICI;
            else if (cmdID == IDM_FLT_POT_ALICI) categoryEnum = CAT_POTANSIYEL_ALICI;
            else if (cmdID == IDM_FLT_POT_SATICI) categoryEnum = CAT_POTANSIYEL_SATICI;
            else if (cmdID == IDM_FLT_YATIRIMCI) categoryEnum = CAT_YATIRIMCI;
            else if (cmdID == IDM_FLT_KIRACI) categoryEnum = CAT_KIRACI_ADAYI;
            else if (cmdID == IDM_FLT_EV_SAHIBI) categoryEnum = CAT_EV_SAHIBI_KIRALIK;
            else if (cmdID == IDM_FLT_YENI) categoryEnum = CAT_YENI_MUSTERI;
            else if (cmdID == IDM_FLT_TAKIP) categoryEnum = CAT_TAKIPTE;
            else if (cmdID == IDM_FLT_RANDEVU) categoryEnum = CAT_RANDEVULU;
            else if (cmdID == IDM_FLT_TEKLIF) categoryEnum = CAT_TEKLIF_VERDI;
            else if (cmdID == IDM_FLT_TAMAMLANDI) categoryEnum = CAT_ISLEM_TAMAM;

            // 2. View'a filtre emrini ver
            m_view.SetCategoryFilter(categoryEnum);

            // 3. (İsteğe Bağlı) Mesaj ver
            // MessageBox(_T("Filtre Uygulandı"), _T("Bilgi"), MB_OK);
        }
        break;



        default:
            {
                CString str;
                str.Format(L"Unknown Button %d\n",cmdID);
                TRACE(str);
            }
            break;
        }
    }

    return S_OK;
}


#include "CTreeListVDlg.h"

void CMainFrame::OnEditDelete()
{
    // O an klavye odağının (focus) hangi pencerede olduğunu bul
    CWnd pFocusWnd = GetFocus();
    if (!pFocusWnd) return;

    // ---------------------------------------------------------
    // DURUM 1: Kullanıcı Ağaç Yapısında (TreeList) mi?
    // ---------------------------------------------------------
    // Not: ID_TREEDOCKER, TreeList'i tutan docker'ın ID'sidir.
    CDocker* pTreeDocker = GetDockFromID(ID_TREEDOCKER);


    // Hiçbiri değilse standart silme işlemi (veya ses çıkarma)
    MessageBeep(MB_ICONASTERISK);
}









// Limit the minimum size of the window.
LRESULT CMainFrame::OnGetMinMaxInfo(UINT msg, WPARAM wparam, LPARAM lparam)
{
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lparam;
    const CSize minimumSize(500, 300);
    lpMMI->ptMinTrackSize.x = DpiScaleInt(minimumSize.cx);
    lpMMI->ptMinTrackSize.y = DpiScaleInt(minimumSize.cy);
    return FinalWindowProc(msg, wparam, lparam);
}

// The IUIRibbon interface provides the ability to specify settings and properties for thr ribbon.

// Called by OnFileOpen and in response to a UWM_DROPFILE message.
void CMainFrame::LoadFile(LPCWSTR fileName)
{
    // Retrieve the PlotPoint data
    if (GetDoc().FileOpen(fileName))
    {
        // Save the filename
        m_pathName = fileName;
        AddMRUEntry(fileName);
    }
    else
        m_pathName = L"";

    GetView().Invalidate();
}

void CMainFrame::MRUFileOpen(UINT mruIndex)
{
    CString mruText = GetMRUEntry(mruIndex);

    if (GetDoc().FileOpen(mruText))
        m_pathName = mruText;
    else
        RemoveMRUEntry(mruText);

    GetView().Invalidate();
}

// Process the messages from the (non-ribbon) Menu and Tool Bar.
// Used when there isn't a ribbon.
BOOL CMainFrame::OnCommand(WPARAM wparam, LPARAM lparam)
{
    UINT id = LOWORD(wparam);

    switch (id)
    {

    case IDM_FLT_HEPSI:
    case IDM_FLT_SICAK_ALICI:
    case IDM_FLT_SICAK_SATICI:
    case IDM_FLT_POT_ALICI:
    case IDM_FLT_POT_SATICI:
    case IDM_FLT_YATIRIMCI:
    case IDM_FLT_KIRACI:
    case IDM_FLT_EV_SAHIBI:
    case IDM_FLT_YENI:
    case IDM_FLT_TAKIP:
    case IDM_FLT_RANDEVU:
    case IDM_FLT_TEKLIF:
    case IDM_FLT_TAMAMLANDI:
    {
        // 1. Enum dönüşümü yap (Eğer Resource ID'ler Enum ile birebir aynı değilse)
        // Eğer Resource.h ve Enum'daki değerler aynıysa direkt cast edebilirsin.
        // Güvenli yöntem maplemektir:

        int categoryEnum = 0;
        if (id == IDM_FLT_HEPSI) categoryEnum = CAT_HEPSI;
        else if (id == IDM_FLT_SICAK_ALICI) categoryEnum = CAT_SICAK_ALICI;
        else if (id == IDM_FLT_SICAK_SATICI) categoryEnum = CAT_SICAK_SATICI;
        else if (id == IDM_FLT_POT_ALICI) categoryEnum = CAT_POTANSIYEL_ALICI;
        else if (id == IDM_FLT_POT_SATICI) categoryEnum = CAT_POTANSIYEL_SATICI;
        else if (id == IDM_FLT_YATIRIMCI) categoryEnum = CAT_YATIRIMCI;
        else if (id == IDM_FLT_KIRACI) categoryEnum = CAT_KIRACI_ADAYI;
        else if (id == IDM_FLT_EV_SAHIBI) categoryEnum = CAT_EV_SAHIBI_KIRALIK;
        else if (id == IDM_FLT_YENI) categoryEnum = CAT_YENI_MUSTERI;
        else if (id == IDM_FLT_TAKIP) categoryEnum = CAT_TAKIPTE;
        else if (id == IDM_FLT_RANDEVU) categoryEnum = CAT_RANDEVULU;
        else if (id == IDM_FLT_TEKLIF) categoryEnum = CAT_TEKLIF_VERDI;
        else if (id == IDM_FLT_TAMAMLANDI) categoryEnum = CAT_ISLEM_TAMAM;

        // 2. View'a filtre emrini ver
        m_view.SetCategoryFilter(categoryEnum);

        // 3. (İsteğe Bağlı) Mesaj ver
         MessageBox(_T("Filtre Uygulandı"), _T("Bilgi"), MB_OK);
    }
    break;


    case IDM_FILE_NEW:          OnFileNew();            return TRUE;
    case IDM_FILE_OPEN:         OnFileOpen();           return TRUE;
    case IDM_FILE_SAVE:         OnFileSave();           return TRUE;
    case IDM_FILE_SAVEAS:       OnFileSaveAs();         return TRUE;
    case IDM_FILE_PRINT:        OnFilePrint();          return TRUE;

    case IDM_IMPORT_SAHIBINDEN:
    {
        DebugLogW(L"[Sahibinden] Menu command received (IDM_IMPORT_SAHIBINDEN). Owner HWND=%p", GetHwnd());

        // Non-modal BrowserWindow-style bulk importer (keeps UI responsive).
        static CSahibindenBulkWindow* s_bulkWnd = nullptr;
        if (s_bulkWnd && s_bulkWnd->IsWindow())
        {
            s_bulkWnd->ShowWindow(SW_SHOWNORMAL);
            ::SetForegroundWindow(s_bulkWnd->GetHwnd());
        }
        else
        {
            s_bulkWnd = new CSahibindenBulkWindow();
            s_bulkWnd->CreateBulkWindow(GetHwnd());
            s_bulkWnd->ShowWindow(SW_SHOWNORMAL);
        }
        return TRUE;
    }

    case IDM_FILE_EXIT:         OnFileExit();           return TRUE;
    case IDW_FILE_MRU_FILE1:
    case IDW_FILE_MRU_FILE2:
    case IDW_FILE_MRU_FILE3:
    case IDW_FILE_MRU_FILE4:
    case IDW_FILE_MRU_FILE5:
    {
        UINT uMRUEntry = LOWORD(wparam) - IDW_FILE_MRU_FILE1;
        MRUFileOpen(uMRUEntry);
        return TRUE;
    }
    case IDM_SAVE_TO_DATABASE:
        m_view.OnSaveListToDatabase();
        return TRUE;
    case IDM_PEN_RED:     return TRUE;
    case IDM_PEN_BLUE:    return TRUE;
    case IDM_PEN_GREEN:   return TRUE;
    case IDC_CMD_VIEW_CARD:
        //OnUploadToCloud();
        break;
    case IDW_VIEW_STATUSBAR:    return OnViewStatusBar();
    case IDW_VIEW_TOOLBAR:      return OnViewToolBar();
    case IDM_VIEW_STARTUP_SYNC_PROGRESS:
        ToggleStartupSyncProgress();
        return TRUE;
    case IDM_SYS_TOGGLE_STARTUP_SYNC_PROGRESS:
        ToggleStartupSyncProgress();
        return TRUE;
    case IDM_HELP_ABOUT:        return OnHelp();

    case IDC_BACKTOLISTVIEW://Stock_List_Update();
        //m_zoomWnd.Destroy();
        ///GetAncestor().SendMessage(UWM_CHANGEVIEW);

        SetNormalView();
        break;

    
	case IDM_FILES_BACK:
	{
	    // Explorer-like BACK: always return to Customers-only mode.
	    // 1) Left panel: ensure customer list is visible (restore selection/scroll).
	    m_MainDocker.GetContainer().ShowCustomersView();

	    // 2) Right panel (TreeList/Property dock): hide it deterministically.
	    if (m_pDockAllRestate && m_pDockAllRestate->IsWindow())
	    {
	        m_pDockAllRestate->ShowWindow(SW_HIDE);
	        if (m_pDockAllRestate->IsDocked() && m_pDockAllRestate->GetDockBar().IsWindow())
	            m_pDockAllRestate->GetDockBar().ShowWindow(SW_HIDE);
	    }
	    if (CDocker* pTree = GetDockFromID(ID_TREEDOCKER))
	    {
	        if (pTree->IsWindow())
	            pTree->ShowWindow(SW_HIDE);
	        if (pTree->IsDocked() && pTree->GetDockBar().IsWindow())
	            pTree->GetDockBar().ShowWindow(SW_HIDE);
	    }

	    // 3) Toolbar context + layout
	    UpdateToolbarContext();
	    RecalcLayout();
	    return TRUE;
	}
case IDC_CMD_VIEW_REPORT:
        MessageBox( L"Rapor görünümü seçildi.", L"Görünüm", MB_OK);
        break;

    case IDC_CMD_VIEW_SMALL:
        MessageBox( L"Küçük görünüm seçildi.", L"Görünüm", MB_OK);
        break;

    case IDC_CMD_VIEW_LIST:
        MessageBox( L"Liste görünümü seçildi.", L"Görünüm", MB_OK);
        break;

    case IDC_CMD_VIEW_DETAILS:
        MessageBox( L"Ayrıntılı görünüm seçildi.", L"Görünüm", MB_OK);
        break;

    case IDM_FILE_UPLOAD_CLOUD:
        OnCloudUploadTest();  // ✅ PUSH (Buluta Gönder)
     break;
    case IDM_GOOGLE_EXIT:
        MessageBox(_T("Çıkış yapılıyor"), _T(""), 0);
     OnLogoutGoogle();
        break;

    }
    // 2) Menü/Ribbon’dan gelen MÜŞTERİ komutlarını sol view'a yolla
    if (IsCustomerCmd(id))
        return GetMainListView().OnCommand(wparam,lparam);

    // 3) Menü/Ribbon’dan gelen MÜLK komutlarını sağ view'a yolla
    if (IsPropertyCmd(id))
    {
        if (CMyTreeListView* pTLV = Gettreelistiew())
            return pTLV->OnCommand(wparam, lparam);
        return FALSE;
    }
    return FALSE;
}


#include "SyncProgressDlg.h"




// Bu tanımların doğru olduğundan (tırnak içinde boşluk olmadığından) emin olun:
static const char* GOOGLE_CLIENT_ID = "807047218656-1jo1hv5n9p8693tavnpsbibr1038e9uv.apps.googleusercontent.com";
static const char* GOOGLE_CLIENT_SECRET = "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";
static const char* GOOGLE_REDIRECT_URI = "http://127.0.0.1:8888"; // Sonunda '/' olmamalı


#include "vCompanyDlg.h"

void CMainFrame::OnCompanySettings()
{
    // 1. Veritabanı yöneticisine eriş
    DatabaseManager& db = DatabaseManager::GetInstance();

    // 2. Firma Ayarları penceresini oluştur (DB referansını gönder)
    CCompanyDialog dlg(db);

    // 3. Pencereyi aç (Modal olarak)
    // Not: CCompanyDialog::OnInitDialog içinde zaten "Kayıt var mı?" kontrolü yapılıyor.
    // Varsa dolduruyor (Update moduna hazır), yoksa boş açıyor (Insert moduna hazır).
    if (dlg.DoModal(*this) == IDOK)
    {
        // Kullanıcı "Kaydet" dediğinde CCompanyDialog::OnOK çalışır ve 
        // veritabanına Insert veya Update işlemini kendisi yapar.
        // Buraya sadece işlem bittikten sonra yapılacaklar (örn: Durum çubuğuna mesaj) yazılır.
        SetStatusText(_T("Firma bilgileri güncellendi."));
    }
}

void CMainFrame::EnsureSyncSetup()
{
 // NOTE: m_login can be created early via default ctor in OnCreate.
 // In that case it has no OAuth config and SignIn() will fail with
 // "Redirect URI eksik".
 if (!m_login || !m_login->IsConfigured())
 {
     m_login = std::make_shared<LoginManager>(
         GOOGLE_CLIENT_ID,
         GOOGLE_CLIENT_SECRET,
         GOOGLE_REDIRECT_URI
     );
 }

 // Initialize cloud provider and local adapter once
 try {
 if (!m_cloud) m_cloud = std::make_unique<FirestoreProvider>(m_login->GetAccessToken(), std::string("emlakcrm-480508"));
 if (!m_local) m_local = std::make_unique<AdoLocalAdapter>();
 if (!m_syncMgr) m_syncMgr = std::make_unique<SyncManager>(*m_local, *m_cloud);
 SyncBridge::GetInstance().Initialize("emlakcrm-480508", m_login.get());
 }
 catch (...) {
 TRACE(L"[EnsureSyncSetup] initialization failed\n");
 }
}

void CMainFrame::SyncTableAsync(const std::string& localTable, const std::string& cloudCollection)
{
 // Ensure login and components
 EnsureSyncSetup();

 // Start background thread to push then pull
 std::shared_ptr<LoginManager> loginCopy = m_login;
 auto cloudPtr = m_cloud.get();
 auto localPtr = m_local.get();

 std::thread t([this, loginCopy, cloudPtr, localPtr, localTable, cloudCollection]() {
 try {
 if (!loginCopy->IsLoggedIn()) {
 if (!loginCopy->SignIn(GetHwnd())) return;
 }

 // Refresh token/access token usage
 std::string token = loginCopy->GetAccessToken();
 if (cloudPtr) {
 // update cloud provider token if needed
 }

 AdoLocalAdapter* localAdapter = reinterpret_cast<AdoLocalAdapter*>(localPtr);
 FirestoreProvider* cloudProvider = reinterpret_cast<FirestoreProvider*>(cloudPtr);
 if (!localAdapter || !cloudProvider) return;

 SyncManager sync(*localAdapter, *cloudProvider);

 // Push local changes first
 sync.PushToCloud(localTable, cloudCollection);

 // Then pull remote changes
 sync.PullFromCloud(localTable, cloudCollection);

 this->PostMessage(WM_USER_SYNC_COMPLETE,1,0);
 }
 catch (const std::exception& e) {
 TRACE(L"[SyncTableAsync] exception: %S\n", e.what());
 this->PostMessage(WM_USER_SYNC_COMPLETE,0,0);
 }
 catch (...) {
 this->PostMessage(WM_USER_SYNC_COMPLETE,0,0);
 }
 });

 t.detach();
}

// Called in response to a UWM_DROPFILE message.

void CMainFrame::OnLogoutGoogle()
{
    if (m_login)
    {
        // 1. Login Manager üzerinden çıkış yap
        m_login->SignOut();

        // 2. Bellekteki Cloud ve Sync nesnelerini yok et (Reset)
        // Eğer bunları resetlemezsen eski token ile hafızada yaşamaya devam ederler.
      m_cloud.reset();    // FirestoreProvider'ı sil
        m_syncMgr.reset();  // SyncManager'ı sil

        // 3. Kullanıcıya bilgi ver
    MessageBox( L"Oturum başarıyla kapatıldı.", L"Bilgi", MB_ICONINFORMATION);
    }
}

// ============================================================================
// ✅ YENİ: PUSH (App → Cloud) SENKRONIZASYONU
// ============================================================================
void CMainFrame::OnCloudUploadTest()
{
 // 1. LoginManager oluştur
    m_login = std::make_shared<LoginManager>(
 GOOGLE_CLIENT_ID,
        GOOGLE_CLIENT_SECRET,
        GOOGLE_REDIRECT_URI
    );

    // 2. Giriş kontrol
    if (!m_login->SignIn(GetHwnd()))
    {
   return;  // Kullanıcı iptal etti
  }

    // 3. Thread için Login Pointer kopyası
    std::shared_ptr<LoginManager> loginForThread = m_login;

    // 4. PUSH işlemini başlat
    std::thread uploadThread([this, loginForThread]() {
      try
        {
      TRACE(L"[CloudUpload] PUSH işlemi başlatıldı\n");

     // Token al
     std::string token = loginForThread->GetAccessToken();
  std::string projectId = "emlakcrm-480508";

 // Providers oluştur
        FirestoreProvider cloudProvider(token, projectId);
  AdoLocalAdapter localAdapter;
  SyncManager syncMgr(localAdapter, cloudProvider);

      // === PHASE 1: MÜŞTERILER BULUTA GÖNDER ===
          TRACE(L"[CloudUpload] Müşteriler buluta gönderiliyor\n");
  if (syncMgr.PushToCloud("Customer", "Customer"))
      {
TRACE(L"[CloudUpload] Müşteriler başarıyla gönderildi\n");
       }
        else
     {
    TRACE(L"[CloudUpload] UYARI: Müşteri gönderme başarısız\n");
       }

     // === PHASE 2: KONUTLAR BULUTA GÖNDER ===
          TRACE(L"[CloudUpload] Konutlar buluta gönderiliyor\n");
   if (syncMgr.PushToCloud("HomeTbl", "Home"))
            {
    TRACE(L"[CloudUpload] Konutlar başarıyla gönderildi\n");
         }
   else
      {
    TRACE(L"[CloudUpload] UYARI: Konut gönderme başarısız\n");
     }

 // === PHASE 3: ARSALAR BULUTA GÖNDER ===
  TRACE(L"[CloudUpload] Arsalar buluta gönderiliyor\n");
            if (syncMgr.PushToCloud("Land", "Land"))
   {
        TRACE(L"[CloudUpload] Arsalar başarıyla gönderildi\n");
  }
   else
        {
           TRACE(L"[CloudUpload] UYARI: Arsa gönderme başarısız\n");
            }

    // === PHASE 4: TARLALAR BULUTA GÖNDER ===
            TRACE(L"[CloudUpload] Tarlalar buluta gönderiliyor\n");
       if (syncMgr.PushToCloud("Field", "Field"))
       {
          TRACE(L"[CloudUpload] Tarlalar başarıyla gönderildi\n");
    }
   else
         {
        TRACE(L"[CloudUpload] UYARI: Tarla gönderme başarısız\n");
         }

  // === PHASE 5: VILLALAR BULUTA GÖNDER ===
    TRACE(L"[CloudUpload] Villalar buluta gönderiliyor\n");
   if (syncMgr.PushToCloud("Villa", "Villa"))
{
          TRACE(L"[CloudUpload] Villalar başarıyla gönderildi\n");
            }
  else
{
           TRACE(L"[CloudUpload] UYARI: Villa gönderme başarısız\n");
          }

 // === BAŞARILI ===
       this->PostMessage(WM_USER_SYNC_COMPLETE, 1, 0);
        }
      catch (const std::exception& e)
        {
  TRACE(L"[CloudUpload] Thread hatası: %S\n", e.what());
            this->PostMessage(WM_USER_SYNC_COMPLETE, 0, 0);
        }
  catch (...)
   {
   TRACE(L"[CloudUpload] Bilinmeyen thread hatası\n");
      this->PostMessage(WM_USER_SYNC_COMPLETE, 0, 0);
        }
  });

    uploadThread.detach();
}

void CMainFrame::SetRibbonContext(int modeIndex)
{
    IUIFramework* pFramework ;
	//m_pIUIFramework.QueryInterface(IID_PPV_ARGS(&pFramework));
    // Win32++ kütüphanesinde IUIFramework pointer'ı genellikle m_pIUIFramework değişkeninde tutulur.
    if (pFramework)
    {
        // Modu hesapla: 0 -> 1, 1 -> 2, 2 -> 4 ...
        // UI_MAKEAPPMODE makrosu yoksa: INT32 iMode = (1 << modeIndex); kullanabilirsin.
        INT32 iMode = UI_MAKEAPPMODE(modeIndex);

        // Framework üzerinden modu ayarla
        HRESULT hr = pFramework->SetModes(iMode);

        if (FAILED(hr))
        {
            TRACE("Ribbon modu degistirilemedi! Hata: 0x%08X\n", hr);
        }
    }
}

LRESULT CMainFrame::OnDropFile(WPARAM wparam)
{
    // wParam is a pointer (LPCWSTR) to the filename
    LPCWSTR fileName = reinterpret_cast<LPCWSTR>(wparam);
    assert(fileName);

    // Load the file
    LoadFile(fileName);
    return 0;
}

void CMainFrame::OnFileExit()
{
    // Issue a close request to the frame
    Close();
}

void CMainFrame::OnFileOpen()
{
    CFileDialog dlg(TRUE, L"csv", NULL,
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
        L"CSV Dosyaları (*.csv)\0*.csv\0Tüm Dosyalar (*.*)\0*.*\0\0");

    // MFC’de: dlg.DoModal(*this)
    // Win32++’ta: dlg.DoModal(GetHwnd()) veya sadece dlg.DoModal()
    if (dlg.DoModal(GetHwnd()) == IDOK)
    {
        CString path = dlg.GetPathName();
        m_view.ImportFromExcelCSV(path);
    }
}

void CMainFrame::OnFileNew()
{
    GetDoc().GetAllPoints().clear();
    m_pathName = L"";
    GetView().Invalidate();
}

// Sends the bitmap extracted from the View window to a printer of your choice
// This function provides a useful reference for printing bitmaps in general
void CMainFrame::OnFilePrint()
{
    try
    {
        // Get the dimensions of the View window
        CRect viewRect = GetView().GetClientRect();
        int width = viewRect.Width();
        int height = viewRect.Height();

        // Copy the bitmap from the View window
        CClientDC viewDC(GetView());
        CMemDC memDC(viewDC);
        memDC.CreateCompatibleBitmap(viewDC, width, height);
        BitBlt(memDC, 0, 0, width, height, viewDC, 0, 0, SRCCOPY);
        CBitmap bmView = memDC.DetachBitmap();
        CPrintDialog printDlg;

        // Bring up a dialog to choose the printer
        if (printDlg.DoModal(GetView()) == IDOK)    // throws exception if there is no default printer
        {
            // Zero and then initialize the members of a DOCINFO structure.
            DOCINFO di;
            memset(&di, 0, sizeof(DOCINFO));
            di.cbSize = sizeof(DOCINFO);
            di.lpszDocName = L"Scribble Printout";
            di.lpszOutput = static_cast<LPWSTR>(nullptr);
            di.lpszDatatype = static_cast<LPWSTR>(nullptr);
            di.fwType = 0;

            // Begin a print job by calling the StartDoc function.
            CDC printDC = printDlg.GetPrinterDC();
            printDC.StartDoc(&di);

            // Inform the driver that the application is about to begin sending data.
            printDC.StartPage();

            BITMAPINFOHEADER bi{};
            bi.biSize = sizeof(bi);
            bi.biHeight = height;
            bi.biWidth = width;
            bi.biPlanes = 1;
            bi.biBitCount = 24;
            bi.biCompression = BI_RGB;

            // Note: BITMAPINFO and BITMAPINFOHEADER are the same for 24 bit bitmaps
            // Get the size of the image data
            VERIFY(memDC.GetDIBits(bmView, 0, height, nullptr, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS));

            // Retrieve the image data
            std::vector<BYTE> vBits(bi.biSizeImage, 0); // a vector to hold the byte array
            BYTE* pByteArray = vBits.data();
            VERIFY(memDC.GetDIBits(bmView, 0, height, pByteArray, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS));

            // Determine the scaling factors required to print the bitmap and retain its original proportions.
            float logPelsX1 = static_cast<float>(viewDC.GetDeviceCaps(LOGPIXELSX));
            float logPelsY1 = static_cast<float>(viewDC.GetDeviceCaps(LOGPIXELSY));
            float logPelsX2 = static_cast<float>(printDC.GetDeviceCaps(LOGPIXELSX));
            float logPelsY2 = static_cast<float>(printDC.GetDeviceCaps(LOGPIXELSY));
            float scaleX = logPelsX2 / logPelsX1;
            float scaleY = logPelsY2 / logPelsY1;

            int scaledWidth = static_cast<int>(static_cast<float>(width) * scaleX);
            int scaledHeight = static_cast<int>(static_cast<float>(height) * scaleY);

            // Use StretchDIBits to scale the bitmap and maintain its original proportions
            VERIFY(printDC.StretchDIBits(0, 0, scaledWidth, scaledHeight, 0, 0, width, height,
                pByteArray, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, SRCCOPY));

            // Inform the driver that the page is finished.
            printDC.EndPage();

            // Inform the driver that document has ended.
            printDC.EndDoc();
        }
    }

    catch (const CException& e)
    {
        // An exception occurred. Display the relevant information.
        MessageBox(e.GetText(), L"Print Failed", MB_ICONWARNING);
    }
}

void CMainFrame::OnExportExcel()
{
    CFileDialog dlg(TRUE, L"csv", NULL,
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
        L"CSV Dosyaları (*.csv)\0*.csv\0Tüm Dosyalar (*.*)\0*.*\0\0");

    // MFC’de: dlg.DoModal(*this)
    // Win32++’ta: dlg.DoModal(GetHwnd()) veya sadece dlg.DoModal()
    if (dlg.DoModal(GetHwnd()) == IDOK)
    {
        CString path = dlg.GetPathName();
        m_view.ImportFromExcelCSV(path);
    }
}



void CMainFrame::OnFileSave()
{
    CFileDialog dlg(FALSE, L"csv", L"Musteriler.csv",
        OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
        L"CSV Dosyaları (*.csv)\0*.csv\0Tüm Dosyalar (*.*)\0*.*\0\0");

    if (dlg.DoModal(GetHwnd()) == IDOK)
    {
        CString path = dlg.GetPathName();
        m_view.ExportToExcelCSV(path);
    }
}

void CMainFrame::OnFileSaveAs()
{
    CFileDialog fileDlg(FALSE, L"dat", nullptr, OFN_OVERWRITEPROMPT, L"Scribble Files (*.dat)\0*.dat\0\0");
    fileDlg.SetTitle(L"Save File");

    // Bring up the file open dialog retrieve the selected filename
    if (fileDlg.DoModal(*this) == IDOK)
    {
        CString str = fileDlg.GetPathName();

        // Save the file
        if (GetDoc().FileSave(str))
        {
            // Save the file name
            m_pathName = str;
            AddMRUEntry(m_pathName);
        }
    }
}

// Called in response to a UWM_GETALLPOINTS message.
LRESULT CMainFrame::OnGetAllPoints()
{
    // Get a pointer to the vector of PlotPoints
    std::vector<PlotPoint>* pAllPoints = &GetDoc().GetAllPoints();

    // Cast the pointer to a LRESULT and return it
    return reinterpret_cast<LRESULT>(pAllPoints);
}











void CMainFrame::OnInitialUpdate()
{











    DWORD style = DS_CLIENTEDGE  | DS_NO_CAPTION | DS_NO_CLOSE;

    m_pTreeDocker = AddDockedChild(new CDockAllRestate, DS_DOCKED_BOTTOM | style, 400, ID_TREEDOCKER);
   // m_docCustomers = AddDockedChild(new CDockFiles, DS_DOCKED_TOP | style, 400, ID_FILESDOCKER);

    
    //m_login = std::make_unique<LoginManager>();
    //m_login->SetClientId("360784613811-ojiavndfkfuetiibn4v8iam30165ir15.apps.googleusercontent.com");
    //m_login->SetClientSecret("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");




    //m_login = std::make_unique<LoginManager>();
    //m_login->SetClientId("360784613811-ojiavndfkfuetiibn4v8iam30165ir15.apps.googleusercontent.com");
    //m_login->SetClientSecret("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");


    // --- Startup Sync Progress ---
    // Ayar açık ise, uygulama her açılışta senkronizasyon progress penceresini gösterir.
    // (Kullanıcı menüden kapatıp/açabilir, ayar kalıcıdır.)
    if (m_showStartupSyncProgress)
    {
        try
        {
            // Login + provider + sync setup
            EnsureSyncSetup();

            // Eğer login yapılmadıysa, kullanıcıdan giriş iste
            if (m_login && !m_login->IsLoggedIn())
            {
                // Kullanıcı iptal ederse progress'i göstermeyelim
                if (!m_login->SignIn(GetHwnd()))
                {
                    return;
                }
            }

            // Token sonrası provider/sync objelerini garanti et
            EnsureSyncSetup();

            if (m_syncMgr)
            {
                SyncProgressDlg dlg(m_syncMgr.get());
                dlg.DoModal(*this);
            }
        }
        catch (...)
        {
            TRACE(L"[StartupSyncProgress] failed\n");
        }
    }

    // Menü check işaretini güncel tut
    UpdateSystemMenuCheck();
}

void CMainFrame::OnMRUList(const PROPERTYKEY* key, const PROPVARIANT* ppropvarValue)
{
    if (ppropvarValue != nullptr && key != nullptr && UI_PKEY_SelectedItem == *key)
    {
        UINT mruItem = ppropvarValue->ulVal;
        MRUFileOpen(mruItem);
    }
}

// Called when the DropdownColorPicker button is pressed.
//void CMainFrame::OnPenColor(const PROPVARIANT* ppropvarValue, IUISimplePropertySet* pCmdExProp)
//{
//    if (ppropvarValue != nullptr)
//    {
//        // Retrieve color type.
//        UINT type = ppropVarValue->uintVal;
//
//        // The Ribbon framework passes color as additional property if the color type is RGB.
//        if (type == UI_SWATCHCOLORTYPE_RGB && pCmdExProp != nullptr)
//        {
//            // Retrieve color.
//            PROPVARIANT var;
//            PropVariantInit(&var);
//            if (0 <= pCmdExProp->GetValue(UI_PKEY_Color, &var))
//            {
//                UINT color = var.uintVal;
//                m_view.SetPenColor((COLORREF)color);
//            }
//        }
//    }
//}

// Called in response to a UWM_SENDPOINT message.
LRESULT CMainFrame::OnSendPoint(WPARAM wparam)
{
    // wParam is a pointer to the vector of PlotPoints
    PlotPoint* pPP = reinterpret_cast<PlotPoint*>(wparam);

    // Dereference the pointer and store the vector of PlotPoints in CDoc
    GetDoc().StorePoint(*pPP);
    return 0;
}

 //OnViewChanged is called when the ribbon is changed.
STDMETHODIMP CMainFrame::OnViewChanged(UINT32, UI_VIEWTYPE typeId, IUnknown* pView, UI_VIEWVERB verb, INT32)
{
    HRESULT result = E_NOTIMPL;

    // Checks to see if the view that was changed was a Ribbon view.
    if (UI_VIEWTYPE_RIBBON == typeId)
    {
        switch (verb)
        {
        case UI_VIEWVERB_CREATE:    // The ribbon has been created.
            result = S_OK;
            break;
        case UI_VIEWVERB_SIZE:      // The ribbon's size has changed.
            RecalcLayout();
            result = S_OK;
            break;
        case UI_VIEWVERB_DESTROY:   // The ribbon has been destroyed.
            result = S_OK;
            break;
        case UI_VIEWVERB_ERROR:
            result = E_FAIL;
            break;
        }
    }

    return result;
}


/* Duran YALÇIN 17/01/2026
Fonksiyon Adı.:   CMainFrame::OnOpenCustomerPropertiesMessage
Açıklama :        ListView (Card mode) çift tık navigasyonunu güvenli şekilde işler.
                 UI kilitlenmelerini önlemek için bu işlem WM_LBUTTONDBLCLK içinde yapılmaz;
                 ListView PostMessage ile buraya erteler. Burada dock layout + TreeListView
                 doldurma tek seferde ve redraw kapalı şekilde yapılır.

*/
LRESULT CMainFrame::OnOpenCustomerPropertiesMessage(WPARAM, LPARAM lparam)
{
    std::unique_ptr<CString> pCari(reinterpret_cast<CString*>(lparam));
    if (!pCari) return 0;

    CString cariKod = *pCari;
    cariKod.Trim();
    if (cariKod.IsEmpty()) return 0;

    // Guard: toolbar/context update + notify re-entrancy
    m_inDeferredCustomerNav = true;

    // Ensure right docker is visible (split mode). Later you can decide based on a toggle.
    ShowAllDockers();

    // Fill TreeListView efficiently.
    CMyTreeListView* pTree = Gettreelistiew();
    if (pTree && pTree->IsWindow())
    {
        ::SendMessage(pTree->GetHwnd(), WM_SETREDRAW, FALSE, 0);
    }

    // Use the existing, refactored fill routine.
    GetMainListView().OpenCustomerPropertiesInTreeList(cariKod);

    if (pTree && pTree->IsWindow())
    {
        ::SendMessage(pTree->GetHwnd(), WM_SETREDRAW, TRUE, 0);
        ::RedrawWindow(pTree->GetHwnd(), nullptr, nullptr,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }

    m_inDeferredCustomerNav = false;
    UpdateToolbarContext();
    return 0;
}

// Yardımcı: Bir butonu ID'sine göre gizler veya gösterir
void CMainFrame::ShowToolbarButton(UINT nID, BOOL bShow)
{
    // Win32++ CToolBar üzerinden TB_HIDEBUTTON mesajı gönderiyoruz
    // MAKELONG(TRUE, 0) -> Gizle, MAKELONG(FALSE, 0) -> Göster
    GetToolBar().SendMessage(TB_HIDEBUTTON, nID, MAKELONG(!bShow, 0));
}

// Mainfrm.cpp içine (En alta ekleyin):

void CMainFrame::UpdateToolbarContext()
{
    // --- 1. ODAK VE SEÇİM DURUMUNU AL ---
    HWND hFocus = ::GetFocus();

    // ListView Bilgisi
    CListCustomerView& listView = GetMainListView();
    HWND hList = listView.GetHwnd();
    int listSelCount = listView.GetSelectedCount();

    // TreeView Bilgisi
    CMyTreeListView* pTree = Gettreelistiew();
    HWND hTree = (pTree) ? pTree->GetHwnd() : nullptr;
    bool treeHasSel = (pTree && pTree->GetSelection() != nullptr);

    // Kim Aktif?
    bool isListActive = (hFocus == hList) || ::IsChild(hList, hFocus);
    bool isTreeActive = (hFocus == hTree) || (hTree && ::IsChild(hTree, hFocus));

    // --- 2. GÖRÜNÜRLÜK AYARLARI ---

    // SENARYO A: MÜŞTERİ LİSTESİ (ListView) AKTİF
    if (isListActive || (!isTreeActive && !isListActive)) // Varsayılan olarak List kabul et
    {
        // >> Müşteri Butonlarını GÖSTER
        ShowToolbarButton(IDM_NEW_RECORD, TRUE);
        ShowToolbarButton(IDM_RECORD_UPDATE, TRUE);
        ShowToolbarButton(ID_MAINLISTVIEWMENU_KAYD32781, TRUE);
        ShowToolbarButton(IDM_DELETE_IS_SELECTED, TRUE);

        // >> Mülk Ekleme Butonlarını GÖSTER (Sadece Müşteri Seçiliyse)
        BOOL canAddProp = (listSelCount == 1);
        ShowToolbarButton(IDM_ADD_HOME, canAddProp);
        ShowToolbarButton(ID_EKLE_ARSA, canAddProp);
        ShowToolbarButton(ID_EKLE_VILLA, canAddProp);
        ShowToolbarButton(ID_EKLE_TARLA, canAddProp);
        ShowToolbarButton(ID_EKLE_TICARIALAN, canAddProp);
        ShowToolbarButton(ID_EKLE_BAGVBAHCE, canAddProp);

        // >> TreeView İşlemlerini GİZLE (Simetri: Burası müşteri alanı)
        ShowToolbarButton(IDM_TREELIST_EDIT, FALSE);
        ShowToolbarButton(IDM_TREELIST_VIEW, FALSE);
        ShowToolbarButton(IDM_TREELIST_PREVIEW, FALSE);
        ShowToolbarButton(IDM_TREELIST_DELETE, FALSE);
        ShowToolbarButton(IDM_TREELIST_SATILDI, FALSE);
    }

    // SENARYO B: PORTFÖY LİSTESİ (TreeView) AKTİF
    else if (isTreeActive)
    {
        // >> Müşteri Butonlarını GİZLE (Simetri: Burası mülk alanı)
        ShowToolbarButton(IDM_NEW_RECORD, FALSE);
        ShowToolbarButton(IDM_RECORD_UPDATE, FALSE);
        ShowToolbarButton(ID_MAINLISTVIEWMENU_KAYD32781, FALSE);
        ShowToolbarButton(IDM_DELETE_IS_SELECTED, FALSE);

        // >> Mülk Ekleme Butonlarını GİZLE (Buradan eklenmez)
        ShowToolbarButton(IDM_ADD_HOME, FALSE);
        ShowToolbarButton(ID_EKLE_ARSA, FALSE);
        ShowToolbarButton(ID_EKLE_VILLA, FALSE);
        ShowToolbarButton(ID_EKLE_TARLA, FALSE);
        ShowToolbarButton(ID_EKLE_TICARIALAN, FALSE);
        ShowToolbarButton(ID_EKLE_BAGVBAHCE, FALSE);

        // >> TreeView İşlemlerini GÖSTER
        ShowToolbarButton(IDM_TREELIST_EDIT, TRUE);
        ShowToolbarButton(IDM_TREELIST_VIEW, TRUE);
        ShowToolbarButton(IDM_TREELIST_PREVIEW, TRUE);
        ShowToolbarButton(IDM_TREELIST_DELETE, TRUE);
        ShowToolbarButton(IDM_TREELIST_SATILDI, TRUE);

        // Aktiflik kontrolü (Seçim yoksa disable yapılabilir ama şimdilik göster/gizle yapıyoruz)
        // EnableToolBarButton(IDM_TREELIST_EDIT, treeHasSel); 
    }

    // Araçlar hep açık kalsın
    ShowToolbarButton(IDM_RANDEVU_OLUSTUR, TRUE);
    ShowToolbarButton(IDM_FILE_UPLOAD_CLOUD, TRUE);
}
// Mainfrm.cpp içine:

void CMainFrame::SetupToolBar()
{
    // ---------------------------------------------------------
    // 1. GENEL İŞLEMLER (Her zaman görünür)
    // ---------------------------------------------------------
    AddToolBarButton(IDM_FILE_NEW);
    AddToolBarButton(IDM_FILE_SAVE);
    AddToolBarButton(IDM_FILE_PRINT);
    AddToolBarButton(0); // Ayırıcı

    // ---------------------------------------------------------
    // 2. MÜŞTERİ YÖNETİMİ (ListView Seçiliyken Aktif)
    // ---------------------------------------------------------
    AddToolBarButton(IDM_NEW_RECORD);                   // Yeni Müşteri
    AddToolBarButton(IDM_RECORD_UPDATE);                // Müşteri Düzenle
    AddToolBarButton(ID_MAINLISTVIEWMENU_KAYD32781);    // Müşteri Görüntüle
    AddToolBarButton(IDM_DELETE_IS_SELECTED);           // Müşteri Sil
    AddToolBarButton(0); // Ayırıcı

    // ---------------------------------------------------------
    // 3. MÜLK / PORTFÖY YÖNETİMİ (TreeList Seçiliyken Aktif)
    // ---------------------------------------------------------
    AddToolBarButton(IDM_TREELIST_EDIT);        // Mülk Düzenle
    AddToolBarButton(IDM_TREELIST_VIEW);        // Mülk Görüntüle
    AddToolBarButton(IDM_TREELIST_PREVIEW);     // Önizleme / Formlar
    AddToolBarButton(IDM_TREELIST_DELETE);      // Mülk Sil
    AddToolBarButton(IDM_TREELIST_SATILDI);     // Satıldı Yap
    AddToolBarButton(0); // Ayırıcı

    // ---------------------------------------------------------
    // 4. HIZLI MÜLK EKLEME (Müşteri Seçiliyken Aktif)
    // ---------------------------------------------------------
    AddToolBarButton(IDM_ADD_HOME);             // Ev Ekle
    AddToolBarButton(ID_EKLE_ARSA);             // Arsa Ekle
    AddToolBarButton(ID_EKLE_VILLA);            // Villa Ekle
    AddToolBarButton(ID_EKLE_TARLA);            // Tarla Ekle
    AddToolBarButton(ID_EKLE_TICARIALAN);       // Ticari Ekle
    AddToolBarButton(ID_EKLE_BAGVBAHCE);        // Bağ/Bahçe Ekle
    AddToolBarButton(0); // Ayırıcı

    // ---------------------------------------------------------
    // 5. ARAÇLAR VE BULUT (Bağlama Göre Değişebilir)
    // ---------------------------------------------------------
    AddToolBarButton(IDM_RANDEVU_OLUSTUR);      // Randevu
    AddToolBarButton(IDM_FILE_UPLOAD_CLOUD);    // Buluta Gönder
    AddToolBarButton(IDM_FILE_UPLOAD_CLOUD_PUSH); // Senkronize Et

    // Butonların düzgün görünmesi için varsa bitmap ayarını yap
    // SetToolBarImages(RGB(255,0,255), IDW_MAIN, 0, 0);
}// This function is called when a ribbon button is updated.
// Refer to IUICommandHandler::UpdateProperty in the Windows 7 SDK documentation.
STDMETHODIMP CMainFrame::UpdateProperty(UINT32 cmdID, __in REFPROPERTYKEY key, __in_opt  const PROPVARIANT*, __out PROPVARIANT* newValue)
{
    HRESULT result = E_NOTIMPL;

    if (UI_PKEY_Enabled == key)
    {
        return UIInitPropertyFromBoolean(UI_PKEY_Enabled, TRUE, newValue);
    }

    switch (cmdID)
    {
    case IDC_MRULIST:
        // Set up the Most Recently Used (MRU) menu
        if (UI_PKEY_Label == key)
        {
            WCHAR label[MAX_PATH] = L"Recent Files";
            result = UIInitPropertyFromString(UI_PKEY_Label, label, newValue);
        }
        else if (UI_PKEY_RecentItems == key)
        {
           // result = PopulateRibbonRecentItems(newValue);
        }
        break;

    case IDC_PEN_COLOR:
        // Set the initial pen color
        result = UIInitPropertyFromUInt32(key, RGB(1, 1, 1), newValue);
        break;
    }

    return result;
}

















LRESULT CMainFrame::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
            // Pencere odağı değişince Toolbar'ı güncelle
        case WM_SETFOCUS:
        case WM_ACTIVATE:
            UpdateToolbarContext();
            break;

            // Listelerden gelen bildirimleri dinle (Seçim değişimi vb.)
        case WM_NOTIFY:
        {
            LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);

            // ListView seçim değişimi (LVN_ITEMCHANGED)
            // TreeView seçim değişimi (TVN_SELCHANGED)
            if (!m_inDeferredCustomerNav &&
                (pHeader->code == LVN_ITEMCHANGED || pHeader->code == TVN_SELCHANGED))
            {
                // Seçim değiştiği an butonları ayarla
                UpdateToolbarContext();
            }
            break;
        }

        case CListCustomerView::UWM_OPEN_CUSTOMER_PROPERTIES:
            return OnOpenCustomerPropertiesMessage(wparam, lparam);

        case UWM_DATA_CHANGED:
            // Dialog/import sonrası verileri yenile
            m_view.RefreshList();
            UpdateToolbarContext();
            return 0;


        case UWM_CONTEXT_CHANGED:
            // wParam: 1=Customer, 2=Property
            m_contextKind = (int)wparam;
            DebugLogW(L"[CTX] Context changed. kind=%d", m_contextKind);
            // Update UI immediately
            UpdateToolbarContext();
            return 0;

        case WM_INITMENUPOPUP:
            // Enable/Disable menu items based on selection/context
            UpdateMenuEnableStates((HMENU)wparam);
            break;



        case UWM_CHANGEVIEW:
            SetView(m_view);
            SetRedraw(FALSE);
            ShowMenu(TRUE);
            ShowToolBar(TRUE);
            SetRedraw(TRUE);
            RedrawWindow();
            break;
        case WM_USER_SYNC_COMPLETE:
            if (wparam == 1)
            {
                MessageBox(L"Bulut senkronizasyonu başarıyla tamamlandı.", L"Tamamlandı", MB_ICONINFORMATION);

                // İsterseniz listeyi yenilemek için view'i update edin:
                // m_view.Invalidate(); 
            }
            else
            {
                MessageBox(L"Senkronizasyon sırasında hata oluştu veya bağlantı kurulamadı.", L"Hata", MB_ICONERROR);
            }
            return 0;

        case UWM_DROPFILE:          return OnDropFile(wparam);
        case UWM_GETALLPOINTS:      return OnGetAllPoints();
        case UWM_SENDPOINT:         return OnSendPoint(wparam);
        case WM_GETMINMAXINFO:      return OnGetMinMaxInfo(msg, wparam, lparam);
        }

        // Use the default message handling for remaining messages.
        return WndProcDefault(msg, wparam, lparam);
    }

    // Catch all unhandled CException types.
    catch (const CException& e)
    {
        // Display the exception and continue.
        CString str1;
        str1 << e.GetText() << L'\n' << e.GetErrorString();

        CString str2;
        str2 << "Error: " << e.what();
        TaskDialogBox(nullptr, str1, str2, TD_ERROR_ICON);
    }

    // Catch all unhandled std::exception types.
    catch (const std::exception& e)
    {
        // Display the exception and continue.
        CString str1 = e.what();
        ::MessageBox(nullptr, str1, L"Error: std::exception", MB_ICONERROR);
    }

    return 0;
}

// DOSYA: src/Mainfrm.cpp

//void CMainFrame::OnCloudSyncTest()
//{
//    // 1. LoginManager'ı Hazırla
//    m_login = std::make_shared<LoginManager>(
//        GOOGLE_CLIENT_ID,
//        GOOGLE_CLIENT_SECRET,
//        GOOGLE_REDIRECT_URI
//    );
//
//    // 2. Giriş Yap (Eğer token yoksa pencere açar)
//    if (!m_login->SignIn(GetHwnd()))
//        return;
//
//    // 3. Kullanıcıya Sor
//    int contactsChoice = MessageBox(
//        L"Telefon Rehberinizi ve Veritabanını Senkronize Etmek İster misiniz?",
//        L"Senkronizasyon",
//        MB_YESNOCANCEL | MB_ICONQUESTION
//    );
//
//    if (contactsChoice == IDCANCEL) return;
//
//    // Thread için login kopyası
//    std::shared_ptr<LoginManager> loginForThread = m_login;
//
//    // 4. ARKA PLAN İŞLEMİNİ BAŞLAT
//    std::thread syncThread([this, loginForThread, contactsChoice]() {
//
//        // [KRİTİK DÜZELTME 1] COM Kütüphanesini Başlat (ADO veritabanı için ŞARTTIR)
//        // Bu satır olmadan veritabanı asla çalışmaz.
//        HRESULT hr = ::CoInitialize(NULL);
//
//        try
//        {
//            TRACE(L"[CloudSync] Thread Başladı...\n");
//
//            // --- A) Gmail Rehber (İsteğe Bağlı) ---
//            if (contactsChoice == IDYES)
//            {
//                // Buraya GoogleServices kodlarınız gelecek...
//                // (Önceki kodunuzdaki gibi kalabilir)
//            }
//
//            // --- B) Cloud Sync Başlatılıyor ---
//            std::string token = loginForThread->GetAccessToken();
//            std::string projectId = "emlakcrm-480508"; // Proje ID'nizi kontrol edin!
//
//            FirestoreProvider cloudProvider(token, projectId);
//            AdoLocalAdapter localAdapter; // Artık hata vermez, çünkü CoInitialize yapıldı.
//            SyncManager syncMgr(localAdapter, cloudProvider);
//
//            // [KRİTİK DÜZELTME 2] Tablo İsim Eşleştirmeleri
//            // Sol Taraf: Yerel Access Tablosu Adı
//            // Sağ Taraf: Cloud Firestore Koleksiyon Adı
//            std::vector<std::pair<std::string, std::string>> taskList = {
//                {"Customer",     "Customer"},
//                {"HomeTbl",      "Home"},       // DİKKAT: Yerelde HomeTbl, Cloud'da Home
//                {"Land",         "Land"},
//                {"Field",        "Field"},
//                {"Villa",        "Villa"},
//                {"Vineyard",     "Vineyard"},
//                {"Commercial",   "Commercial"},
//                {"Appointments", "Appointments"},
//                {"Offers",       "Offers"}
//            };
//
//            // Hepsini Tek Seferde Eşitle (Önce Gönder, Sonra Al)
//            syncMgr.SyncMany(taskList, [](const std::string& msg, int percent) {
//                // İlerlemeyi Output penceresine yaz
//                TRACE(L"[Sync] %S (%%%d)\n", msg.c_str(), percent);
//                });
//
//            // İşlem Bitti Mesajı (Main Thread'e ilet)
//            this->PostMessage(WM_USER + 100, 1, 0);
//        }
//        catch (const std::exception& e)
//        {
//            TRACE(L"[CloudSync] HATA: %S\n", e.what());
//            this->PostMessage(WM_USER + 100, 0, 0);
//        }
//
//        // [KRİTİK DÜZELTME SONU] COM Kütüphanesini Kapat
//        if (SUCCEEDED(hr)) ::CoUninitialize();
//
//        });
//
//    syncThread.detach(); // Thread'i serbest bırak
//}



// Mainfrm.cpp -> OnCloudSyncTest
#include <nlohmann/json.hpp>

void CMainFrame::OnCloudSyncTest()
{
    // 1. Login İşlemleri
    m_login = std::make_shared<LoginManager>(
        GOOGLE_CLIENT_ID,
        GOOGLE_CLIENT_SECRET,
        GOOGLE_REDIRECT_URI
    );

    if (!m_login->SignIn(GetHwnd())) return;

    // Kullanıcıya sor
    int contactsChoice = MessageBox(
        L"Telefon Rehberinizi ve Veritabanını Senkronize Etmek İster misiniz?",
        L"Senkronizasyon",
        MB_YESNOCANCEL | MB_ICONQUESTION
    );

    if (contactsChoice == IDCANCEL) return;

    // Thread içinde kullanmak için kopyalar
    std::shared_ptr<LoginManager> loginForThread = m_login;

    std::thread syncThread([this, loginForThread, contactsChoice]()
        {
            // COM Başlatma (Thread içinde zorunludur)
            HRESULT hr = ::CoInitialize(NULL);

            try
            {
                TRACE(L"[CloudSync] Sync thread başladı\n");

                // Veritabanı yöneticisini al ve bağlantıyı garanti et
                DatabaseManager& db = DatabaseManager::GetInstance();
                if (!db.EnsureConnection()) {
                    TRACE(L"[CloudSync] HATA: Veritabanı bağlantısı kurulamadı!\n");
                    return;
                }

                // -----------------------------
                // A) GMAIL CONTACTS (KONTROLLÜ AKTARIM)
                // -----------------------------
                //if (contactsChoice == IDYES)
                //{
                //    std::string token = loginForThread->GetAccessToken();
                //    GoogleServices googleSvc(token);
                //    auto gmailContacts = googleSvc.GetAllContacts();

                //    // Yardımcı Lambda: Telefonu temizle (Sadece rakamlar, son 10 hane)
                //    auto CleanPhone = [](const std::string& s) -> std::string {
                //        std::string res;
                //        for (char c : s) {
                //            if (isdigit(c)) res += c;
                //        }
                //        // Son 10 haneyi al (0555... ile 555... eşleşsin diye)
                //        if (res.length() > 10) return res.substr(res.length() - 10);
                //        return res;
                //        };

                //    // Mevcut numaraları RAM'e çek (Performans için tek tek SQL atmıyoruz)
                //    std::vector<std::string> existingPhones;
                //    try {
                //        // DatabaseManager üzerinden sorgu atıyoruz
                //        _RecordsetPtr rs = db.IConnectionPtr->Execute(
                //            _bstr_t(L"SELECT Telefon FROM Customer WHERE Deleted=FALSE"), NULL, adCmdText);

                //        while (!rs->EndOfFile) {
                //            _variant_t v = rs->Fields->GetItem(L"Telefon")->Value;
                //            if (v.vt != VT_NULL) {
                //                CString cVal = (LPCWSTR)(_bstr_t)v;
                //                existingPhones.push_back(CleanPhone(db.CStringToAnsi(cVal)));
                //            }
                //            rs->MoveNext();
                //        }
                //        rs->Close();
                //    }
                //    catch (...) {}

                //    int eklenen = 0;
                //    int atlanan = 0;

                //    for (const auto& [name, phone] : gmailContacts)
                //    {
                //        if (name.empty() || phone.empty()) continue;

                //        std::string cleanInput = CleanPhone(phone);
                //        if (cleanInput.length() < 7) continue; // Çok kısa numarayı geç

                //        // RAM'deki listede var mı?
                //        bool exists = false;
                //        for (const auto& existing : existingPhones) {
                //            if (existing == cleanInput) { // Temizlenmiş hallerini karşılaştır
                //                exists = true;
                //                break;
                //            }
                //        }

                //        if (exists) {
                //            atlanan++;
                //            continue;
                //        }

                //        // Yeni Kayıt Oluştur
                //        Customer_cstr customer;
                //        customer.Cari_Kod = db.GenerateNextCariCode();
                //        customer.AdSoyad = CString(name.c_str());
                //        customer.Telefon = CString(phone.c_str());
                //        customer.MusteriTipi = _T("Bireysel");
                //        customer.Durum = _T("Aktif");

                //        // KRİTİK: sync_id ve Updated_At doldurulmalı
                //        customer.sync_id = db.GenerateGuidString();
                //        customer.Updated_At = db.GetCurrentIsoUtc();
                //        customer.Deleted = _T("FALSE");

                //        if (db.InsertGlobal(customer)) {
                //            eklenen++;
                //            existingPhones.push_back(cleanInput); // Eklenen de artık "var" sayılır
                //        }
                //    }
                //    TRACE(L"[GoogleContacts] %d yeni kişi eklendi, %d kişi zaten vardı.\n", eklenen, atlanan);
                //}

                // -----------------------------
                // B) CLOUD SYNC (FIRESTORE)
                // -----------------------------
                std::string token = loginForThread->GetAccessToken();
                FirestoreProvider cloudProvider(token, "emlakcrm-480508"); // Proje ID'niz

                // KRİTİK DÜZELTME: DatabaseManager'ın kullandığı yolu veriyoruz ki "D:\RealEstate.mdb"yi bulsun.
                CString wideConnStr = db.GetConnectionString();
                std::string ansiConnStr = db.CStringToAnsi(wideConnStr);
                AdoLocalAdapter localAdapter(ansiConnStr);

                SyncManager syncMgr(localAdapter, cloudProvider);

                // 1. Önce Gönder (PUSH)
                TRACE(L"[CloudSync] Local -> Cloud (Push)...\n");
                syncMgr.PushToCloud("Customer", "Customer");
                syncMgr.PushToCloud("HomeTbl", "Home");
                syncMgr.PushToCloud("Land", "Land");
                syncMgr.PushToCloud("Field", "Field");
                syncMgr.PushToCloud("Villa", "Villa");
                syncMgr.PushToCloud("Vineyard", "Vineyard");
                syncMgr.PushToCloud("Commercial", "Commercial");

                // 2. Sonra İndir (PULL)
                TRACE(L"[CloudSync] Cloud -> Local (Pull)...\n");
                syncMgr.PullFromCloud("Customer", "Customer");
                syncMgr.PullFromCloud("HomeTbl", "Home");
                syncMgr.PullFromCloud("Land", "Land");
                syncMgr.PullFromCloud("Field", "Field");
                syncMgr.PullFromCloud("Villa", "Villa");
                syncMgr.PullFromCloud("Vineyard", "Vineyard");
                syncMgr.PullFromCloud("Commercial", "Commercial");

                // -----------------------------
                // C) SYNC ZAMANI GÜNCELLEME (EN ÖNEMLİ DÜZELTME)
                // -----------------------------
                // Artık "Şu anki zamanı" kaydediyoruz. Böylece bir sonraki sefer 
                // sadece bu andan sonraki değişiklikler gelecek.
                CString nowUtc = db.GetCurrentIsoUtc(); // Örn: "2024-05-21T10:00:00Z"
                localAdapter.SetLastSyncStampIsoUtc(db.CStringToAnsi(nowUtc));

                TRACE(L"[CloudSync] Sync tamamlandı. Yeni Zaman Damgası: %s\n", nowUtc.GetString());

                // Arayüze bitti bilgisini gönder
                this->PostMessage(WM_USER + 100, 1, 0);
            }
            catch (const std::exception& e)
            {
                TRACE(L"[CloudSync] HATA: %S\n", e.what());
                this->PostMessage(WM_USER + 100, 0, 0);
            }

            if (SUCCEEDED(hr)) ::CoUninitialize();
        });

    syncThread.detach();
}


//void CMainFrame::OnCloudSyncTest()
//{
//    // 1. LoginManager
//    m_login = std::make_shared<LoginManager>(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET, GOOGLE_REDIRECT_URI);
//    if (!m_login->SignIn(GetHwnd())) return;
//
//    // 2. Kullanıcı Onayı
//    if (MessageBox(L"Verileri Cloud'dan indirip eşitlemek istiyor musunuz?", L"Sync", MB_YESNO) == IDNO) return;
//
//    std::shared_ptr<LoginManager> loginForThread = m_login;
//
//    // 3. Thread Başlat
//    std::thread syncThread([this, loginForThread]() {
//
//        // ADO Başlat (Zorunlu)
//        HRESULT hr = ::CoInitialize(NULL);
//
//        try
//        {
//            TRACE(L"[CloudSync] Thread başlatıldı\n");
//
//            std::string token = loginForThread->GetAccessToken();
//            std::string projectId = "emlakcrm-480508";
//
//            FirestoreProvider cloudProvider(token, projectId);
//            AdoLocalAdapter localAdapter;
//            SyncManager syncMgr(localAdapter, cloudProvider);
//
//            // =========================================================
//            // [SİHİRLİ DOKUNUŞ] TARİHİ SIFIRLA
//            // =========================================================
//            // Bunu eklemezseniz, "Ben zaten güncelim" der ve mülkleri indirmez.
//            localAdapter.SetLastSyncStampIsoUtc("1970-01-01T00:00:00Z");
//            TRACE(L"[CloudSync] Tarih 1970 yapıldı, tam indirme yapılacak.\n");
//            // =========================================================
//
//            // Tablo Listesi (HomeTbl -> Home eşleşmesine dikkat)
//            std::vector<std::pair<std::string, std::string>> taskList = {
//                {"Customer",     "Customer"},
//                {"HomeTbl",      "Home"},       // Access Tablosu: HomeTbl, Cloud: Home
//                {"Land",         "Land"},
//                {"Field",        "Field"},
//                {"Villa",        "Villa"},
//                {"Vineyard",     "Vineyard"},
//                {"Commercial",   "Commercial"},
//                {"Appointments", "Appointments"},
//                {"Offers",       "Offers"}
//            };
//
//            // Hepsini Tek Seferde Eşitle
//            syncMgr.SyncMany(taskList, [](const std::string& msg, int percent) {
//                TRACE(L"[Sync] %S (%d%%)\n", msg.c_str(), percent);
//                });
//
//            this->PostMessage(WM_USER + 100, 1, 0); // Başarılı
//        }
//        catch (const std::exception& e)
//        {
//            TRACE(L"[CloudSync] Hata: %S\n", e.what());
//            this->PostMessage(WM_USER + 100, 0, 0);
//        }
//
//        if (SUCCEEDED(hr)) ::CoUninitialize();
//        });
//
//    syncThread.detach();
//}


// ============================================================================
//  Startup Sync Progress (persistent setting)
// ============================================================================

static const wchar_t* kEmlakSettingsRegPath = L"Software\\EmlakCRM\\Settings";
static const wchar_t* kStartupSyncProgressValue = L"ShowStartupSyncProgress";

CString CMainFrame::GetAppSettingsIniPath() const
{
    // Eski/gelecek kullanım için: exe yanına ini yolu (şimdilik registry kullanıyoruz).
    wchar_t path[MAX_PATH]{};
    ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    CString ini(path);
    int dot = ini.ReverseFind(L'.');
    if (dot > 0)
        ini = ini.Left(dot);
    ini += L".ini";
    return ini;
}

void CMainFrame::LoadAppSettings()
{
    m_showStartupSyncProgress = true; // default

    HKEY hKey{};
    if (::RegOpenKeyExW(HKEY_CURRENT_USER, kEmlakSettingsRegPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwType = REG_DWORD;
        DWORD dwVal = 1;
        DWORD cbData = sizeof(dwVal);
        if (::RegQueryValueExW(hKey, kStartupSyncProgressValue, nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwVal), &cbData) == ERROR_SUCCESS)
        {
            m_showStartupSyncProgress = (dwVal != 0);
        }
        ::RegCloseKey(hKey);
    }
}

void CMainFrame::SaveAppSettings() const
{
    HKEY hKey{};
    if (::RegCreateKeyExW(HKEY_CURRENT_USER, kEmlakSettingsRegPath, 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        DWORD dwVal = m_showStartupSyncProgress ? 1u : 0u;
        ::RegSetValueExW(hKey, kStartupSyncProgressValue, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwVal), sizeof(dwVal));
        ::RegCloseKey(hKey);
    }
}

void CMainFrame::UpdateSystemMenuCheck() const
{
    // 1) System menu check
    HMENU hSys = ::GetSystemMenu(GetHwnd(), FALSE);
    if (hSys)
    {
        ::CheckMenuItem(hSys, IDM_SYS_TOGGLE_STARTUP_SYNC_PROGRESS,
            MF_BYCOMMAND | (m_showStartupSyncProgress ? MF_CHECKED : MF_UNCHECKED));
    }

    // 2) Main menu check
    HMENU hMenu = ::GetMenu(GetHwnd());
    if (hMenu)
    {
        ::CheckMenuItem(hMenu, IDM_VIEW_STARTUP_SYNC_PROGRESS,
            MF_BYCOMMAND | (m_showStartupSyncProgress ? MF_CHECKED : MF_UNCHECKED));
        ::DrawMenuBar(GetHwnd());
    }
}

void CMainFrame::ToggleStartupSyncProgress()
{
    m_showStartupSyncProgress = !m_showStartupSyncProgress;
    SaveAppSettings();
    UpdateSystemMenuCheck();
}

// Enable/Disable menu items based on current context and selection.
void CMainFrame::UpdateMenuEnableStates(HMENU hMenu)
{
    if (!hMenu) return;

    // Customer selection (ListView)
    bool hasCustomerSel = false;
    if (GetMainListView().IsWindow())
    {
        hasCustomerSel = (GetMainListView().GetNextItem(-1, LVNI_SELECTED) != -1);
    }

    // Property selection (TreeList)
    bool hasPropertySel = false;
    if (m_pDockAllRestate)
    {
        auto& tlv = m_pDockAllRestate->GetCDockFiles().GetDockAllRestateTreeListView();
        if (tlv.IsWindow())
            hasPropertySel = tlv.HasSelection();
    }

    const bool wantCustomer = (m_contextKind == 1);
    const bool wantProperty = (m_contextKind == 2);

    auto setMenu = [&](UINT id, bool enabled)
    {
        UINT state = enabled ? MF_ENABLED : MF_GRAYED;
        ::EnableMenuItem(hMenu, id, MF_BYCOMMAND | state);
    };

    // ---- Customer-dependent commands ----
    if (wantCustomer)
    {
        setMenu(IDM_LISTVIEW_EDIT_CUSTOMER, hasCustomerSel);
        setMenu(IDM_RECORD_UPDATE, hasCustomerSel);
        setMenu(IDM_DELETE_IS_SELECTED, hasCustomerSel);
        setMenu(IDM_ADD_HOME, hasCustomerSel);
        setMenu(IDM_OZELLIKLER, hasCustomerSel);
    }

    // ---- Property-dependent commands ----
    if (wantProperty)
    {
        setMenu(IDM_TREELIST_EDIT, hasPropertySel);
        setMenu(IDM_TREELIST_DELETE, hasPropertySel);
        setMenu(IDM_TREELIST_PREVIEW, hasPropertySel);

        // Status changes
        setMenu(IDM_STATUS_ACTIVE, hasPropertySel);
        setMenu(IDM_STATUS_SOLD, hasPropertySel);
        setMenu(IDM_STATUS_PRICE_DOWN, hasPropertySel);
        setMenu(IDM_STATUS_URGENT, hasPropertySel);
        setMenu(IDM_STATUS_PASSIVE, hasPropertySel);

        // Other property actions
        setMenu(IDM_TREELIST_SATILDI, hasPropertySel);
        setMenu(IDM_TREELIST_SATIS_DEVAM, hasPropertySel);
        setMenu(IDM_TREELIST_WAIT, hasPropertySel);
        setMenu(IDM_TREELIST_UNCERTAIN, hasPropertySel);
    }
}
