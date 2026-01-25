#include "stdafx.h"
#include "SahibindenImportDlg.h"
#include "SahibindenImporter.h"
#include "UserMessages.h" 
#include <fstream>
#include "Files.h"
#include <shlobj.h>
#include <ctime>

#ifndef IDC_BROWSER_PLACEHOLDER
#define IDC_BROWSER_PLACEHOLDER 3005
#endif

CSahibindenImportDlg::CSahibindenImportDlg(HWND hParent)
    : CDialog(IDD_SAHI_IMPORT)
{
}



void CSahibindenImportDlg::ApplyClipStyles()
{
    if (!GetHwnd()) return; // Win32++: GetHwnd() varsa
    // Eğer GetHwnd() yoksa: HWND hWnd = m_hWnd; kullan

    HWND hWnd = GetHwnd();

    LONG_PTR style = ::GetWindowLongPtr(hWnd, GWL_STYLE);
    style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    ::SetWindowLongPtr(hWnd, GWL_STYLE, style);

    // Stilin uygulanması için:
    ::SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}









BOOL CSahibindenImportDlg::OnInitDialog()
{
    m_editUrl.AttachDlgItem(IDC_SAHI_URL, *this);
    m_editContactName.AttachDlgItem(IDC_SAHI_CONTACT_NAME, *this);
    m_editContactPhone.AttachDlgItem(IDC_SAHI_CONTACT_PHONE, *this);
    m_btnGo.AttachDlgItem(IDOK, *this);
    m_btnImport.AttachDlgItem(IDC_SAHI_DOWNLOAD, *this);
    m_btnPasteClip.AttachDlgItem(IDC_SAHI_PASTE_CLIP, *this);
    m_listLog.AttachDlgItem(IDC_SAHI_LOG, *this);

    m_editUrl.SetWindowText(_T("https://www.sahibinden.com/ilan/emlak-konut-satilik-sahibinden-sarigulukte-site-icerisinde-3-plus1-1294950437/detay"));

    // WebView2 child penceresi dialog uzerinde repaint/odak sorunlari cikartabiliyor.
    // Bu flag'ler flicker ve "dialog tiklanmiyor" semptomlarini ciddi azaltir.
    // NOT: WS_CLIPCHILDREN/WS_CLIPSIBLINGS olmadan WebView2, WM_PAINT/WM_NCHITTEST
    // akisinda parent dialog'un hit-test'ini bozabiliyor ve sanki dialog pasifmis gibi
    // gorunebiliyor (kullanici ancak baska pencereyi aktive edince duzeliyor).
    ApplyClipStyles();
    // WebView2 oluşturma callback tabanlı olsa da, dialog açılışında henüz
    // message loop tam oturmadığı için bazı makinelerde odak/klik sorunları
    // oluşabiliyor. Bu yüzden init'i dialog açıldıktan hemen sonra post ediyoruz.
    PostMessage(WM_APP_INIT_BROWSER, 0, 0);

    // Modal dialog + owned window kombinasyonunda bazen pencere "pasif" acilabiliyor.
    // SetForegroundWindow burada her zaman calismayabilir (foreground lock timeout).
    // Cozum: WS_EX_CONTROLPARENT ile tab/focus davranisini duzelt, ancak APPWINDOW
    // owned modal pencerelerde z-order/focus sorunlarina yol acabiliyor.
    LONG_PTR ex = ::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE);
    // WS_EX_APPWINDOW: owned modal pencerelerde taskbar/activation stabilitesini artirir.
    // (Bazı sistemlerde dialog ilk açılışta foreground olamıyor ve mouse/klavye "yokmuş"
    // gibi davranabiliyor. Bu flag bunu azaltır.)
    ::SetWindowLongPtr(GetHwnd(), GWL_EXSTYLE, ex | WS_EX_CONTROLPARENT | WS_EX_APPWINDOW);
    ::SetWindowPos(GetHwnd(), nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    PostMessage(WM_APP_FORCE_ACTIVATE, 0, 0);

    // Focus'u biz veriyoruz: Win32 dialog kuralina gore fokus verdiysek FALSE donmeliyiz.
    m_editUrl.SetFocus();
    return FALSE;
}

void CSahibindenImportDlg::ResizeBrowser()
{
    if (!m_browser.GetHwnd()) return;
    RECT rcBrowser;
    HWND hPlace = GetDlgItem(IDC_BROWSER_PLACEHOLDER);
    if (hPlace) {
        ::GetWindowRect(hPlace, &rcBrowser);
        ScreenToClient(rcBrowser);
    }
    else {
        RECT rcClient; rcClient = GetClientRect();
        rcBrowser = rcClient; rcBrowser.top += 120;
    }
    m_browser.SetWindowPos(NULL, rcBrowser, SWP_NOZORDER | SWP_SHOWWINDOW);
}

void CSahibindenImportDlg::OnButtonNavigate()
{
    CString strUrl;
    strUrl = m_editUrl.GetWindowText();
    if (strUrl.IsEmpty()) return;
    if (strUrl.Find(_T("http")) == -1) strUrl = _T("https://") + strUrl;
    LogLine(_T("Gidiliyor: ") + strUrl);
    m_browser.Navigate(strUrl);
}

// --- KRİTİK GÜNCELLEME BURADA ---
void CSahibindenImportDlg::OnButtonImport()
{
    if (m_importInProgress) return;

    LogLine(_T("Veri çekiliyor... (sayfanın tam yüklenmesi bekleniyor)"));
    m_btnImport.EnableWindow(FALSE);
    m_editUrl.SetWindowText(_T("https://www.sahibinden.com/ilan/emlak-konut-satilik-sahibinden-sarigulukte-site-icerisinde-3-plus1-1294950437/detay"));

    m_importUrl = m_editUrl.GetWindowText();

    m_importTry = 0;
    m_importStartTick = GetTickCount();
    m_importInProgress = true;

    BeginImportPoll();
}

void CSahibindenImportDlg::BeginImportPoll()
{
    // 250ms aralıklarla JSON'i kontrol et. UI kilitlenmesin diye timer kullanıyoruz.
    // Win32++ tarafında SetTimer overload/forwarding bazı sistemlerde WM_TIMER üretmeyebiliyor.
    // Bu yüzden doğrudan WinAPI kullanıyoruz.
    ::SetTimer(GetHwnd(), TIMER_IMPORT_POLL, 250, nullptr);
    LogLine(_T("Import poll timer basladi."));
}

void CSahibindenImportDlg::EndImportPoll()
{
    ::KillTimer(GetHwnd(), TIMER_IMPORT_POLL);
    m_importInProgress = false;
    m_btnImport.EnableWindow(TRUE);
}

void CSahibindenImportDlg::PollImportOnce()
{
    if (!m_importInProgress) return;

    const DWORD elapsed = GetTickCount() - m_importStartTick;
    m_importTry++;

    // 1) Öncelik: DOM içinden gaPageViewTrackingJson@data-json
    m_browser.GetListingJSON([this, elapsed](std::wstring jsonText) {
        std::wstring trimmed = jsonText;
        while (!trimmed.empty() && iswspace(trimmed.front())) trimmed.erase(trimmed.begin());
        while (!trimmed.empty() && iswspace(trimmed.back())) trimmed.pop_back();

        if (!trimmed.empty() && trimmed != L"null" && trimmed != L"undefined") {
            LogLine(_T("JSON paketi alındı, ayrıştırılıyor..."));
            EndImportPoll();
            this->ProcessJsonContent(jsonText);
            return;
        }

        if (elapsed >= 15000) {
            // 15sn sonra HTML fallback
            LogLine(_T("JSON bulunamadı (timeout). HTML üzerinden deneniyor..."));
            EndImportPoll();
            m_browser.GetSourceCode([this](std::wstring htmlContent) {
                this->ProcessHtmlContent(htmlContent);
            });
            return;
        }

        // Arada bir kısa durum yazalım
        if (m_importTry % 8 == 0) {
            CString msg;
            msg.Format(_T("Bekleniyor... deneme=%d sure=%u ms"), m_importTry, (unsigned)elapsed);
            LogLine(msg);
        }
    });
}


// JSON Verisini İşle
void CSahibindenImportDlg::ProcessJsonContent(const std::wstring& jsonText)
{
    LogLine(_T("İlan veri paketi yakalandı! Ayrıştırılıyor..."));

    // KRITIK: Sadece JSON ile kaydedersek satıcı/telefon/ozellikler eksik kalabilir.
    // Bu nedenle JSON'dan sonra HTML'i de alıp birlikte kaydedecegiz.
    CString currentUrl = m_editUrl.GetWindowText();

    // JSON -> UTF-8
    std::string utf8Json;
    int len = WideCharToMultiByte(CP_UTF8, 0, jsonText.data(), (int)jsonText.size(), NULL, 0, NULL, NULL);
    utf8Json.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, jsonText.data(), (int)jsonText.size(), utf8Json.data(), len, NULL, NULL);

    LogLine(_T("HTML alınıyor (satıcı/telefon/özellikler için)..."));
    m_browser.GetSourceCode([this, currentUrl, utf8Json](std::wstring htmlContent) {
        // HTML -> UTF-8
        std::string utf8Html;
        int hlen = WideCharToMultiByte(CP_UTF8, 0, htmlContent.c_str(), (int)htmlContent.size(), NULL, 0, NULL, NULL);
        utf8Html.resize(hlen);
        WideCharToMultiByte(CP_UTF8, 0, htmlContent.c_str(), (int)htmlContent.size(), utf8Html.data(), hlen, NULL, NULL);

        // Manuel iletisim bilgileri (kullanici onayli)
        CString manualName = m_editContactName.GetWindowText();
        CString manualPhone = m_editContactPhone.GetWindowText();

        SahibindenImporter importer;
        bool success = importer.ImportFromJsonAndHtmlString(currentUrl, utf8Json, utf8Html,
            manualName, manualPhone,
            [this](const CString& msg) {
            this->LogLine(msg);
        });

        if (success) {
            LogLine(_T(">>> İŞLEM BAŞARILI <<<"));
            MessageBox(_T("İlan kaydedildi."), _T("Başarılı"), MB_ICONINFORMATION);
            if (GetParent()) ::PostMessage(GetParent(), UWM_DATA_CHANGED, 0, 0);
        } else {
            LogLine(_T("Hata: Kayıt başarısız. (JSON+HTML)"));
            MessageBox(_T("Veriler alınsa da veritabanına kayıt edilemedi."), _T("Hata"), MB_ICONERROR);
        }
        m_btnImport.EnableWindow(TRUE);
    });
}

// Yedek HTML İşleme (Eski yöntem)
void CSahibindenImportDlg::ProcessHtmlContent(const std::wstring& html)
{
    if (html.find(L"Just a moment") != std::wstring::npos) {
        LogLine(_T("⚠️ Bot Koruması! Lütfen doğrulamayı yapın."));
        MessageBox(_T("Lütfen 'Robot değilim' kutucuğunu işaretleyin."), _T("Uyarı"), MB_ICONWARNING);
        m_btnImport.EnableWindow(TRUE);
        return;
    }

    std::string utf8Html;
    int len = WideCharToMultiByte(CP_UTF8, 0, html.c_str(), (int)html.size(), NULL, 0, NULL, NULL);
    utf8Html.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, html.c_str(), (int)html.size(), utf8Html.data(), len, NULL, NULL);

    SahibindenImporter importer;
    CString currentUrl;
    currentUrl = m_editUrl.GetWindowText();

    bool success = importer.ImportFromHtmlString(currentUrl, utf8Html, [this](const CString& msg) {
        this->LogLine(msg);
        });

    if (success) {
        LogLine(_T(">>> İŞLEM BAŞARILI (HTML Modu) <<<"));
        MessageBox(_T("İlan kaydedildi."), _T("Başarılı"), MB_ICONINFORMATION);
        if (GetParent()) ::PostMessage(GetParent(), UWM_DATA_CHANGED, 0, 0);
    }
    else {
        LogLine(_T("Hata: Veriler okunamadı."));
        std::ofstream out("C:\\sahibinden_dump.html", std::ios::binary);
        if (out.is_open()) { out.write(utf8Html.c_str(), utf8Html.size()); out.close(); }
        MessageBox(_T("Veriler okunamadı. İlan detayları ekranda görünüyor mu?"), _T("Hata"), MB_ICONERROR);
    }
    m_btnImport.EnableWindow(TRUE);
}

INT_PTR CSahibindenImportDlg::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_SHOWWINDOW:
        // Dialog ilk kez gosterildiginde (modal icin) aktivasyon bazen gecikebiliyor.
        // Bu da fare/klavye "hic calismiyor" hissi veriyor. Burada tek seferlik
        // net bir aktivasyon veriyoruz.
        if (wParam) {
            ::SetWindowPos(GetHwnd(), HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            ::SetForegroundWindow(GetHwnd());
            ::SetActiveWindow(GetHwnd());
            ::BringWindowToTop(GetHwnd());
        }
        break;
    case WM_APP_INIT_BROWSER:
        // Browser init'i dialog message loop basladıktan sonra yapıyoruz.
        if (!m_browser.GetHwnd()) {
            LogLine(_T("[Sahibinden] Browser init..."));
            m_browser.InitBrowser(GetHwnd());
            ResizeBrowser();
            m_browser.ShowWindow(SW_SHOW);
            // Bazi sistemlerde dialog ilk acilisinda aktif olamiyor ve click "donuyor" gibi gorunuyor.
            // Zorla aktive edelim.
            ::SetForegroundWindow(GetHwnd());
            ::SetActiveWindow(GetHwnd());
            ::BringWindowToTop(GetHwnd());
            CString startUrl = m_editUrl.GetWindowText();
            if (startUrl.IsEmpty()) startUrl = _T("https://www.sahibinden.com");
            m_browser.Navigate(startUrl);
        }
        return 0;
    case WM_APP_FORCE_ACTIVATE:
        // Alt-Tab / Explorer uzerine gelince duzelen semptomu icin:
        // "NOACTIVATE" kullanmak tam ters etki yaratabiliyor. Bu blokta
        // dialog'u GOSTER + ON PLANA GETIR + AKTIF ET akisini netlestiriyoruz.
        ::ShowWindow(GetHwnd(), SW_SHOWNORMAL);

        // En uste al + goster
        ::SetWindowPos(GetHwnd(), HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

        // Foreground kilidi her zaman izin vermez; ama menu komutundan acildigi icin
        // buyuk oranda calisir.
        ::SetForegroundWindow(GetHwnd());
        ::SetActiveWindow(GetHwnd());
        ::BringWindowToTop(GetHwnd());
        ::SetFocus(m_editUrl.GetHwnd());
        return 0;
    case WM_SETFOCUS:
        // Parent dialog odak aldiginda WebView2 bazen focus'u yutuyor.
        // URL kutusuna odak verelim ki klavye girdisi kaybolmasin.
        if (m_editUrl.GetHwnd()) ::SetFocus(m_editUrl.GetHwnd());
        return 0;
    case WM_MOUSEACTIVATE:
        // Modal dialog + WebView2 kombinasyonunda bazen pencere aktif olmadan tiklama gidiyor.
        // Burada her tiklamada dialog'u aktive ediyoruz.
        ::SetActiveWindow(GetHwnd());
        return MA_ACTIVATE;
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) {
            ::BringWindowToTop(GetHwnd());
        }
        break;
    case WM_SIZE:
        ResizeBrowser();
        break;
    case WM_TIMER:
        if (wParam == TIMER_IMPORT_POLL) {
            PollImportOnce();
            return 0;
        }
        break;
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CSahibindenImportDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT id = LOWORD(wParam);
    switch (id) {
    case IDOK:
        OnButtonNavigate();
        return TRUE;
    case IDC_SAHI_DOWNLOAD:
        OnButtonImport();
        return TRUE;
    case IDC_SAHI_PASTE_CLIP:
        {
            if (::OpenClipboard(GetHwnd())) {
                HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
                if (hData) {
                    const wchar_t* pText = (const wchar_t*)::GlobalLock(hData);
                    if (pText) {
                        CString s = pText;
                        ::GlobalUnlock(hData);
                        s.Trim();

                        // Clipboard'ta iki satir varsa: 1=ad, 2=telefon
                        int pos = s.Find(L"\r\n");
                        if (pos < 0) pos = s.Find(L"\n");
                        if (pos >= 0) {
                            CString name = s.Left(pos); name.Trim();
                            CString rest = s.Mid(pos); rest.Trim();
                            m_editContactName.SetWindowText(name);
                            m_editContactPhone.SetWindowText(rest);
                        } else {
                            // Tek satir: telefon gibi gorunuyorsa telefona, degilse ada bas.
                            CString digits;
                            for (int i = 0; i < s.GetLength(); ++i) {
                                TCHAR ch = s[i];
                                if (ch >= _T('0') && ch <= _T('9')) digits += ch;
                            }
                            if (digits.GetLength() >= 10)
                                m_editContactPhone.SetWindowText(s);
                            else
                                m_editContactName.SetWindowText(s);
                        }
                    }
                }
                ::CloseClipboard();
            }
        }
        return TRUE;
    case IDCANCEL:
        EndDialog(IDCANCEL);
        return TRUE;
    }
    return CDialog::OnCommand(wParam, lParam);
}


void CSahibindenImportDlg::LogLine(const CString& text) {
    if (m_listLog.GetHwnd()) {
        int idx = m_listLog.AddString(text);
        if (idx != LB_ERR) m_listLog.SetCurSel(idx);
    }

    // Ayrica dosyaya da yaz (UTF-8). VS Output'a bagimli kalmayalim.
    // Not: std::wofstream locale/encoding problemi yuzunden bazi makinelerde mojibake uretiyor.
    // Burada explicit UTF-8 yazariz.
    try {
        std::wstring pathW = GetLogFilePath();

        // timestamp
        std::time_t t = std::time(nullptr);
        std::tm tm{};
        localtime_s(&tm, &t);
        wchar_t ts[64]{};
        wcsftime(ts, 64, L"%Y-%m-%d %H:%M:%S", &tm);

        CString line;
        line.Format(_T("[%s] %s\r\n"), ts, (LPCTSTR)text);

        // Wide -> UTF-8
        int n = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)line, -1, nullptr, 0, nullptr, nullptr);
        if (n > 1) {
            std::string u8(n - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)line, -1, &u8[0], n - 1, nullptr, nullptr);

            // Append (create if needed)
            // If new file -> write UTF-8 BOM once
            FILE* fp = nullptr;
            _wfopen_s(&fp, pathW.c_str(), L"ab");
            if (fp) {
                // file size check
                long long size = 0;
                _fseeki64(fp, 0, SEEK_END);
                size = _ftelli64(fp);
                if (size == 0) {
                    const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
                    fwrite(bom, 1, 3, fp);
                }
                fwrite(u8.data(), 1, u8.size(), fp);
                fclose(fp);
            }
        }
    }
    catch (...) {
        // log yazimi kritik degil
    }
}

std::wstring CSahibindenImportDlg::GetLogFilePath() const
{
    wchar_t tempPath[MAX_PATH]{};
    ::GetTempPathW(MAX_PATH, tempPath);
    std::wstring dir = std::wstring(tempPath) + L"RibbonDockFrame";
    ::CreateDirectoryW(dir.c_str(), nullptr);
    return dir + L"\\sahibinden_import.log";
}
void CSahibindenImportDlg::OnCancel() { EndDialog(IDCANCEL); }