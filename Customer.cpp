#include "stdafx.h"
#include "RibbonApp.h"
#include "Files.h"
#include "Customer.h"
#include "GoogleServices.h" // Google entegrasyonu için
#include "Mainfrm.h"        // LoginManager'a erişmek için
#include <thread>           // Arka plan işlemi için


CCustomerDialog::CCustomerDialog(UINT nResID /*= IDD_CUSTOMER_DIALOG*/)
    : CDialog(nResID)
{
}


CCustomerDialog::CCustomerDialog(DatabaseManager& dbManagerRef, DialogMode mode, const CString& cariKodToEdit /*= _T("")*/)
// YapÄ±cÄ± Listesi (Member Initializer List) ile Ã¼ye deÄŸiÅŸkenlerini baÅŸlatÄ±rÄ±z:
    : CDialog(IDD_CUSTOMER_DIALOG),
    dbManager(dbManagerRef), // DatabaseManager referansÄ±nÄ± saklar
    m_dialogMode(mode),        // Modu (INEWUSER/IUPDATEUSER) saklar
    m_cariKodToEdit(cariKodToEdit) // Cari Kodu saklar
{
    // YapÄ±cÄ± gÃ¶vdesi boÅŸ kalabilir, tÃ¼m iÅŸi InitDialog yapacaktÄ±r.
}

// 1️⃣ BAŞLATMA (OnInitDialog)
BOOL CCustomerDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ComboBox içeriklerini doldur (Data binding'den önce yapılmalı)
    OnSetCtrl();

    if (m_dialogMode == IUPDATEUSER)
    {
        // --- GÜNCELLEME MODU ---
        SetWindowText(_T("Müşteri Kaydını Güncelle"));
        ::SetDlgItemText(*this, IDOK, _T("Kaydı Güncelle"));

        // Veriyi DB'den Map olarak çek
        // TABLE_NAME_CUSTOMER ("Customer") tablosundan, Cari_Kod anahtarı ile çek
        auto dataMap = dbManager.FetchRecordMap(TABLE_NAME_CUSTOMER, _T("Cari_Kod"), m_cariKodToEdit);

        if (!dataMap.empty()) {
            // Otomatik Eşleştirme ve Ekrana Basma
            dbManager.Bind_Data_To_UI(this, TABLE_NAME_CUSTOMER, dataMap);
        }

        // Cari Kod değiştirilemez (PK)
        ::SendMessage(GetDlgItem(IDC_EDIT_CARI_KOD), EM_SETREADONLY, TRUE, 0);
    }
    else
    {
        // --- YENİ KAYIT MODU ---
        SetWindowText(_T("Yeni Müşteri Kaydı"));
        ::SetDlgItemText(*this, IDOK, _T("Kayıt Ekle"));

        // Varsayılan verileri hazırla
        std::map<CString, CString> defaultData;
        defaultData[_T("Cari_Kod")] = dbManager.GenerateNextCariCode();

        // Tarih formatı (dd.mm.yyyy hh:mm)
        SYSTEMTIME st; GetLocalTime(&st);
        CString now; now.Format(_T("%02d.%02d.%04d %02d:%02d"), st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
        defaultData[_T("KayitTarihi")] = now;
        defaultData[_T("Durum")] = _T("Aktif"); // Varsayılan durum

        // Ekrana Bas
        dbManager.Bind_Data_To_UI(this, TABLE_NAME_CUSTOMER, defaultData);
    }

    return TRUE;
}


// 2️⃣ COMBOBOX DOLDURMA (OnSetCtrl)
void CCustomerDialog::OnSetCtrl()
{
    // Helper lambda: Combo ID'sine string listesi doldurur
    auto Fill = [&](int id, const std::vector<CString>& items) {
        HWND hCombo = GetDlgItem(id);
        if (hCombo) {
            ::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
            for (const auto& s : items)
                ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s);
        }
        };

    Fill(IDC_COMBO_MUSTERI_TIPI, { _T("Bireysel"), _T("Ticari") });
    Fill(IDC_COMBO_DURUM, { _T("Aktif"), _T("Pasif"), _T("Kara Liste") });
}



// Customer.cpp (CCustomerDialog sÄ±nÄ±fÄ± iÃ§inde)

// Customer_cstr yapÄ±sÄ±ndaki veriyi tÃ¼m Dialog kontrollerine yazar.
void CCustomerDialog::SetDataToControls(const Customer_cstr& data)
{
    // CEdit::SetWindowText() metodu kullanÄ±larak tÃ¼m EditBox'lar doldurulur.

    // --- EditBox Kontrolleri ---

    // Cari Kod: Sadece GÃ¼ncelleme modunda veri iÃ§erir ve read-only olmalÄ±dÄ±r.
    m_editCariKod.SetWindowText(data.Cari_Kod);

    m_editAdSoyad.SetWindowText(data.AdSoyad);
    m_editTelefon.SetWindowText(data.Telefon);
    m_editTelefon2.SetWindowText(data.Telefon2);
    m_editEmail.SetWindowText(data.Email);
    m_editAdres.SetWindowText(data.Adres);
    m_editSehir.SetWindowText(data.Sehir);
    m_editIlce.SetWindowText(data.Ilce);
    m_editTCKN.SetWindowText(data.TCKN);
    m_editVergiNo.SetWindowText(data.VergiNo);

    // KayÄ±t Tarihi: Genellikle sadece okunur bir alana yazÄ±lÄ±r.
    m_editKayitTarihi.SetWindowText(data.KayitTarihi);

    m_editNotlar.SetWindowText(data.Notlar);


    // --- ComboBox Kontrolleri ---

    // MÃ¼ÅŸteri Tipi: ComboBox'Ä±n mevcut seÃ§enekleri iÃ§inden eÅŸleÅŸeni seÃ§er.
    m_comboMusteriTipi.SelectString(-1, data.MusteriTipi);

    // Durum: ComboBox'Ä±n mevcut seÃ§enekleri iÃ§inden eÅŸleÅŸeni seÃ§er.
    m_comboDurum.SelectString(-1, data.Durum);
}












// 4️⃣ KOMUT YÖNETİMİ
BOOL CCustomerDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case IDOK:
        OnOK();
        return TRUE;
    case IDCANCEL:
        EndDialog(IDCANCEL);
        return TRUE;
    case IDC_LOAD_CSV:
        OnLoadCsvClicked();
        return TRUE;
    }
    return FALSE; // Win32++ default işleyişine bırak
}
void CCustomerDialog::LoadCustomerToControls()
{
    SetDlgItemText(IDC_EDIT_CARI_KOD,     m_customer.Cari_Kod);
    SetDlgItemText(IDC_EDIT_ADSOYAD,      m_customer.AdSoyad);
    SetDlgItemText(IDC_EDIT_TELEFON,      m_customer.Telefon);
    SetDlgItemText(IDC_EDIT_TELEFON2,     m_customer.Telefon2);
    SetDlgItemText(IDC_EDIT_EMAIL,        m_customer.Email);
    SetDlgItemText(IDC_EDIT_ADRES,        m_customer.Adres);
    SetDlgItemText(IDC_EDIT_SEHIR,        m_customer.Sehir);
    SetDlgItemText(IDC_EDIT_ILCE,         m_customer.Ilce);
    SetDlgItemText(IDC_EDIT_TCKN,         m_customer.TCKN);
    SetDlgItemText(IDC_EDIT_VERGINO,      m_customer.VergiNo);
    SetDlgItemText(IDC_EDIT_KAYITTARIHI,  m_customer.KayitTarihi);
    SetDlgItemText(IDC_EDIT_NOTLAR,       m_customer.Notlar);


}

void CCustomerDialog::SaveControlsToCustomer()
{
    
   m_customer.Cari_Kod       = GetDlgItemText(IDC_EDIT_CARI_KOD     );
   m_customer.AdSoyad        = GetDlgItemText(IDC_EDIT_ADSOYAD      );
   m_customer.Telefon        = GetDlgItemText(IDC_EDIT_TELEFON      );
   m_customer.Telefon2       = GetDlgItemText(IDC_EDIT_TELEFON2     );
   m_customer.Email          = GetDlgItemText(IDC_EDIT_EMAIL        );
   m_customer.Adres          = GetDlgItemText(IDC_EDIT_ADRES        );
   m_customer.Sehir          = GetDlgItemText(IDC_EDIT_SEHIR        );
   m_customer.Ilce           = GetDlgItemText(IDC_EDIT_ILCE         );
   m_customer.TCKN           = GetDlgItemText(IDC_EDIT_TCKN         );
   m_customer.VergiNo        = GetDlgItemText(IDC_EDIT_VERGINO      );
   m_customer.KayitTarihi    = GetDlgItemText(IDC_EDIT_KAYITTARIHI  );
   m_customer.Notlar         = GetDlgItemText(IDC_EDIT_NOTLAR       );

    // ComboBox seÃ§imleri
    //CComboBox cbType = GetDlgItem(IDC_COMBO_MUSTERI_TIPI);
   m_customer.MusteriTipi = cbType.GetWindowText();

    //CComboBox cbDurum = GetDlgItem(IDC_COMBO_DURUM);
   m_customer.Durum = cbDurum.GetWindowText();
}


void CCustomerDialog::GetDialogData(Customer_cstr& data)
{
    // Win32++ kuralÄ±na uygun olarak CEdit::GetWindowText() metodunun 
    // CString dÃ¶ndÃ¼ren aÅŸÄ±rÄ± yÃ¼klemesi kullanÄ±lÄ±yor.

    // --- EditBox Kontrollerinden Veri Alma ---

    data.Cari_Kod = dbManager.GenerateNextCariCode();
    data.AdSoyad = m_editAdSoyad.GetWindowText();
    data.Telefon = m_editTelefon.GetWindowText();
    data.Telefon2 = m_editTelefon2.GetWindowText();
    data.Email = m_editEmail.GetWindowText();
    data.Adres = m_editAdres.GetWindowText();
    data.Sehir = m_editSehir.GetWindowText();
    data.Ilce = m_editIlce.GetWindowText();
    data.TCKN = m_editTCKN.GetWindowText();
    data.VergiNo = m_editVergiNo.GetWindowText();
    data.Notlar = m_editNotlar.GetWindowText();

    // KayÄ±t Tarihi: VeritabanÄ±nda otomasyon iÃ§in gerekebilir, ancak kullanÄ±cÄ±dan da alÄ±nabilir.
    data.KayitTarihi = m_editKayitTarihi.GetWindowText();


    // --- ComboBox Kontrollerinden Veri Alma ---

    // MÃ¼ÅŸteri Tipi (ComboBox)
    int selMusteriTipi = m_comboMusteriTipi.GetCurSel();
    if (selMusteriTipi != CB_ERR)
    {



        int len = m_comboMusteriTipi.GetLBTextLen(selMusteriTipi);
        if (len != CB_ERR)
        {
            // Gerekli bellek alanÄ±nÄ± ayÄ±r ve doldurulabilir iÅŸaretÃ§iyi al
            m_comboMusteriTipi.GetLBText(selMusteriTipi, data.MusteriTipi.GetBuffer(len + 1));
            data.MusteriTipi.ReleaseBuffer(); // BelleÄŸi serbest bÄ±rak ve uzunluÄŸu ayarla
        }
    }
    else
    {
        // EÄŸer seÃ§ili bir Ã¶ÄŸe yoksa (veya manuel giriÅŸ varsa) metni al
        data.MusteriTipi = m_comboMusteriTipi.GetWindowText();
    }

    // Durum (ComboBox)
    int selDurum = m_comboDurum.GetCurSel();
    if (selDurum != CB_ERR)
    {
        //m_comboDurum.GetLBText(selDurum, data.Durum.AllocSysString());

        int len = m_comboDurum.GetLBTextLen(selDurum);
        if (len != CB_ERR)
        {
            // Gerekli bellek alanÄ±nÄ± ayÄ±r ve doldurulabilir iÅŸaretÃ§iyi al
            m_comboDurum.GetLBText(selDurum, data.Durum.GetBuffer(len + 1));
            data.Durum.ReleaseBuffer(); // BelleÄŸi serbest bÄ±rak ve uzunluÄŸu ayarla
        }




    }
    else
    {
        // EÄŸer seÃ§ili bir Ã¶ÄŸe yoksa metni al
        data.Durum = m_comboDurum.GetWindowText();
    }
}



// 3️⃣ KAYDETME (OnOK)
void CCustomerDialog::OnOK()
{
    // A) UI'dan verileri Map olarak oku (Otomatik)
    std::map<CString, CString> dataMap;
    dbManager.Bind_UI_To_Data(this, TABLE_NAME_CUSTOMER, dataMap);

    // B) Zorunlu Alan Kontrolü
    if (dataMap[_T("AdSoyad")].IsEmpty()) {
        MessageBox(_T("Ad Soyad alanı boş bırakılamaz."), _T("Hata"), MB_ICONWARNING);
        return;
    }

    // C) Eksik/Sistem Alanlarını Tamamla
    if (dataMap[_T("Cari_Kod")].IsEmpty()) {
        dataMap[_T("Cari_Kod")] = dbManager.GenerateNextCariCode();
    }

    // Çalışma Durumu varsayılanı
    if (dataMap[_T("Calisma_Durumu")].IsEmpty())
        dataMap[_T("Calisma_Durumu")] = _T("0");

    // Sync için zaman damgası ve ID
    dataMap[_T("Updated_At")] = dbManager.GetCurrentIsoUtc();

    // Eğer yeni kayıtsa sync_id verelim (Update ise zaten vardır)
    if (m_dialogMode == INEWUSER && dataMap[_T("sync_id")].IsEmpty()) {
        dataMap[_T("sync_id")] = dbManager.GenerateGuidString();
        dataMap[_T("Deleted")] = _T("0");
    }

    // D) Map -> Struct Dönüşümü
    Customer_cstr c;
    for (const auto& [key, val] : dataMap) {
        DatabaseManager::SetFieldByStringName(c, key, val);
    }

    // E) Veritabanı İşlemi
    bool success = false;
    dbManager.EnsureConnection(); // IsConnected yerine EnsureConnection daha güvenli

    if (m_dialogMode == INEWUSER) {
        success = dbManager.InsertGlobal(c);
    }
    else {
        success = dbManager.UpdateGlobal(c);
    }

    // F) Sonuç ve Google Entegrasyonu
    if (success) {
        // 1. Listeyi Yenile
        GetContainerApp()->GetMainFrame().GetMainListView().InsertItems();

        // =========================================================
        // 2. GOOGLE CONTACTS ENTEGRASYONU (Sadece Yeni Kayıtta)
        // =========================================================
        if (m_dialogMode == INEWUSER)
        {
            // MainFrame üzerinden LoginManager'a ulaşalım
            CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
            // Not: MainFrame içinde GetLoginManager() fonksiyonu olmalı veya m_login public olmalı.
            // Eğer GetLoginManager yoksa, m_login shared_ptr'sine erişim sağlayın.
            auto loginMgr = mainFrame.m_login; // Erişim yetkisine göre düzenleyin

            if (loginMgr && loginMgr->IsLoggedIn())
            {
                std::string token = loginMgr->GetAccessToken();

                // Verileri hazırla (CString -> std::string dönüşümü)
                CString rawName = dataMap[_T("AdSoyad")];
                CString rawTel = dataMap[_T("Telefon")];
                CString rawEmail = dataMap[_T("Email")];

                // Ad ve Soyadı ayır (Son boşluğa göre)
                CString firstName = rawName;
                CString lastName = _T("");
                int spacePos = rawName.ReverseFind(' ');
                if (spacePos > 0) {
                    firstName = rawName.Left(spacePos);
                    lastName = rawName.Mid(spacePos + 1);
                }

                // UI donmasın diye thread başlatıyoruz
                std::thread googleThread([token, firstName, lastName, rawTel, rawEmail]() {
                    try {
                        GoogleServices google(token);

                        // Dönüşümleri Thread içinde yapıyoruz (DatabaseManager Singleton güvenli varsayılır)
                        std::string sAd = DatabaseManager::GetInstance().CStringToAnsi(firstName);
                        std::string sSoyad = DatabaseManager::GetInstance().CStringToAnsi(lastName);
                        std::string sTel = DatabaseManager::GetInstance().CStringToAnsi(rawTel);
                        std::string sEmail = DatabaseManager::GetInstance().CStringToAnsi(rawEmail);

                        google.CreateContact(sAd, sSoyad, sTel, sEmail);
                    }
                    catch (...) {
                        // Thread içi hata ana programı çökertmesin
                        OutputDebugString(_T("[GoogleContacts] Thread hatası.\n"));
                    }
                    });

                // Thread'i serbest bırak (iş bitince kendi kapanır)
                googleThread.detach();
            }
        }
        // =========================================================

        MessageBox(_T("İşlem başarıyla tamamlandı."), _T("Bilgi"), MB_ICONINFORMATION);
        EndDialog(IDOK);
    }
    else {
        MessageBox(_T("Kayıt sırasında hata oluştu."), _T("Hata"), MB_ICONERROR);
    }
}


// 5️⃣ CSV / TXT YÜKLEME
void CCustomerDialog::OnLoadCsvClicked()
{
    CFileDialog fileDlg(TRUE, _T("txt"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Metin/CSV Dosyaları (*.txt;*.csv)\0*.txt;*.csv\0Tüm Dosyalar (*.*)\0*.*\0\0"));

    if (fileDlg.DoModal(*this) == IDOK)
    {
        if (LoadDataFromCsv(fileDlg.GetPathName())) {
            MessageBox(_T("Veriler forma yüklendi."), _T("Başarılı"), MB_ICONINFORMATION);
        }
        else {
            MessageBox(_T("Dosya okunamadı veya format hatası."), _T("Hata"), MB_ICONERROR);
        }
    }
}

bool CCustomerDialog::LoadDataFromCsv(const CString& filePath)
{
    std::wifstream wFile(filePath.GetString());
    if (!wFile.is_open()) return false;

    std::wstringstream buffer;
    buffer << wFile.rdbuf();
    std::wstring content = buffer.str();
    wFile.close();

    if (content.empty()) return false;

    // --- Verileri Haritaya Çıkar ---
    std::map<CString, CString> extractedData;
    std::wistringstream ss(content);
    std::wstring line;

    // UI'daki kolon adlarıyla eşleşen geçici map
    std::map<CString, CString> uiData;

    while (std::getline(ss, line))
    {
        CString currentLine(line.c_str());
        currentLine.Trim();
        if (currentLine.IsEmpty()) continue;

        // a) ETİKET:DEĞER
        int colonPos = currentLine.Find(L':');
        if (colonPos != -1)
        {
            CString key = currentLine.Left(colonPos);
            CString value = currentLine.Mid(colonPos + 1);
            key.Trim(); key.MakeUpper(); value.Trim();

            // Eğer dosyadaki anahtar bir DB alanı ise eşle
            // Örnek: Notlar alanına eklemek için biriktirme yapılabilir
            // Basit eşleştirme örnekleri:
            if (key == _T("AD SOYAD")) uiData[_T("AdSoyad")] = value;
            else if (key == _T("TELEFON")) uiData[_T("Telefon")] = value;
        }
        // b) ADRES PARSE
        else if (currentLine.Find(L'/') != -1)
        {
            // Basit tokenize
            int start = 0;
            CString token = currentLine.Tokenize(_T("/"), start);
            token.Trim();
            if (!token.IsEmpty()) { uiData[_T("Sehir")] = token; }

            token = currentLine.Tokenize(_T("/"), start);
            token.Trim();
            if (!token.IsEmpty()) { uiData[_T("Ilce")] = token; }

            // Kalanı adres yap
            CString kalan = currentLine.Mid(start);
            kalan.Trim();
            if (!kalan.IsEmpty()) uiData[_T("Adres")] = kalan;
        }
    }

    // Eğer veri bulunduysa UI'ya bas
    if (!uiData.empty()) {
        dbManager.Bind_Data_To_UI(this, TABLE_NAME_CUSTOMER, uiData);
        return true;
    }

    return false; // Veri bulunamadı
}