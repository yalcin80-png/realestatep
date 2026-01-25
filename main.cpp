/////////////////////////////
// main.cpp
//

#include "stdafx.h"
#include "RibbonApp.h"

#if defined (_MSC_VER) && (_MSC_VER >= 1920)      // VS2019 or higher
#pragma warning( suppress : 28251 )  // Ignore the annotation requirement for wWinMain.
#endif


// --- BU FONKSİYONU EKLEYİN ---
void EnableModernBrowser()
{
    // Programın adını (örn: RibbonDockFrame.exe) dinamik olarak al
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    CString appName = PathFindFileName(szFileName);

    // FEATURE_BROWSER_EMULATION anahtarını aç
    HKEY hKey;
    if (RegCreateKey(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION"),
        &hKey) == ERROR_SUCCESS)
    {
        // IE11 Modu (11001 = 0x2AF9)
        DWORD value = 11001;
        RegSetValueEx(hKey, appName, 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
}













int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    try
    {
        // Start Win32++.
        CRibbonFrameApp theApp;

        // Run the application.
        return theApp.Run();
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

    return -1;
}
