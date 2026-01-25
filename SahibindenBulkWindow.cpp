#include "stdafx.h"
#include "SahibindenBulkWindow.h"
#include "SahibindenImporter.h" // Projenizde mevcut olduğunu varsayıyoruz
#include "json.hpp"

#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstdio>
#include "Files.h"
// Win32xx namespace kullanımı
using namespace Win32xx;

#ifndef TIMER_BULK_IMPORT
#define TIMER_BULK_IMPORT 9001
#endif

// Control IDs
enum ControlIDs {
    IDC_BULK_URL = 9101,
    IDC_BULK_SPEED,
    IDC_BULK_START,
    IDC_BULK_STOP,
    IDC_BULK_PAUSE,
    IDC_BULK_LOG
};

static CString ExtractCssPseudoContent(const CString& raw)
{
    // Ornek: ".cssxxxx:before {content: 'Dürdane S.';}" -> "Dürdane S."
    CString s = raw;
    s.Trim();
    const CString key = _T("content");
    int p = s.Find(key);
    if (p < 0) return s;

    // content sonrasi ilk tirnak
    int q1 = s.Find(_T("'"), p);
    int q2 = -1;
    if (q1 >= 0) q2 = s.Find(_T("'"), q1 + 1);
    if (q1 >= 0 && q2 > q1) {
        CString out = s.Mid(q1 + 1, q2 - q1 - 1);
        out.Trim();
        if (!out.IsEmpty()) return out;
    }
    q1 = s.Find(_T("\""), p);
    if (q1 >= 0) q2 = s.Find(_T("\""), q1 + 1);
    if (q1 >= 0 && q2 > q1) {
        CString out = s.Mid(q1 + 1, q2 - q1 - 1);
        out.Trim();
        if (!out.IsEmpty()) return out;
    }
    return s;
}

static CString NormalizePhoneText(const CString& raw)
{
    // Bosluk ve parantez/.- temizle, +90 -> 0 ile baslat
    CString s = raw;
    s.Trim();
    if (s.IsEmpty()) return s;

    CString out;
    for (int i = 0; i < s.GetLength(); ++i) {
        TCHAR c = s[i];
        if ((c >= _T('0') && c <= _T('9')) || c == _T('+')) out += c;
    }
    if (out.Left(3) == _T("+90")) {
        out = _T("0") + out.Mid(3);
    }
    return out;
}
// UTF-8 std::string -> std::wstring
inline std::wstring Utf8ToW(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], len);
    return w;
}

// ANSI (ACP) std::string -> std::wstring (bazı yerlerde işine yarar)
inline std::wstring AnsiToW(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], len);
    return w;
}

// std::wstring -> UTF-8 std::string
inline std::string WToUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], len, nullptr, nullptr);
    return s;
}


namespace {
    // JS: İlan linklerini topla
    const wchar_t* kJsExtractAdLinks =
        L"(() => {\n"
        L"  const anchors = Array.from(document.querySelectorAll('a[href*=\"/ilan/\"]'));\n"
        L"  const urls = [];\n"
        L"  for (const a of anchors) {\n"
        L"    const h = a.getAttribute('href');\n"
        L"    if (!h) continue;\n"
        L"    if (h.indexOf('/ilan/') === -1) continue;\n"
        L"    if (h.indexOf('/detay') === -1) continue;\n"
        L"    const u = new URL(h, location.origin).toString();\n"
        L"    urls.push(u);\n"
        L"  }\n"
        L"  const uniq = Array.from(new Set(urls));\n"
        L"  return JSON.stringify(uniq);\n"
        L"})()";

    // JS: Sonraki sayfa
    const wchar_t* kJsNextPage =
        L"(() => {\n"
        L"  const sels = [\n"
        L"    'a[rel=\"next\"]',\n"
        L"    'a[title*=\"Sonraki\"]',\n"
        L"    'a[aria-label*=\"Sonraki\"]',\n"
        L"    'ul.pageNaviButtons li.next a',\n"
        L"    'li.next a'\n"
        L"  ];\n"
        L"  for (const s of sels) {\n"
        L"    const a = document.querySelector(s);\n"
        L"    if (a && a.href) return a.href;\n"
        L"  }\n"
        L"  return null;\n"
        L"})()";

    // JS: Ilan sayfasindan satici adi + telefon bilgisini cek.
    // Not: Sahibinden bazen ismi CSS ::before content ile basabilir.
    // Bu nedenle hem textContent hem de computedStyle(::before/::after).content okunur.
    const wchar_t* kJsExtractContact = LR"JS(
(() => {
  const clean = (s) => (s || '').replace(/\s+/g,' ').trim();
  const stripQuotes = (s) => {
    s = clean(s);
    if (!s) return '';
    // computedStyle content genelde "..." veya '...' gelir
    const c0 = s.charAt(0);
    const c1 = s.charAt(s.length - 1);
    if ((c0 === '"' && c1 === '"') || (c0 === "'" && c1 === "'"))
      return s.substring(1, s.length - 1);
    return s;
  };

  const pseudo = (el) => {
    if (!el) return '';
    try {
      const b = stripQuotes(getComputedStyle(el,'::before').content);
      if (b && b !== 'none') return b;
      const a = stripQuotes(getComputedStyle(el,'::after').content);
      if (a && a !== 'none') return a;
    } catch(e) {}
    return '';
  };

  const scope = document.querySelector('.classifiedUserBox') || document.querySelector('.user-info-area') || document.body;

  // 1) isim: once bilinen selector'lar
  const nameSelectors = [
    '.username-info-area h5',
    '.username-info-area',
    '.user-info-store-name',
    '.store-profile-title',
    '.classifiedUserBox h5',
    '.classifiedUserBox .name',
    '.user-info-area h5'
  ];

  let ownerName = '';
  for (const sel of nameSelectors) {
    const el = document.querySelector(sel);
    const t = clean(el ? el.textContent : '');
    if (t && t.length >= 2) { ownerName = t; break; }
    const p = pseudo(el);
    if (p && p.length >= 2) { ownerName = p; break; }
  }

  // 2) isim bulunmazsa: scope icinde pseudo content taramasi
  if (!ownerName && scope) {
    const els = Array.from(scope.querySelectorAll('*'));
    for (let i=0; i<els.length && i<1200; i++) {
      const p = pseudo(els[i]);
      if (p && p.length >= 2 && /[A-Za-zÇĞİÖŞÜçğıöşü]/.test(p) && !/\d/.test(p)) {
        ownerName = p;
        break;
      }
    }
  }

  // telefon
  let ownerPhone = '';
  const telA = document.querySelector('a[href^="tel:"]');
  if (telA) {
    const h = telA.getAttribute('href') || '';
    ownerPhone = clean(h.replace('tel:', ''));
  }

  const normalizePhone = (s) => {
    s = clean(s);
    if (!s) return '';
    // rakam + + kalsin
    s = s.replace(/[^0-9+]/g,'');
    // +90 ile baslarsa 0 ekleyerek normalize et
    if (s.startsWith('+90')) s = '0' + s.substring(3);
    return s;
  };

  if (!ownerPhone && scope) {
    const txt = clean(scope.innerText || '');
    const reList = [
      /(?:\+?90\s*)?0?\s*5\d{2}\s*[\-\(\)]?\s*\d{3}\s*[\-\s]?\d{2}\s*[\-\s]?\d{2}/,
      /0\s*\(?\s*\d{3}\s*\)?\s*\d{3}\s*\d{2}\s*\d{2}/
    ];
    for (const re of reList) {
      const m = txt.match(re);
      if (m && m[0]) { ownerPhone = normalizePhone(m[0]); break; }
    }
  }

  // pseudo content'ten telefon arama
  if (!ownerPhone && scope) {
    const els = Array.from(scope.querySelectorAll('*'));
    for (let i=0; i<els.length && i<1200; i++) {
      const p = pseudo(els[i]);
      if (!p) continue;
      const norm = normalizePhone(p);
      if (norm && norm.length >= 10) { ownerPhone = norm; break; }
    }
  }

  return JSON.stringify({ OwnerName: ownerName, OwnerPhone: ownerPhone });
})()
)JS";
}

CSahibindenBulkWindow::CSahibindenBulkWindow()
{
    // Basit random seed (throttle jitter icin)
    ::srand((unsigned int)::GetTickCount());
}


CSahibindenBulkWindow::~CSahibindenBulkWindow() noexcept
{
    Stop();
}

HWND CSahibindenBulkWindow::CreateBulkWindow(HWND hOwner)
{
    m_ownerHwnd = hOwner;
    // Pencere başlığı ve stili
    CString title = _T("Sahibinden Bulk Import (BrowserWindow)");

    // Win32++ CreateEx kullanımı: Varsayılan rect ile oluşturur, OnSize düzenler.
    return CreateEx(WS_EX_CONTROLPARENT, _T("Win32++ Window"), title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 800, hOwner, nullptr);
}

// Win32++'ın ana mesaj döngüsü
LRESULT CSahibindenBulkWindow::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TIMER:
        if (wParam == TIMER_BULK_IMPORT) Tick();
        return 0;

    case WM_SIZE:
    {
        const int cx = LOWORD(lParam);
        const int cy = HIWORD(lParam);
        LayoutControls(cx, cy);
        return 0;
    }

    case WM_COMMAND:
    {
        const UINT id = LOWORD(wParam);
        const UINT notify = HIWORD(wParam);

        // ComboBox secimi degisti
        if (id == IDC_BULK_SPEED && notify == CBN_SELCHANGE)
        {
            int sel = m_comboSpeed.GetCurSel();
            if (sel < 0) sel = 1;
            ApplySpeedPreset(sel);
            SaveCheckpoint(true);
            return 0;
        }

        // Buton tıklamaları vb.
        if (lParam != 0) // Kontrolden gelen mesaj
        {
            OnCommand(id);
        }
        return 0;
    }

        // WM_CREATE ve WM_DESTROY varsayılan CWnd içinde OnCreate/OnDestroy'u çağırır,
        // ancak burada açıkça handle edilmesini istersek:
    }

    return CWnd::WndProc(uMsg, wParam, lParam);
}

int CSahibindenBulkWindow::OnCreate(CREATESTRUCT& cs)
{
    // CWnd::OnCreate(cs) Win32++'da sanal değildir, doğrudan çağrılmaz, 
    // ancak CWnd::WndProc WM_CREATE gelince bu sanal metodu tetikler.

    CreateControls();

    // Varsayılan
    m_editUrl.SetWindowText(_T("https://www.sahibinden.com/satilik-daire/gaziantep-sehitkamil/sahibinden"));

    // Browser'ı başlat (HWND artık geçerli)
    m_browser.InitBrowser(GetHwnd());
    m_browser.ShowWindow(SW_SHOW);

    // Checkpoint varsa yukle (URL, hiz profili, kaldigi yer vb.)
    LoadCheckpoint();

    // UI secimine gore hiz ayarini uygula
    int sel = m_comboSpeed.GetCurSel();
    if (sel < 0) sel = 1;
    ApplySpeedPreset(sel);

    // İlk yerleşim
    RECT rc{};
    rc=GetClientRect();
    LayoutControls(rc.right - rc.left, rc.bottom - rc.top);

    // İlk yerleşim
    {
        RECT rc{};
        rc=GetClientRect();
        LayoutControls(rc.right - rc.left, rc.bottom - rc.top);
    }

    // Timer başlat
    m_timer = SetTimer(TIMER_BULK_IMPORT, 200, nullptr);

    UpdateTitleStats();

    return 0;
}

void CSahibindenBulkWindow::OnDestroy()
{
    Stop();
    if (m_timer) {
        KillTimer(TIMER_BULK_IMPORT);
        m_timer = 0;
    }
    // Win32++'da base class OnDestroy çağrısı zorunlu değildir ama temizlik için iyidir.
}

void CSahibindenBulkWindow::OnCommand(UINT id)
{
    switch (id)
    {
    case IDC_BULK_START:
        Start();
        break;
    case IDC_BULK_STOP:
        Stop();
        break;

    case IDC_BULK_PAUSE:
    {
        if (!m_running) break;
        bool paused = (bool)m_userPaused.load();
        if (!paused)
        {
            m_userPaused = true;
            m_userResumeState = m_state;
            m_state = State::PausedUser;
            m_btnPause.SetWindowText(_T("Devam"));
            LogLine(_T("[Bulk] Pause."));
            SaveCheckpoint(true);
        }
        else
        {
            m_userPaused = false;
            m_state = m_userResumeState;
            m_btnPause.SetWindowText(_T("Pause"));
            LogLine(_T("[Bulk] Devam."));
            SaveCheckpoint(true);
        }
        break;
    }
    }
}

void CSahibindenBulkWindow::CreateControls()
{
    // 1. URL Edit Kutusu (Sınıf Adı: "Edit")
    // WS_EX_CLIENTEDGE: Çökük (3D) kenarlık verir, edit kutusu gibi görünmesini sağlar.
    m_editUrl.CreateEx(WS_EX_CLIENTEDGE, _T("Edit"), _T(""),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 0, 0, // Boyutları 0 veriyoruz, LayoutControls'da ayarlanacak
        *this,      // Parent (Bu pencere)
        (HMENU)IDC_BULK_URL); // Control ID

    // Hız Profili Combo (Yavaş/Normal/Çok Yavaş)
    m_comboSpeed.CreateEx(0, _T("ComboBox"), _T(""),
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        0, 0, 0, 0,
        *this,
        (HMENU)IDC_BULK_SPEED);
    m_comboSpeed.AddString(_T("Yavaş"));
    m_comboSpeed.AddString(_T("Normal"));
    m_comboSpeed.AddString(_T("Çok Yavaş (Güvenli)"));
    m_comboSpeed.SetCurSel(1);

    // 2. Başlat Butonu (Sınıf Adı: "Button")
    m_btnStart.CreateEx(0, _T("Button"), _T("Baslat"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        0, 0, 0, 0,
        *this,
        (HMENU)IDC_BULK_START);

    // 3. Durdur Butonu
    m_btnStop.CreateEx(0, _T("Button"), _T("Durdur"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        0, 0, 0, 0,
        *this,
        (HMENU)IDC_BULK_STOP);

    // Durdur butonu başlangıçta pasif olsun
    m_btnStop.EnableWindow(FALSE);

    // 3b. Pause/Devam Butonu
    m_btnPause.CreateEx(0, _T("Button"), _T("Pause"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_VCENTER,
        0, 0, 0, 0,
        *this,
        (HMENU)IDC_BULK_PAUSE);

    m_btnPause.EnableWindow(FALSE);

    // 4. Log Listesi (Sınıf Adı: "ListBox")
    m_listLog.CreateEx(WS_EX_CLIENTEDGE, _T("ListBox"), nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        0, 0, 0, 0,
        *this,
        (HMENU)IDC_BULK_LOG);

    // 5. Browser (Özel Sınıf)
    // CSimpleBrowser muhtemelen kendi içinde Create veya CreateEx'i sarmalıyordur.
    // Eğer CWnd türevi basit bir sınıfsa:
    m_browser.Create(*this);
}
void CSahibindenBulkWindow::LayoutControls(int cx, int cy)
{
    if (cx == 0 || cy == 0) return;

    const int pad = 8;
    const int rowH = 28;
    const int btnW = 90;
    const int cmbW = 180;
    const int logH = 160;

    int x = pad;
    int y = pad;

    // URL Edit
    int editW = cx - pad * 6 - btnW * 3 - cmbW;
    if (editW < 200) editW = 200;

    // Win32++ SetWindowPos sarmalayıcısı
    if (m_editUrl.IsWindow())
        m_editUrl.SetWindowPos(nullptr, x, y, editW, rowH, SWP_NOZORDER | SWP_SHOWWINDOW);
    x += editW + pad;

    if (m_comboSpeed.IsWindow())
        m_comboSpeed.SetWindowPos(nullptr, x, y, cmbW, rowH, SWP_NOZORDER | SWP_SHOWWINDOW);
    x += cmbW + pad;

    if (m_btnStart.IsWindow())
        m_btnStart.SetWindowPos(nullptr, x, y, btnW, rowH, SWP_NOZORDER | SWP_SHOWWINDOW);
    x += btnW + pad;

    if (m_btnPause.IsWindow())
        m_btnPause.SetWindowPos(nullptr, x, y, btnW, rowH, SWP_NOZORDER | SWP_SHOWWINDOW);
    x += btnW + pad;

    if (m_btnStop.IsWindow())
        m_btnStop.SetWindowPos(nullptr, x, y, btnW, rowH, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Browser Alanı
    y += rowH + pad;
    int browserH = cy - y - logH - pad * 2;
    if (browserH < 120) browserH = 120;

    if (m_browser.IsWindow())
        m_browser.SetWindowPos(nullptr, pad, y, cx - pad * 2, browserH, SWP_NOZORDER | SWP_SHOWWINDOW);

    // Log Listesi
    y += browserH + pad;
    if (m_listLog.IsWindow())
        m_listLog.SetWindowPos(nullptr, pad, y, cx - pad * 2, logH, SWP_NOZORDER | SWP_SHOWWINDOW);
}

void CSahibindenBulkWindow::LogLine(const CString& s)
{
    if (!m_listLog.IsWindow()) return;
    int idx = m_listLog.AddString(s);
    m_listLog.SetCurSel(idx);
}

std::wstring CSahibindenBulkWindow::Trim(const std::wstring& s)
{
    size_t a = 0;
    while (a < s.size() && iswspace(s[a])) a++;
    size_t b = s.size();
    while (b > a && iswspace(s[b - 1])) b--;
    return s.substr(a, b - a);
}

DWORD CSahibindenBulkWindow::RandRange(DWORD minMs, DWORD maxMs)
{
    if (maxMs <= minMs) return minMs;
    const DWORD span = (maxMs - minMs) + 1;
    const DWORD r = (DWORD)(::rand() % (int)span);
    return minMs + r;
}

void CSahibindenBulkWindow::ScheduleNextAction(DWORD delayMs)
{
    const DWORD now = ::GetTickCount();
    m_nextActionTick = now + delayMs;
}

void CSahibindenBulkWindow::PauseWithBackoff(const CString& reason, State resumeState)
{
    // Engel sayisi arttikca bekleme suresi artar (cap var)
    m_blockCount++;
    // exponential-ish: base * 2^(blockCount-1)
    DWORD mult = 1;
    for (int i = 1; i < m_blockCount; ++i)
    {
        if (mult > 1024) break; // safety
        mult *= 2;
    }
    unsigned long long waitMs = (unsigned long long)m_backoffBaseMs * (unsigned long long)mult;
    if (waitMs > m_backoffMaxMs) waitMs = m_backoffMaxMs;

    // jitter ekle (0.8x - 1.2x)
    const DWORD jitter = RandRange((DWORD)(waitMs * 8 / 10), (DWORD)(waitMs * 12 / 10));

    CString msg;
    msg.Format(_T("[Bulk] %s. Bekleme: %u sn (engel sayisi=%d)"), reason.c_str(), (unsigned)(jitter/1000), m_blockCount);
    LogLine(msg);

    m_statBackoffCount++;
    UpdateTitleStats();

    m_resumeState = resumeState;
    m_state = State::PausedBot;
    ScheduleNextAction(jitter);

    SaveCheckpoint(true);
}

void CSahibindenBulkWindow::Start()
{
    if (m_running) return;

    CString url = m_editUrl.GetWindowText();
    if (url.IsEmpty()) {
        MessageBox(_T("Lutfen bir arama/liste URL gir."), _T("Bulk Import"), MB_OK | MB_ICONWARNING);
        return;
    }

    // Checkpoint'ten devam
    const bool resume = (m_checkpointLoaded && m_checkpointWasRunning && !m_currentListUrl.empty());

    m_running = true;
    m_stateStartTick = GetTickCount();

    // gunluk kota tarih kontrolu
    {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        wchar_t buf[32]{};
        swprintf(buf, 32, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        std::wstring today(buf);
        if (m_dailyDate != today) {
            m_dailyDate = today;
            m_dailyCount = 0;
        }
    }

    if (!resume)
    {
        // Temiz baslangic
        m_adUrls.clear();
        m_adIndex = 0;
        m_nextListUrl.clear();
        m_currentListUrl = (LPCWSTR)url;
        m_pageCount = 0;
        m_blockCount = 0;
        m_adsSinceLongPause = 0;

        m_statFoundLinks = 0;
        m_statSavedOk = 0;
        m_statSavedFail = 0;
        m_statBackoffCount = 0;
        m_statLongPauseCount = 0;
    }

    m_state = State::LoadList;
    ScheduleNextAction(0);

    m_btnStart.EnableWindow(FALSE);
    m_btnStop.EnableWindow(TRUE);
    m_btnPause.EnableWindow(TRUE);
    m_btnPause.SetWindowText(_T("Pause"));
    m_userPaused = false;

    LogLine(resume ? _T("[Bulk] Checkpoint'ten devam ediliyor...") : _T("[Bulk] Baslatiliyor..."));
    LogLine(_T("[Bulk] Liste sayfasi yukleniyor..."));

    // Resume'da: elimizde m_currentListUrl var
    CString nav = resume ? CString(m_currentListUrl.c_str()) : url;
    m_browser.Navigate(nav);

    m_checkpointWasRunning = false; // artik calisiyor
    SaveCheckpoint(true);
}

void CSahibindenBulkWindow::Stop()
{
    if (!m_running) return;

    m_running = false;
    m_state = State::Idle;

    m_btnStart.EnableWindow(TRUE);
    m_btnStop.EnableWindow(FALSE);
    m_btnPause.EnableWindow(FALSE);
    m_btnPause.SetWindowText(_T("Pause"));
    m_userPaused = false;

    LogLine(_T("[Bulk] Durduruldu."));

    // Gunluk rapor
    {
        std::wofstream f(GetDailyReportPath(), std::ios::app);
        if (f) {
            f << L"\n=== Bulk Import Report ===\n";
            f << L"Found links: " << m_statFoundLinks << L"\n";
            f << L"Saved ok:    " << m_statSavedOk << L"\n";
            f << L"Saved fail:  " << m_statSavedFail << L"\n";
            f << L"Backoffs:    " << m_statBackoffCount << L"\n";
            f << L"Long pauses: " << m_statLongPauseCount << L"\n";
            f << L"Daily count: " << m_dailyCount << L" / " << m_dailyQuotaMax << L"\n";
        }
    }

    SaveCheckpoint(true);
    UpdateTitleStats();
}

void CSahibindenBulkWindow::Tick()
{
    if (!m_running) return;

    const DWORD now = ::GetTickCount();

    // Genel zamanlayici: planlanan zamandan once hicbir sey yapma
    if (m_nextActionTick != 0 && now < m_nextActionTick)
        return;

    switch (m_state)
    {
    case State::LoadList:
        // Liste sayfasi navigation tetiklendi, biraz bekleyip DOM'u hazirla
        m_state = State::WaitListReady;
        ScheduleNextAction(RandRange(m_listReadyMinMs, m_listReadyMaxMs));
        break;

    case State::WaitListReady:
        RequestListingLinks();
        break;

    case State::LoadAd:
        // Ilan sayfasi navigation tetiklendi, insan gibi biraz bekle
        m_state = State::WaitAdReady;
        ScheduleNextAction(RandRange(m_adReadyMinMs, m_adReadyMaxMs));
        break;

    case State::WaitAdReady:
        RequestAdJson();
        break;

    case State::NextAd:
        NavigateToAd();
        break;

    case State::PausedUser:
        // Kullanici pause ettiginde hicbir sey yapma.
        return;

    case State::PausedBot:
        // Bekleme suresi dolunca kaldigi yerden devam et
        if (m_resumeState == State::LoadAd)
        {
            // Ayni ilan URL'sini yeniden ac
            if (m_adIndex < m_adUrls.size())
            {
                CString u(m_adUrls[m_adIndex].c_str());
                LogLine(_T("[Bulk] Yeniden deneniyor (ilan)..."));
                m_browser.Navigate(u);
                m_state = State::LoadAd;
                ScheduleNextAction(RandRange(m_adReadyMinMs, m_adReadyMaxMs));
            }
            else
            {
                m_state = State::LoadList;
                ScheduleNextAction(RandRange(m_listReadyMinMs, m_listReadyMaxMs));
            }
        }
        else
        {
            // Listeyi yeniden ac
            if (!m_currentListUrl.empty())
            {
                LogLine(_T("[Bulk] Yeniden deneniyor (liste)..."));
                m_browser.Navigate(m_currentListUrl.c_str());
            }
            m_state = State::LoadList;
            ScheduleNextAction(RandRange(m_listReadyMinMs, m_listReadyMaxMs));
        }
        break;

    default:
        break;
    }
}

void CSahibindenBulkWindow::RequestListingLinks()
{
    LogLine(_T("[Bulk] Linkler cekiliyor..."));

    m_browser.ExecuteScriptGetResult(kJsExtractAdLinks, [this](std::wstring res) {
        if (!m_running) return;

        res = Trim(res);
        if (res.empty() || res == L"null" || res == L"undefined") {
            LogLine(_T("[Bulk] Link bulunamadi, tekrar denenecek..."));
            ScheduleNextAction(RandRange(3000, 7000));
            return;
        }

        try {
            int len = WideCharToMultiByte(CP_UTF8, 0, res.c_str(), (int)res.size(), nullptr, 0, nullptr, nullptr);
            std::string utf8(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, res.c_str(), (int)res.size(), utf8.data(), len, nullptr, nullptr);

            nlohmann::json j = nlohmann::json::parse(utf8);
            if (!j.is_array() || j.empty()) {
                LogLine(_T("[Bulk] Liste bos gorunuyor, tekrar denenecek..."));
                ScheduleNextAction(RandRange(4000, 9000));
                return;
            }

            m_adUrls.clear();
            for (auto& it : j) {
                if (it.is_string()) {
                    std::string u8 = it.get<std::string>();
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), (int)u8.size(), nullptr, 0);
                    std::wstring wu(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), (int)u8.size(), &wu[0], wlen);
                    m_adUrls.push_back(wu);
                }
            }

            CString msg;
            msg.Format(_T("[Bulk] %d ilan linki bulundu."), (int)m_adUrls.size());
            LogLine(msg);

            m_statFoundLinks += (int)m_adUrls.size();
            UpdateTitleStats();
            SaveCheckpoint(true);

            m_browser.ExecuteScriptGetResult(kJsNextPage, [this](std::wstring next) {
                if (!m_running) return;
                next = Trim(next);
                if (!next.empty() && next != L"null" && next != L"undefined") {
                    m_nextListUrl = next;
                }
                else {
                    m_nextListUrl.clear();
                }

                m_adIndex = 0;

                // Insan gibi kısa bir nefes: hemen ilana zıplama
                m_state = State::NextAd;
                ScheduleNextAction(RandRange(2000, 5000));
                LogLine(_T("[Bulk] Ilanlara gecmeden once kisa bekleme..."));
                SaveCheckpoint(true);
                });
        }
        catch (...) {
            LogLine(_T("[Bulk] Link parse hatasi."));
            ScheduleNextAction(RandRange(4000, 9000));
        }
        });
}

void CSahibindenBulkWindow::NavigateToAd()
{
    if (!m_running) return;

    if (m_adIndex >= m_adUrls.size()) {
        RequestNextPage();
        return;
    }

    CString msg;
    msg.Format(_T("[Bulk] Ilan aciliyor (%d/%d)"), (int)(m_adIndex + 1), (int)m_adUrls.size());
    LogLine(msg);

    m_state = State::LoadAd;
    ScheduleNextAction(0);
    m_stateStartTick = GetTickCount();

    CString url(m_adUrls[m_adIndex].c_str());
    m_browser.Navigate(url);
}

void CSahibindenBulkWindow::RequestAdJson()
{
    if (!m_running) return;

    m_browser.GetListingJSON([this](std::wstring jsonText) {
        if (!m_running) return;
        std::wstring t = Trim(jsonText);
        if (t.empty() || t == L"null" || t == L"undefined") {
            // Henuz hazir degil: insan gibi biraz bekleyip tekrar dene
            ScheduleNextAction(RandRange(1500, 3500));
            return;
        }
        RequestAdHtmlAndSave(jsonText);
        });
}
static std::string WStringToUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], len, nullptr, nullptr);
    return s;
}

static CString Utf8ToCString(const std::string& s)
{
    if (s.empty()) return CString();

    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], len);

    return CString(w.c_str());
}











void CSahibindenBulkWindow::RequestAdHtmlAndSave(const std::wstring& jsonText)
{
    if (!m_running) return;

    // Index guard (out-of-range'e karsi)
    if (m_adIndex >= (int)m_adUrls.size())
    {
        LogLine(_T("[Bulk] Islem tamamlandi (index bitti)."));
        Stop();
        return;
    }

    // 1) Oncelik: satıcı adı + telefon (sayfada acik gorunuyorsa) JS ile cek.
    m_browser.ExecuteScriptGetResult(kJsExtractContact, [this, jsonText](std::wstring contactJson)
        {
            if (!m_running) return;

            CString manualName;
            CString manualPhone;

            // contactJson genelde JSON string döner: {"OwnerName":"...","OwnerPhone":"..."}
            // Bazen "null"/"undefined" dönebilir.
            std::wstring t = Trim(contactJson);
            if (!t.empty() && t != L"null" && t != L"undefined")
            {
                try
                {
                    const std::string utf8 = WStringToUtf8(t);
                    nlohmann::json cj = nlohmann::json::parse(utf8, nullptr, false);

                    if (cj.is_object())
                    {
                        auto itName = cj.find("OwnerName");
                        if (itName != cj.end() && itName->is_string())
                            manualName = ExtractCssPseudoContent(Utf8ToCString(itName->get<std::string>()));

                        auto itPhone = cj.find("OwnerPhone");
                        if (itPhone != cj.end() && itPhone->is_string())
                            manualPhone = NormalizePhoneText(ExtractCssPseudoContent(Utf8ToCString(itPhone->get<std::string>())));

                        // Istersen OwnerMobile da ekleyebilirsin:
                        // auto itMob = cj.find("OwnerMobile");
                        // ...
                    }
                }
                catch (...)
                {
                    // ignore: manual alanlar bos kalabilir, ana kayit yine devam eder
                }
            }

            // 2) HTML kaynak kodunu al, JSON + HTML + (varsa) satıcı bilgileri ile importer'a ver.
            m_browser.GetSourceCode([this, jsonText, manualName, manualPhone](std::wstring html)
                {
                    if (!m_running) return;

                    // Bot koruma / olagan disi erisim tespiti
                    if (html.find(L"Olağan dışı erişim tespit ettik") != std::wstring::npos ||
                        html.find(L"Olagan disi erisim tespit ettik") != std::wstring::npos)
                    {
                        PauseWithBackoff(_T("Olagan disi erisim engeli"), State::LoadAd);
                        return;
                    }

                    if (html.find(L"Just a moment") != std::wstring::npos ||
                        html.find(L"Attention Required") != std::wstring::npos)
                    {
                        PauseWithBackoff(_T("Bot korumasi algilandi"), State::LoadAd);
                        return;
                    }

                    // json/html utf8'e cevir
                    const std::string utf8Json = WStringToUtf8(jsonText);
                    const std::string utf8Html = WStringToUtf8(html);

                    // URL
                    CString url(m_adUrls[m_adIndex].c_str());

                    SahibindenImporter importer;

                    bool ok = importer.ImportFromJsonAndHtmlString(
                        url,
                        utf8Json,
                        utf8Html,
                        manualName,
                        manualPhone,
                        [this](const CString& line)
                        {
                            this->LogLine(line);
                        });

                    LogLine(ok ? _T("[Bulk] Kaydedildi.") : _T("[Bulk] Kayit basarisiz."));

                    // ✅ UI yenileme: Bulk import DB'ye yazdiysa ana view'lara haber ver.
                    // MainFrame UWM_DATA_CHANGED yakalayip m_view.RefreshList() cagiriyor.
                    if (ok && m_ownerHwnd)
                        ::PostMessage(m_ownerHwnd, UWM_DATA_CHANGED, 0, 0);

                    // Istatistik + kota
                    if (ok) {
                        m_statSavedOk++;
                        m_dailyCount++;
                    }
                    else {
                        m_statSavedFail++;
                    }
                    UpdateTitleStats();
                    SaveCheckpoint();

                    if (m_dailyQuotaMax > 0 && m_dailyCount >= m_dailyQuotaMax) {
                        LogLine(_T("[Bulk] Gunluk kota doldu. Program kapanacak, yarin kaldigi yerden devam edecek."));
                        m_checkpointWasRunning = true;
                        SaveCheckpoint(true);
                        // Pencereyi kapat (app kendi mesaj dongusune gore kapanir)
                        PostMessage(WM_CLOSE, 0, 0);
                        return;
                    }

                    // Sonraki ilan (insan gibi bekle)
                    m_adIndex++;
                    m_adsSinceLongPause++;

                    DWORD delay = RandRange(m_betweenAdsMinMs, m_betweenAdsMaxMs);
                    if (m_adsSinceLongPause >= m_longPauseEveryAds) {
                        delay = RandRange(m_longPauseMinMs, m_longPauseMaxMs);
                        m_adsSinceLongPause = 0;
                        LogLine(_T("[Bulk] Kisa mola (ban riskini azaltmak icin)..."));
                        m_statLongPauseCount++;
                    }

                    m_state = State::NextAd;
                    ScheduleNextAction(delay);
                });
        });
}

void CSahibindenBulkWindow::RequestNextPage()
{
    if (!m_running) return;

    m_pageCount++;
    if (m_nextListUrl.empty() || m_pageCount >= m_maxPages) {
        LogLine(_T("[Bulk] Tamamlandi."));
        Stop();
        return;
    }

    LogLine(_T("[Bulk] Sonraki sayfaya geciliyor..."));
    m_currentListUrl = m_nextListUrl;
    m_browser.Navigate(m_nextListUrl.c_str());

    m_state = State::LoadList;
    ScheduleNextAction(0);
    m_stateStartTick = GetTickCount();
    m_adUrls.clear();
    m_adIndex = 0;
    m_nextListUrl.clear();
}

// =============================================================
// Checkpoint / Profil / Rapor
// =============================================================

std::wstring CSahibindenBulkWindow::GetExeDir()
{
    wchar_t path[MAX_PATH]{ };
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring p(path);
    size_t pos = p.find_last_of(L"\\/");
    if (pos != std::wstring::npos) p = p.substr(0, pos);
    return p;
}

std::wstring CSahibindenBulkWindow::GetCheckpointPath() const
{
    return GetExeDir() + L"\\SahibindenBulkCheckpoint.json";
}

std::wstring CSahibindenBulkWindow::GetDailyReportPath() const
{
    // tarih bazli rapor
    SYSTEMTIME st{};
    GetLocalTime(&st);
    wchar_t buf[64]{};
    swprintf(buf, 64, L"BulkReport_%04d%02d%02d.txt", st.wYear, st.wMonth, st.wDay);
    return GetExeDir() + L"\\" + std::wstring(buf);
}

void CSahibindenBulkWindow::ClearCheckpoint()
{
    try {
        _wremove(GetCheckpointPath().c_str());
    } catch (...) {}
    m_checkpointLoaded = false;
    m_checkpointWasRunning = false;
}

void CSahibindenBulkWindow::LoadCheckpoint()
{
    m_checkpointLoaded = false;
    m_checkpointWasRunning = false;

    std::ifstream f(GetCheckpointPath());
    if (!f) return;

    nlohmann::json j;
    try {
        f >> j;
    }
    catch (...) {
        return;
    }

    try {
        if (j.contains("url") && j["url"].is_string()) {
            std::wstring u = Utf8ToW(j["url"].get<std::string>());
            if (!u.empty()) {
                m_editUrl.SetWindowText(u.c_str());
            }
        }
        if (j.contains("speedPreset") && j["speedPreset"].is_number_integer()) {
            m_speedPreset = j["speedPreset"].get<int>();
            if (m_speedPreset < 0) m_speedPreset = 1;
            if (m_speedPreset > 2) m_speedPreset = 2;
            m_comboSpeed.SetCurSel(m_speedPreset);
        }

        if (j.contains("dailyQuotaMax") && j["dailyQuotaMax"].is_number_integer())
            m_dailyQuotaMax = j["dailyQuotaMax"].get<int>();
        if (j.contains("dailyCount") && j["dailyCount"].is_number_integer())
            m_dailyCount = j["dailyCount"].get<int>();
        if (j.contains("dailyDate") && j["dailyDate"].is_string())
            m_dailyDate = Utf8ToW(j["dailyDate"].get<std::string>());

        if (j.contains("currentListUrl") && j["currentListUrl"].is_string())
            m_currentListUrl = Utf8ToW(j["currentListUrl"].get<std::string>());
        if (j.contains("nextListUrl") && j["nextListUrl"].is_string())
            m_nextListUrl = Utf8ToW(j["nextListUrl"].get<std::string>());
        if (j.contains("pageCount") && j["pageCount"].is_number_integer())
            m_pageCount = j["pageCount"].get<int>();

        if (j.contains("adIndex") && j["adIndex"].is_number_integer())
            m_adIndex = (size_t)j["adIndex"].get<int>();

        if (j.contains("adUrls") && j["adUrls"].is_array()) {
            m_adUrls.clear();
            for (auto& it : j["adUrls"]) {
                if (it.is_string()) {
                    m_adUrls.push_back(Utf8ToW(it.get<std::string>()));
                }
            }
        }

        if (j.contains("blockCount") && j["blockCount"].is_number_integer())
            m_blockCount = j["blockCount"].get<int>();
        if (j.contains("adsSinceLongPause") && j["adsSinceLongPause"].is_number_integer())
            m_adsSinceLongPause = j["adsSinceLongPause"].get<int>();

        if (j.contains("statFoundLinks") && j["statFoundLinks"].is_number_integer())
            m_statFoundLinks = j["statFoundLinks"].get<int>();
        if (j.contains("statSavedOk") && j["statSavedOk"].is_number_integer())
            m_statSavedOk = j["statSavedOk"].get<int>();
        if (j.contains("statSavedFail") && j["statSavedFail"].is_number_integer())
            m_statSavedFail = j["statSavedFail"].get<int>();
        if (j.contains("statBackoff") && j["statBackoff"].is_number_integer())
            m_statBackoffCount = j["statBackoff"].get<int>();
        if (j.contains("statLongPause") && j["statLongPause"].is_number_integer())
            m_statLongPauseCount = j["statLongPause"].get<int>();

        if (j.contains("wasRunning") && j["wasRunning"].is_boolean())
            m_checkpointWasRunning = j["wasRunning"].get<bool>();

        if (j.contains("userPaused") && j["userPaused"].is_boolean())
            m_userPaused = j["userPaused"].get<bool>();
        if (j.contains("userResumeState") && j["userResumeState"].is_number_integer())
            m_userResumeState = (State)j["userResumeState"].get<int>();

        // UI
        if ((bool)m_userPaused) { m_state = State::PausedUser; m_btnPause.SetWindowText(_T("Devam")); }

        m_checkpointLoaded = true;
        LogLine(_T("[Bulk] Checkpoint yuklendi."));
    }
    catch (...) {
        // ignore
    }
}

void CSahibindenBulkWindow::SaveCheckpoint(bool force)
{
    const DWORD now = GetTickCount();
    if (!force) {
        if (m_lastCheckpointTick != 0 && (now - m_lastCheckpointTick) < m_checkpointEveryMs)
            return;
    }
    m_lastCheckpointTick = now;

    nlohmann::json j;
    try {
        CString url = m_editUrl.GetWindowText();
        j["url"] = WToUtf8((LPCWSTR)url);
        j["speedPreset"] = m_speedPreset;
        j["dailyQuotaMax"] = m_dailyQuotaMax;
        j["dailyCount"] = m_dailyCount;
        j["dailyDate"] = WToUtf8(m_dailyDate);

        j["currentListUrl"] = WToUtf8(m_currentListUrl);
        j["nextListUrl"] = WToUtf8(m_nextListUrl);
        j["pageCount"] = m_pageCount;
        j["adIndex"] = (int)m_adIndex;

        nlohmann::json arr = nlohmann::json::array();
        for (auto& u : m_adUrls) arr.push_back(WToUtf8(u));
        j["adUrls"] = arr;

        j["blockCount"] = m_blockCount;
        j["adsSinceLongPause"] = m_adsSinceLongPause;

        j["statFoundLinks"] = m_statFoundLinks;
        j["statSavedOk"] = m_statSavedOk;
        j["statSavedFail"] = m_statSavedFail;
        j["statBackoff"] = m_statBackoffCount;
        j["statLongPause"] = m_statLongPauseCount;

        j["wasRunning"] = (bool)(m_running ? true : m_checkpointWasRunning);

        j["userPaused"] = (bool)m_userPaused.load();
        j["userResumeState"] = (int)m_userResumeState;

        std::ofstream f(GetCheckpointPath(), std::ios::trunc);
        if (f) f << j.dump(2);
    }
    catch (...) {
        // ignore
    }
}

void CSahibindenBulkWindow::ApplySpeedPreset(int presetIndex)
{
    m_speedPreset = presetIndex;

    // Baz degerler (Normal)
    int longEvery = 10;
    DWORD longMin = 30000;
    DWORD longMax = 90000;
    DWORD listMin = 2000, listMax = 4500;
    DWORD adMin = 3000, adMax = 6500;
    DWORD betweenMin = 4500, betweenMax = 12000;

    double mult = 1.0;
    if (presetIndex == 0) mult = 1.5;          // Yavas
    else if (presetIndex == 2) mult = 2.5;     // Cok yavas

    // Cok yavas profilde daha sik mola
    if (presetIndex == 2) {
        longEvery = 7;
        longMin = 45000;
        longMax = 150000;
    }

    m_longPauseEveryAds = longEvery;
    m_longPauseMinMs = (DWORD)(longMin * mult);
    m_longPauseMaxMs = (DWORD)(longMax * mult);

    m_listReadyMinMs = (DWORD)(listMin * mult);
    m_listReadyMaxMs = (DWORD)(listMax * mult);
    m_adReadyMinMs = (DWORD)(adMin * mult);
    m_adReadyMaxMs = (DWORD)(adMax * mult);
    m_betweenAdsMinMs = (DWORD)(betweenMin * mult);
    m_betweenAdsMaxMs = (DWORD)(betweenMax * mult);
}

void CSahibindenBulkWindow::UpdateTitleStats()
{
    CString title;
    title.Format(_T("Sahibinden Bulk Import | OK:%d FAIL:%d | Today:%d/%d | Backoff:%d"),
        m_statSavedOk, m_statSavedFail, m_dailyCount, m_dailyQuotaMax, m_statBackoffCount);
    SetWindowText(title);
}