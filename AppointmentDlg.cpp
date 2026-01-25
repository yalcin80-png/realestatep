#include "stdafx.h"
#include "AppointmentDlg.h"
#include "RibbonApp.h"
#include "Mainfrm.h"
#include <thread>

CAppointmentDlg::CAppointmentDlg(const CString& custID, const CString& custName, const CString& propID, const CString& propInfo)
    : CDialog(IDD_APPOINTMENT_DIALOG), m_custID(custID), m_custName(custName), m_propID(propID), m_propInfo(propInfo)
{
}

BOOL CAppointmentDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 1. Durum (Status) Kutusunu Doldur
    HWND hCombo = GetDlgItem(IDC_APP_STATUS);
    if (hCombo)
    {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Bekliyor"));
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Tamamlandı"));
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("İptal"));
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Ertelendi"));

        // Varsayılan olarak "Bekliyor" seç
        SendMessage(hCombo, CB_SELECTSTRING, -1, (LPARAM)_T("Bekliyor"));
    }

    // 2. Başlık Ayarı ve Mod Kontrolü
    if (m_isEditMode && !m_appID.IsEmpty())
    {
        SetWindowText(_T("Randevu Detayları / Düzenle"));

        // --- DÜZENLEME MODU ---
        // Veritabanından mevcut kaydı çek
        Appointment_cstr app = m_db.GetRecordByCode<Appointment_cstr>(m_appID);

        // UI Kontrollerini Otomatik Doldur (DatabaseManager yeteneği)
        m_db.Bind_Data_To_UI(this, app);

        // Mülk ve Müşteri Bilgilerini Elle Güncelle (Struct'ta ID yazar ama biz Ekrana Bilgi yazacağız)
        m_custID = app.CustomerID;
        m_propID = app.PropertyID;
        m_propType = app.PropertyType;

        // Müşteri Adını Bul (Opsiyonel: DB'den tekrar çekmek gerekebilir)
        Customer_cstr cust = m_db.GetCustomerByCariKod(m_custID);
        m_custName = cust.AdSoyad;

        // Ekrana Yaz
        SetDlgItemText(IDC_APP_CUST_ID, m_custID);
        SetDlgItemText(IDC_APP_CUST_INFO, m_custName);

        // Mülk Bilgisi
        CString propDisplay = m_propType + _T(" - ") + m_propID;
        SetDlgItemText(IDC_APP_PROP_ID, m_propID);
        SetDlgItemText(IDC_APP_PROP_INFO, propDisplay);
    }
    else
    {
        SetWindowText(_T("Yeni Randevu Oluştur"));

        // --- YENİ KAYIT MODU ---
        // Dışarıdan gelen (Constructor) bilgileri bas
        SetDlgItemText(IDC_APP_CUST_ID, m_custID);
        SetDlgItemText(IDC_APP_CUST_INFO, m_custName);

        if (!m_propID.IsEmpty()) {
            SetDlgItemText(IDC_APP_PROP_ID, m_propID);
            SetDlgItemText(IDC_APP_PROP_INFO, m_propInfo);
        }
        else {
            SetDlgItemText(IDC_APP_PROP_INFO, _T("Mülk Seçilmedi"));
        }

        // Varsayılan Başlık Önerisi
        CString defTitle;
        defTitle.Format(_T("Randevu: %s"), m_custName.GetString());
        SetDlgItemText(IDC_APP_TITLE, defTitle);

        // Tarih/Saat kontrolünü şimdiki zamana ayarla
        SYSTEMTIME st; GetLocalTime(&st);
        SendDlgItemMessage(IDC_APP_DATE, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
        SendDlgItemMessage(IDC_APP_TIME, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
    }

    return TRUE;
}


void CAppointmentDlg::OnOK()
{
    // 1. Verileri UI'dan Topla (Map)
    std::map<CString, CString> dataMap;
    m_db.Bind_UI_To_Data(this, TABLE_NAME_APPOINTMENT, dataMap);

    // 2. Validasyon (Zorunlu Alanlar)
    if (dataMap[_T("Title")].IsEmpty()) {
        MessageBox(_T("Lütfen bir başlık giriniz."), _T("Eksik Bilgi"), MB_ICONWARNING);
        return;
    }

    // 3. Tarih ve Saat Birleştirme (Hassas İşlem)
    // DateTimePicker kontrollerinden veriyi ham olarak alıp birleştiriyoruz.
    SYSTEMTIME stDate = { 0 }, stTime = { 0 };
    SendDlgItemMessage(IDC_APP_DATE, DTM_GETSYSTEMTIME, 0, (LPARAM)&stDate);
    SendDlgItemMessage(IDC_APP_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&stTime);

    // DB Formatı: "YYYY-MM-DD HH:MM"
    CString sAppDate;
    sAppDate.Format(_T("%04d-%02d-%02d %02d:%02d"),
        stDate.wYear, stDate.wMonth, stDate.wDay, stTime.wHour, stTime.wMinute);

    dataMap[_T("AppDate")] = sAppDate;

    // 4. Eksik ID ve Tip Bilgilerini Tamamla
    // UI binding bazen readonly alanları veya hidden ID'leri alamayabilir, manuel garantiye alalım.
    dataMap[_T("CustomerID")] = m_custID;
    dataMap[_T("PropertyID")] = m_propID;
    dataMap[_T("PropertyType")] = m_propType;

    // 5. Sync Verilerini Hazırla
    CString nowIso = m_db.GetCurrentIsoUtc();
    dataMap[_T("Updated_At")] = nowIso;

    if (!m_isEditMode) {
        // Yeni kayıtsa ID üret
        dataMap[_T("sync_id")] = m_db.GenerateGuidString();
        dataMap[_T("Deleted")] = _T("0");
        dataMap[_T("IsNotified")] = _T("0");
    }
    // Edit modunda sync_id ve Deleted korunur, sadece Updated_At değişir.

    // 6. Veritabanı Struct'ını Doldur
    Appointment_cstr app;
    for (const auto& [key, val] : dataMap) {
        DatabaseManager::SetFieldByStringName(app, key, val);
    }

    // 7. Veritabanına Yaz
    bool success = false;
    m_db.EnsureConnection();

    if (m_isEditMode) {
        app.AppID = m_appID; // Hangi kaydı güncelleyeceğimizi bilsin
        success = m_db.UpdateGlobal(app);
    }
    else {
        success = m_db.InsertGlobal(app);
    }

    // 8. Sonuç ve Google Takvim İşlemi
    if (success)
    {
        // --- GOOGLE CALENDAR ENTEGRASYONU ---
        auto loginPtr = GetContainerApp()->GetMainFrame().m_login; // Erişim yolunuzu kontrol edin

        if (loginPtr && loginPtr->IsLoggedIn())
        {
            std::string token = loginPtr->GetAccessToken();

            // Thread parametrelerini hazırla (CString -> std::string dönüşümü thread içinde yapılacak)
            CString title = app.Title;
            CString desc = app.Description;

            // Google için Başlangıç ve Bitiş Zamanı (ISO 8601)
            // Örnek: "2023-12-01T14:30:00" (Yerel saat varsayıyoruz, Google UTC isteyebilir, "Z" eklemiyoruz)
            CString sStart;
            sStart.Format(_T("%04d-%02d-%02dT%02d:%02d:00"), stDate.wYear, stDate.wMonth, stDate.wDay, stTime.wHour, stTime.wMinute);

            // Bitiş saati (Varsayılan 1 saat sonrası)
            // Basitlik için sadece saati 1 artırıyoruz, gün dönümü mantığı eklemiyorum.
            int endHour = stTime.wHour + 1;
            if (endHour >= 24) endHour = 23; // Gün taşmasın

            CString sEnd;
            sEnd.Format(_T("%04d-%02d-%02dT%02d:%02d:00"), stDate.wYear, stDate.wMonth, stDate.wDay, endHour, stTime.wMinute);

            // Arka planda gönder
            std::thread t([token, title, desc, sStart, sEnd]() {
                try {
                    GoogleServices google(token);
                    google.CreateEvent(
                        DatabaseManager::GetInstance().CStringToAnsi(title),
                        DatabaseManager::GetInstance().CStringToAnsi(desc),
                        DatabaseManager::GetInstance().CStringToAnsi(sStart), // Start
                        DatabaseManager::GetInstance().CStringToAnsi(sEnd)    // End
                    );
                }
                catch (...) {}
                });
            t.detach();
        }

        MessageBox(_T("Randevu başarıyla kaydedildi."), _T("Bilgi"), MB_OK | MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else
    {
        MessageBox(_T("Veritabanına kayıt sırasında hata oluştu!"), _T("Hata"), MB_ICONERROR);
    }
}
void CAppointmentDlg::OnPrint()
{
    // 1. Verileri Hazırla
    PreviewItem item;
    item.docType = DOC_APPOINTMENT_FORM; // Fabrika bunu tanıyacak
    item.title = _T("Randevu Formu");

    // Ekranda ne görüyorsak onu gönderiyoruz
    CString val;

    val = GetDlgItemText(IDC_APP_TITLE);
    item.fields.push_back({ _T("Title"), val });

    // Tarih + Saat
    SYSTEMTIME stDate, stTime;
    DateTime_GetSystemtime(GetDlgItem(IDC_APP_DATE), &stDate);
    DateTime_GetSystemtime(GetDlgItem(IDC_APP_TIME), &stTime);
    CString sDate; sDate.Format(_T("%02d.%02d.%04d %02d:%02d"), stDate.wDay, stDate.wMonth, stDate.wYear, stTime.wHour, stTime.wMinute);
    item.fields.push_back({ _T("Date"), sDate });

    val = GetDlgItemText(IDC_APP_LOCATION);
    item.fields.push_back({ _T("Location"), val });

    val = GetDlgItemText(IDC_APP_CUST_INFO);
    item.fields.push_back({ _T("CustName"), val });

    // Telefon bilgisini DB'den çekmek gerekebilir veya struct'tan al
    // Customer_cstr cust = m_db.GetCustomerByCariKod(m_custID);
    // item.fields.push_back({ _T("CustTel"), cust.Telefon });

    val = GetDlgItemText(IDC_APP_PROP_INFO);
    item.fields.push_back({ _T("PropInfo"), val });

    val = GetDlgItemText(IDC_APP_DESC);
    item.fields.push_back({ _T("Notes"), val });

    // 2. Önizleme Ekranını Aç (MainFrame üzerinden)
    CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();

    // Diyalogu kapatıp ana ekrana dönmek istersen:
    EndDialog(IDOK);

    // Önizleme Moduna Geç
    mainFrame.GetPreviewDlg().SetPreviewData(item);
    mainFrame.SetPreviewView(); // Görünümü değiştir
}
INT_PTR CAppointmentDlg::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_BTN_APP_PRINT) {
        OnPrint();
        return TRUE;
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}

BOOL CAppointmentDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_BTN_SELECT_PROP:
        OnSelectProperty();
        return TRUE;
    case IDC_PREVIEW_PRINT:
        OnPrintToDlg();
        break;
    }
    return FALSE; // Win32++ default işleyişine bırak
}





void CAppointmentDlg::OnPrintToDlg()
{
    // 1. Yazdırma Paketini Hazırla
    PreviewItem item;
    item.docType = DOC_APPOINTMENT_FORM; // LayoutFactory bunu tanıyıp Randevu Formu çizecek
    item.title = _T("Randevu Formu");

    // --- UI ÜZERİNDEKİ VERİLERİ AL ---

    // Başlık
    CString val;
    val = GetDlgItemText(IDC_APP_TITLE);
    item.fields.push_back({ _T("Title"), val });

    // Tarih ve Saat (DateTimePicker'dan)
    SYSTEMTIME stDate = { 0 }, stTime = { 0 };
    DateTime_GetSystemtime(GetDlgItem(IDC_APP_DATE), &stDate);
    DateTime_GetSystemtime(GetDlgItem(IDC_APP_TIME), &stTime);

    CString sDate;
    sDate.Format(_T("%02d.%02d.%04d %02d:%02d"),
        stDate.wDay, stDate.wMonth, stDate.wYear, stTime.wHour, stTime.wMinute);
    item.fields.push_back({ _T("Date"), sDate });

    // Konum
    val = GetDlgItemText(IDC_APP_LOCATION);
    item.fields.push_back({ _T("Location"), val });

    // Mülk Bilgisi
    val = GetDlgItemText(IDC_APP_PROP_INFO);
    item.fields.push_back({ _T("PropInfo"), val });

    // Notlar
    val = GetDlgItemText(IDC_APP_DESC);
    item.fields.push_back({ _T("Notes"), val });

    // --- VERİTABANINDAN EK BİLGİ AL ---

    // Müşteri Adı (UI'da var)
    val =  GetDlgItemText(IDC_APP_CUST_INFO);
    item.fields.push_back({ _T("CustName"), val });

    // Müşteri Telefonu (UI'da yok, DB'den çekiyoruz)
    if (!m_custID.IsEmpty())
    {
        Customer_cstr cust = m_db.GetCustomerByCariKod(m_custID);
        item.fields.push_back({ _T("CustTel"), cust.Telefon });
    }
    else
    {
        item.fields.push_back({ _T("CustTel"), _T("-") });
    }

    // --- 2. ANA PENCEREYE GÖNDER ---

    // Erişim
    CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();

    // Diyalogu kapat (İsteğe bağlı, açık da kalabilir)
    // EndDialog(IDOK); 

    // Veriyi yükle
    mainFrame.GetPreviewDlg().SetPreviewData(item);

    // Görünümü "Baskı Önizleme" moduna geçir
    mainFrame.SetPreviewView();
}









// Mülk Seçme Butonu Tıklandığında
void CAppointmentDlg::OnSelectProperty()
{
    // Cari Kod boşsa uyarı ver
    if (m_custID.IsEmpty()) {
        MessageBox(_T("Müşteri bilgisi bulunamadı!"), _T("Hata"), MB_ICONERROR);
        return;
    }

    CPropertySelectDlg dlg(m_custID);
    if (dlg.DoModal() == IDOK)
    {
        // Seçilenleri al
        m_propID = dlg.GetSelectedPropertyID();
        CString type = dlg.GetSelectedPropertyType();
        m_propInfo = type + _T(" - ") + dlg.GetSelectedPropertyInfo();

        // UI Güncelle
        SetDlgItemText(IDC_APP_PROP_ID, m_propID);
        SetDlgItemText(IDC_APP_PROP_INFO, m_propInfo);

        // Veritabanına kaydetmek için Type bilgisini de saklamalıyız
        // (Bunun için Appointment_cstr içine PropertyType eklemiştik, oraya gidecek)
        // Burada gizli bir static text veya member variable kullanabiliriz.
        m_propType = type; // Class member olarak tanımla: CString m_propType;
    }
}


