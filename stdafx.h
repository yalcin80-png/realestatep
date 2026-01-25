/////////////////////////////
// StdAfx.h
//

// This file is used for precompiled headers.
// Rarely modified header files should be included.
// Based on code provided by Lynn Allan.


#ifndef STDAFX_H
#define STDAFX_H


#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

enum CustomerCategory
{
    CAT_HEPSI               = 0,              // Filtre yok
    CAT_SICAK_ALICI         = 1,
    CAT_SICAK_SATICI        = 2,
    CAT_POTANSIYEL_ALICI    = 3,
    CAT_POTANSIYEL_SATICI   =  4,
    CAT_YATIRIMCI           = 5,
    CAT_KIRACI_ADAYI        = 6,
    CAT_EV_SAHIBI_KIRALIK   = 7,
    CAT_YENI_MUSTERI        = 8,
    CAT_TAKIPTE             = 9,            // Aranacak
    CAT_RANDEVULU           = 10,
    CAT_TEKLIF_VERDI        = 11,
    CAT_ISLEM_TAMAM         = 12        // Satış/Kiralama Tamamlandı
};







//#ifndef UNICODE
//#define UNICODE
//#endif
//#ifndef _UNICODE
//#define _UNICODE
//#endif
// Predefinitions for windows.h go here
//#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

// Specify Win32xx specific predefinitions here
//#define NO_USING_NAMESPACE        // Don't use Win32xx namespace

// Rarely modified header files should be included here
#include <vector>               // Add support for std::vector
#include <map>                  // Add support for std::map
#include <string>               // Add support for std::string
#include <sstream>              // Add support for stringstream
#include <cassert>              // Add support for the assert macro
#include <stdio.h>              // Add support for C style printf, sprintf, etc.
#include <stdlib.h>             // Add support for C style character conversions atoi etc.
#include <cstdarg>              // Add support for va_list
#include <tchar.h>              // Add support for C style TCHARs.

// Add the Win32++ library
#include <wxx_appcore.h>        // Add CWinApp
#include <wxx_archive.h>        // Add CArchive
#include <wxx_controls.h>       // Add CAnimation, CComboBox, CComboBoxEx, CDateTime, CHeader, CHotKey, CIPAddress, CProgressBar, CSpinButton, CScrollBar, CSlider, CToolTip
#include <wxx_criticalsection.h> // Add CCriticalSection, CThreadLock
#include <wxx_cstring.h>        // Add CString, CStringA, CStringW
#include <wxx_ddx.h>            // Add CDataExchange
#include <wxx_dialog.h>         // Add CDialog, CResizer
#include <wxx_dockframe.h>      // Add CDockFrame, CMDIDockFrame
#include <wxx_docking.h>        // Add CDocker, CDockContainer
#include <wxx_exception.h>      // Add CException, CFileException, CNotSupportedException, CResourceException, CUserException, CWinException
#include <wxx_file.h>           // Add CFile
#include <wxx_filefind.h>       // Add CFileFind
#include <wxx_folderdialog.h>   // Add CFolderDialog
#include <wxx_frame.h>          // Add CFrame
#include <wxx_gdi.h>            // Add CDC, CGDIObject, CBitmap, CBrush, CFont, CPalatte, CPen, CRgn
#include <wxx_hglobal.h>        // Add CHGlobal
#include <wxx_imagelist.h>      // Add CImageList
#include <wxx_listview.h>       // Add CListView
#include <wxx_mdi.h>            // Add CMDIChild, CMDIFrame, CDockMDIFrame
#include <wxx_menu.h>           // Add CMenu
#include <wxx_menubar.h>        // Add CMenuBar
#include <wxx_metafile.h>       // Add CMetaFile, CEnhMetaFile
#include <wxx_mutex.h>          // Add CEvent, CMutex, CSemaphore
#include <wxx_propertysheet.h>  // Add CPropertyPage, CPropertySheet
#include <wxx_rebar.h>          // Add CRebar
#include <wxx_rect.h>           // Add CPoint, CRect, CSize
#include <wxx_regkey.h>         // Add CRegKey
#include <wxx_richedit.h>       // Add CRichEdit
#include <wxx_scrollview.h>     // Add CScrollView
#include <wxx_setup.h>          // Adds macros, windows header files, and CObject
#include <wxx_socket.h>         // Add CSocket
#include <wxx_statusbar.h>      // Add CStatusBar
#include <wxx_stdcontrols.h>    // Add CButton, CEdit, CListBox
#include <wxx_tab.h>            // Add CTab, CTabbedMDI
#include <wxx_textconv.h>       // Add AtoT, AtoW, TtoA, TtoW, WtoA, WtoT etc.
#include <wxx_themes.h>         // Add MenuTheme, ReBarTheme, StatusBarTheme, ToolBarTheme
#include <wxx_thread.h>         // Add CWinThread
#include <wxx_time.h>           // Add CTime
#include <wxx_toolbar.h>        // Add CToolBar
#include <wxx_treeview.h>       // Add CTreeView
#include "wxx_treelistview.h"

#include <wxx_webbrowser.h>     // Add CWebBrowser, CAXHost
#include <wxx_wincore.h>        // Add CWnd

// WIN32_LEAN_AND_MEAN can't be used with these
#ifndef WIN32_LEAN_AND_MEAN
  #include <wxx_commondlg.h>      // Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog
  #include <wxx_folderdialogex.h> // Add CFolderDialogEx
  #include <wxx_preview.h>        // Add CPrintPreview
  #include <wxx_printdialogs.h>   // Add CPageSetupDialog, CPrintDialog
  #include <wxx_printdialogex.h>  // Add CPrintDialogEx
  #include <wxx_taskdialog.h>     // Add CTaskDialog

  // Visual Studio
  #if defined (_MSC_VER)
    #include <wxx_ribbon.h>       // Add CRibbon, CRibbonFrame
  #endif

#endif  // define WIN32_LEAN_AND_MEAN

// ---------------------------------------------------------------------------
// Variadic TRACE
// Some files use TRACE(fmt, ...) style calls. Win32++ typically provides a
// single-argument TRACE. Provide a compatible variadic version to avoid
// compile warnings and to keep debug output useful.
// ---------------------------------------------------------------------------
#ifdef TRACE
  #undef TRACE
#endif

#if defined(_DEBUG)
  inline void __TraceV(const wchar_t* fmt, va_list ap)
  {
      wchar_t buf[2048] = { 0 };
      _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, fmt ? fmt : L"", ap);
      ::OutputDebugStringW(buf);
  }
  inline void __TraceF(const wchar_t* fmt, ...)
  {
      va_list ap;
      va_start(ap, fmt);
      __TraceV(fmt, ap);
      va_end(ap);
  }
  #define TRACE(...) __TraceF(__VA_ARGS__)
#else
  #define TRACE(...) ((void)0)
#endif

#endif  // define STDAFX_H
