


#include "stdafx.h"
#include "vFieldDlg.h"
#include <fstream>
#include <shellapi.h>

//=========================================================
//  OnInitDialog
//  Amaç: Form açıldığında kontrolleri hazırlar ve veriyi yükler.
//=========================================================
BOOL CFieldDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ComboBox ve kontrolleri hazırla
    OnSetCtrl();

    // Kod alanlarını salt okunur yap (Otomatik üretim veya PK güvenliği için)
    ::SendMessage(GetDlgItem(IDC_FLD_CODE), EM_SETREADONLY, TRUE, 0);
    ::SendMessage(GetDlgItem(IDC_FLD_CARI_KOD), EM_SETREADONLY, TRUE, 0);

    if (m_mode == INEWUSER)
    {
        SetWindowText(_T("Yeni Tarla Kaydı Ekle"));

        // Varsayılan değerler ve yeni kod üretimi
        SetDlgItemText(IDC_FLD_CODE, m_db.GenerateNextFieldCode());
        if (!m_cariKod.IsEmpty()) SetDlgItemText(IDC_FLD_CARI_KOD, m_cariKod);

        // Tarih ve Durum varsayılanları
        ::SendMessage(GetDlgItem(IDC_FLD_DURUM), CB_SELECTSTRING, -1, (LPARAM)_T("Aktif"));
    }
    else // IUPDATEUSER
    {
        SetWindowText(_T("Tarla Kaydını Düzenle"));

        if (!m_fieldCodeToEdit.IsEmpty())
        {
            // MERKEZİ SİSTEM: Kaydı Map olarak çek
            auto dataMap = m_db.FetchRecordMap(TABLE_NAME_FIELD, _T("Field_Code"), m_fieldCodeToEdit);

            if (!dataMap.empty())
            {
                // OTOMASYON: Map verisini UI kontrollerine bas (SchemaManager kullanır)
                m_db.Bind_Data_To_UI(this, TABLE_NAME_FIELD, dataMap);

                if (dataMap.count(_T("Cari_Kod"))) m_cariKod = dataMap[_T("Cari_Kod")];
            }
            else
            {
                MessageBox(_T("Kayıt veritabanında bulunamadı!"), _T("Hata"), MB_ICONERROR);
                EndDialog(IDCANCEL);
            }
        }
    }

    return TRUE;
}
//=========================================================
//  OnOK (Kaydet)
//=========================================================
void CFieldDialog::OnOK()
{
    std::map<CString, CString> uiData;

    // OTOMASYON: Ekrandaki tüm verileri Map'e topla
    m_db.Bind_UI_To_Data(this, TABLE_NAME_FIELD, uiData);

    // Veri güvenliği ve sayısal alan kontrolü
    SanitizeDataMap(uiData);

    // Zorunlu alanların korunması
    if (m_mode == IUPDATEUSER) uiData[_T("Field_Code")] = m_fieldCodeToEdit;
    if (uiData[_T("Cari_Kod")].IsEmpty()) uiData[_T("Cari_Kod")] = m_cariKod;

    uiData[_T("Updated_At")] = m_db.GetCurrentIsoUtc();

    // Struct Dönüşümü: Map'ten Field_cstr oluştur
    Field_cstr fieldData;
    for (const auto& [key, val] : uiData) {
        DatabaseManager::SetFieldByStringName(fieldData, key, val);
    }

    // Veritabanı İşlemi
    bool success = (m_mode == INEWUSER) ? m_db.InsertGlobal(fieldData) : m_db.UpdateGlobal(fieldData);

    if (success)
    {
        MessageBox(_T("Tarla kaydı başarıyla kaydedildi."), _T("Bilgi"), MB_ICONINFORMATION);
        CDialog::OnOK();
    }
}

void CFieldDialog::SanitizeDataMap(std::map<CString, CString>& dataMap)
{
    // Sayısal alanların boş kalmamasını sağla (Veritabanı tipiyle uyum için)
    auto CleanNum = [&](const CString& key) {
        if (dataMap.find(key) == dataMap.end()) return;
        CString v = dataMap[key];
        CString n;
        for (int i = 0; i < v.GetLength(); i++) if (_istdigit(v[i])) n += v[i];
        dataMap[key] = n.IsEmpty() ? CString(_T("0")) : n;        };

    CleanNum(_T("Fiyat"));
    CleanNum(_T("Metrekare"));
    CleanNum(_T("PricePerM2"));
}
void CFieldDialog::OnSetCtrl()
{
    // dataIsMe.cpp içindeki GetProperties<Field_cstr> listesiyle uyumlu listeler
    std::vector<CString> varyok = { _T("Var"), _T("Yok") };
    std::vector<CString> types = { _T("Tarla"), _T("Zeytinlik"), _T("Meyve Bahçesi"), _T("Bağ") };
    std::vector<CString> tapu = { _T("Müstakil Parsel"), _T("Hisseli"), _T("Zilliyet") };
    std::vector<CString> status = { _T("Aktif"), _T("Satıldı"), _T("Pasif") };

    FillCombo(IDC_FLD_SULAMA, varyok);
    FillCombo(IDC_FLD_ACCESS, varyok);
    FillCombo(IDC_FLD_DEED_STATUS, tapu);
    FillCombo(IDC_FLD_DURUM, status);
}
void CFieldDialog::FillCombo(int id, const std::vector<CString>& items)
{
    HWND hCtrl = GetDlgItem(id);
    if (!hCtrl) return;
    ::SendMessage(hCtrl, CB_RESETCONTENT, 0, 0);
    for (const auto& s : items) ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s);
}
//=========================================================
//  UpdateUIFromMap (SchemaManager Entegrasyonu)
//  Amaç: Veri haritasını Schema'ya bakarak ID'lere dağıtır.
//=========================================================
void CFieldDialog::UpdateUIFromMap(const std::map<CString, CString>& dataMap)
{
    // SchemaManager'dan bu tabloya ait alan tanımlarını al
    // (Varsayım: Tablo adı "Fields" veya benzeri bir key ile kayıtlı)
    // Eğer SchemaManager tablo bazlı değilse manuel ID map'i kullanabiliriz ama
    // SchemaManager::GetSchema mantığını kullanmaya çalışalım.

    // Şimdilik Resource ID ve Key eşleşmesini SchemaManager'dan çekiyoruz
    // (Eğer SchemaManager tam hazır değilse burası manuel kalabilir ama mantığı şöyledir)
    const auto& schema = SchemaManager::Get().GetSchema(_T("Fields"));

    for (const auto& field : schema)
    {
        // UI bileşeni olmayan (resID=0) alanları geç
        if (field.resID == 0) continue;

        // Map'te bu alanın karşılığı var mı? (DB Name ile eşleşir)
        auto it = dataMap.find(field.dbName);
        if (it != dataMap.end())
        {
            if (field.type == FT_TEXT || field.type == FT_INT || field.type == FT_DOUBLE || field.type == FT_MEMO)
            {
                // Combo mu Edit mi kontrolü
                // (Basitçe GetDlgItem ile tip kontrolü yapılabilir veya hepsi SetControlText ile yönetilir)

                // ComboBox ise seç, Edit ise yaz
                CWnd pWnd = GetDlgItem(field.resID);
                if (pWnd) {
                    // CComboBox olup olmadığını anlamanın basit yolu (Win32++'da class name vs)
                    // Şimdilik generic SetControlText/SetComboSelection ayrımını manuel yapıyoruz
                    // veya helper kullanıyoruz.

                    // Basit bir trick: Eğer değer Combobox listesinde varsa Combo'dur.
                    // Ama en güvenlisi Schema'da FieldType belirtmek.
                    // SchemaManager.h'de FieldType enum var.

                    // ComboBox olan alanlar genellikle FT_TEXT ama belirli bir listesi vardır.
                    // Burada basitleştirilmiş bir yaklaşım kullanacağız:
                    SetControlText(field.resID, it->second); // Generic set
                }
            }
        }
    }

    // NOT: SchemaManager tam kurulu değilse veya ID eşleşmesi eksikse
    // manuel override yapabiliriz, ancak hedefimiz bunu otomatize etmek.

    // Manuel Özel Durumlar (Schema'da olmayanlar veya özel formatlar)
    // Örn: Fiyat formatlama
    CString fiyat = dataMap.at(_T("Fiyat"));
    if (!fiyat.IsEmpty()) SetControlText(IDC_FLD_PRICE, fiyat);
}

//=========================================================
//  GetMapFromUI
//=========================================================
std::map<CString, CString> CFieldDialog::GetMapFromUI()
{
    std::map<CString, CString> out;
    const auto& schema = SchemaManager::Get().GetSchema(_T("Fields"));

    for (const auto& field : schema)
    {
        if (field.resID == 0) continue;
        out[field.dbName] = GetControlText(field.resID);
    }
    return out;
}

//=========================================================
//  OnLoadFromClipboard (SahibindenParser Kullanımı)
//=========================================================
void CFieldDialog::OnLoadFromClipboard()
{
    // 1. Panodan metni al
    CString text;
    if (OpenClipboard(GetHwnd())) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            text = (LPCWSTR)GlobalLock(hData);
            GlobalUnlock(hData);
        }
        CloseClipboard();
    }

    if (text.IsEmpty()) {
        MessageBox(_T("Pano boş!"), _T("Uyarı"), MB_ICONWARNING);
        return;
    }

    // 2. YENİ PARSER'I ÇAĞIR
    auto parsedData = SahibindenParser::Parse(text);

    if (parsedData.empty()) {
        MessageBox(_T("Anlamlı veri bulunamadı."), _T("Hata"), MB_ICONERROR);
        return;
    }

    // 3. Hesaplamalar (Parser yapmıyorsa burada yap)
    // Fiyat/m2 hesapla
    if (parsedData.find(_T("PricePerM2")) == parsedData.end())
    {
        double f = _tstof(parsedData[_T("Fiyat")]);
        double m = _tstof(parsedData[_T("Metrekare")]);
        if (f > 0 && m > 0) {
            CString s; s.Format(_T("%.0f"), f / m);
            // Noktalama ekle (helper fonksiyonu eski dosyadan alabilirsin)
            // s = WithDots(s); 
            parsedData[_T("PricePerM2")] = s;
        }
    }

    // 4. Arayüzü güncelle
    UpdateUIFromMap(parsedData);

    MessageBox(_T("Veriler panodan başarıyla aktarıldı."), _T("Başarılı"), MB_ICONINFORMATION);
}

//=========================================================
//  OnMergeWithKml
//=========================================================
void CFieldDialog::OnMergeWithKml()
{
    if (m_rawKmlData.IsEmpty()) {
        MessageBox(_T("Önce KML dosyası yükleyin."), _T("Uyarı"), MB_ICONWARNING);
        return;
    }

    // 1. KML Ayrıştır (Internal Helper ile)
    auto kmlMap = ParseKmlInternal(m_rawKmlData);

    // 2. Arayüzdeki veriyi al
    auto uiMap = GetMapFromUI();

    // 3. Akıllı Birleştirme
    auto finalMap = SmartMerge(uiMap, kmlMap);

    // 4. Geri Yükle
    UpdateUIFromMap(finalMap);

    MessageBox(_T("KML verileri mevcut verilerle harmanlandı."), _T("Bilgi"), MB_ICONINFORMATION);
}

//=========================================================
//  OnLoadFromKml
//=========================================================
void CFieldDialog::OnLoadFromKml()
{
    CFileDialog dlg(TRUE, _T("kml"), NULL, OFN_FILEMUSTEXIST, _T("KML Files|*.kml||"));
    if (dlg.DoModal(*this) == IDOK)
    {
        m_rawKmlData = ReadFileContent(dlg.GetPathName());
        if (m_rawKmlData.IsEmpty()) return;

        // KML verisini parse et ve arayüze bas (Mevcut veriyi ezer)
        auto kmlMap = ParseKmlInternal(m_rawKmlData);
        UpdateUIFromMap(kmlMap);
    }
}

//=========================================================
//  Yardımcı: Struct <-> Map
//  (Eğer SchemaManager reflection desteklemiyorsa bu köprü şarttır)
//=========================================================
std::map<CString, CString> CFieldDialog::StructToMap(const Field_cstr& d)
{
    std::map<CString, CString> m;
    m[_T("Field_Code")] = d.Field_Code;
    m[_T("ListingNo")] = d.ListingNo;
    m[_T("Adres")] = d.Adres;
    m[_T("Lokasyon")] = d.Lokasyon;
    m[_T("Metrekare")] = d.Metrekare;
    m[_T("Ada")] = d.Ada;
    m[_T("Parsel")] = d.Parsel;
    m[_T("Pafta")] = d.Pafta;
    m[_T("Fiyat")] = d.Fiyat;
    m[_T("PricePerM2")] = d.PricePerM2;
    m[_T("ImarDurumu")] = d.ImarDurumu;
    m[_T("Kaks")] = d.Kaks;
    m[_T("Gabari")] = d.Gabari;
    m[_T("ToprakTuru")] = d.ToprakTuru;
    m[_T("Sulama")] = d.Sulama;
    // ... Diğer alanlar
    return m;
}

void CFieldDialog::MapToStruct(const std::map<CString, CString>& m, Field_cstr& d)
{
    // Güvenli okuma lambda'sı
    auto get = [&](const CString& k) {
        auto it = m.find(k);
        return (it != m.end()) ? it->second : CString();
        };

    d.Field_Code = get(_T("Field_Code"));
    d.ListingNo = get(_T("ListingNo"));
    d.Adres = get(_T("Adres"));
    d.Metrekare = get(_T("Metrekare"));
    d.Ada = get(_T("Ada"));
    d.Parsel = get(_T("Parsel"));
    d.Fiyat = get(_T("Fiyat"));
    d.ImarDurumu = get(_T("ImarDurumu"));
    d.Kaks = get(_T("Kaks"));
    d.Gabari = get(_T("Gabari"));
    // ... Diğer alanlar
}

//=========================================================
//  UI Helpers
//=========================================================
void CFieldDialog::SetControlText(UINT id, const CString& text)
{
    // 1. Doğrudan HWND alıyoruz (CWnd* değil)
    HWND hControl = GetDlgItem(id);
    if (!hControl) return;

    // 2. Kontrolün tipini anla
    TCHAR className[256];
    ::GetClassName(hControl, className, 255);
    CString sClass = className;

    // 3. Eğer ComboBox ise
    if (sClass.Find(_T("ComboBox")) != -1) // ComboBox sınıf ismini kontrol et
    {
        // Geçici bir nesne oluştur
        CComboBox combo;
        combo.Attach(hControl); // HWND'yi nesneye bağla

        // İşlemleri yap
        int idx = combo.FindStringExact(-1, text);
        if (idx != CB_ERR)
            combo.SetCurSel(idx);
        else
        {
            // Listede yoksa seçimi kaldır
            combo.SetCurSel(-1);
            // Eğer "DropList" değil de "DropDown" ise metni yazabilirsin:
            // combo.SetWindowText(text); 
        }

        combo.Detach(); // ÇOK ÖNEMLİ: İş bitince bağı kopar, yoksa nesne yok olurken pencereyi de kapatır!
    }
    else
    {
        // Edit veya diğerleri için standart Windows API yeterli
        ::SetWindowText(hControl, text);
    }
}
CString CFieldDialog::GetControlText(UINT id)
{
    HWND hControl = GetDlgItem(id);
    if (!hControl) return _T("");

    CString text;
    TCHAR className[256];
    ::GetClassName(hControl, className, 255);
    CString sClass = className;

    if (sClass.Find(_T("ComboBox")) != -1)
    {
        CComboBox combo;
        combo.Attach(hControl);

        int sel = combo.GetCurSel();
        if (sel != CB_ERR)
        {
            // Seçili öğenin metnini güvenli şekilde al (AllocSysString OUTPUT buffer değildir!).
            int len = combo.GetLBTextLen(sel);
            if (len != CB_ERR)
            {
                combo.GetLBText(sel, text.GetBuffer(len + 1));
                text.ReleaseBuffer();
            }
        }
        else
        {
            // Seçim yoksa edit kısmındaki metni al
            text = combo.GetWindowText();
        }

        combo.Detach(); // Unutma!
    }
    else
    {
        // Standart okuma
        int len = ::GetWindowTextLength(hControl);
        if (len > 0)
        {
            ::GetWindowText(hControl, text.GetBuffer(len + 1), len + 1);
            text.ReleaseBuffer();
        }
    }
    return text;
}
//=========================================================
//  SmartMerge
//=========================================================
std::map<CString, CString> CFieldDialog::SmartMerge(
    const std::map<CString, CString>& primary,
    const std::map<CString, CString>& secondary)
{
    auto result = primary;
    // Öncelikli KML alanları (Tapu bilgileri KML'den daha güvenilirdir)
    std::vector<CString> priorityKeys = { _T("Ada"), _T("Parsel"), _T("Metrekare"), _T("Pafta") };

    for (const auto& kv : secondary) {
        bool isPriority = false;
        for (const auto& k : priorityKeys) if (k == kv.first) isPriority = true;

        // Eğer birincil kaynakta boşsa VEYA bu alan öncelikliyse ikincilden al
        if (result[kv.first].IsEmpty() || isPriority) {
            result[kv.first] = kv.second;
        }
    }
    return result;
}

//=========================================================
//  ParseKmlInternal (Eski kodu modüler hale getirdik)
//=========================================================
std::map<CString, CString> CFieldDialog::ParseKmlInternal(const CString& kmlContent)
{
    // Buraya eski ParseKmlData mantığını taşıyabilirsiniz.
    // Ancak map dönüş tipi ile daha temiz olur.
    // Şimdilik CFieldDialog içindeki eski parse kodunu buraya copy-paste yapıp
    // dönüş tipini ayarlamanız yeterli.

    // Örn:
    std::map<CString, CString> out;
    // ... XML parsing logic ...
    // <SimpleData name="Ada">123</SimpleData> vb.
    return out;
}

//=========================================================
//  Message Map & DialogProc
//=========================================================
INT_PTR CFieldDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_COMMAND) {
        int id = LOWORD(wParam);
        switch (id) {
        case IDC_FLD_BTN_GETCLIPBOARD: OnLoadFromClipboard(); return TRUE;
        case IDC_FLD_LOAD_KML:         OnLoadFromKml();       return TRUE;
        case IDC_FLD_MERGE:            OnMergeWithKml();      return TRUE;
        case IDC_OPEN_IN_EARTH:        OnOpenInEarth();       return TRUE;
        }
    }
    return CDialog::DialogProc(uMsg, wParam, lParam);
}

void CFieldDialog::LoadCombos()
{
    // Yardımcı Lambda: Güvenli ComboBox doldurma (Hata düzeltildi)
    auto Fill = [&](UINT id, const std::vector<CString>& list) {
        // 1. Kontrolün Handle'ını al
        HWND hCtrl = GetDlgItem(id);
        if (!hCtrl) return;

        // 2. Geçici CComboBox nesnesi oluştur ve bağla
        CComboBox combo;
        combo.Attach(hCtrl);

        // 3. İçeriği temizle ve doldur
        combo.ResetContent();
        for (const auto& s : list) {
            combo.AddString(s);
        }

        // 4. Varsayılan olarak ilkini seç (Genelde "Seçiniz")
        combo.SetCurSel(0);

        // 5. BAĞLANTIYI KOPAR (Çok önemli, yoksa pencereyi yok eder)
        combo.Detach();
        };

    // --- LİSTELERİN DOLUMU (Eski verilerinizle tamamlandı) ---

    // 1. Toprak Türleri
    Fill(IDC_FLD_TOPRAK, {
        _T("Seçiniz"), _T("Tarla"), _T("Arazi"), _T("Bağ & Bahçe"), _T("Zeytinlik"),
        _T("Fındıklık"), _T("Sera"), _T("Mera"), _T("Orman"), _T("Sit Alanı"),
        _T("Sanayi"), _T("Depo & Antrepo"), _T("Kültürel Tesis"), _T("Eğitim"),
        _T("Enerji Depolama"), _T("Sağlık"), _T("Turizm"), _T("Villa"), _T("Ticari"), _T("Toplu Konut")
        });

    // 2. Sulama Durumu
    Fill(IDC_FLD_SULAMA, {
        _T("Seçiniz"), _T("Sulak"), _T("Kuru"), _T("Kısmen Sulak")
        });

    // 3. Tapu Durumu
    Fill(IDC_FLD_DEED_STATUS, {
        _T("Seçiniz"), _T("Hisseli Tapu"), _T("Müstakil Tapulu"), _T("Tahsis Tapu"),
        _T("Zilliyet Tapu"), _T("Kooperatif Hisseli Tapu"),
        _T("Yurt Dışı Tapulu"), _T("Tapu Kaydı Yok")
        });

    // 4. İmar Durumu
    Fill(IDC_FLD_IMAR, {
        _T("Seçiniz"), _T("Ada"), _T("A-Lejantlı"), _T("Arazi"), _T("Bağ & Bahçe"),
        _T("Depo & Antrepo"), _T("Eğitim"), _T("Enerji Depolama"), _T("Konut"),
        _T("Kültürel Tesis"), _T("Muhtelif"), _T("Özel Kullanım"), _T("Sağlık"),
        _T("Sanayi"), _T("Sera"), _T("Sit Alanı"), _T("Spor Alanı"), _T("Tarla"),
        _T("Tarla + Bağ"), _T("Ticari"), _T("Ticari + Konut"), _T("Toplu Konut"),
        _T("Turizm"), _T("Turizm + Konut"), _T("Turizm + Ticari"), _T("Villa"), _T("Zeytinlik")
        });

    // 5. KAKS (Emsal)
    Fill(IDC_FLD_KAKS, {
        _T("Seçiniz"),
        _T("0.05"), _T("0.10"), _T("0.15"), _T("0.17"), _T("0.20"), _T("0.24"),
        _T("0.25"), _T("0.30"), _T("0.35"), _T("0.40"), _T("0.45"), _T("0.50"),
        _T("0.60"), _T("0.70"), _T("0.75"), _T("0.80"), _T("0.90"), _T("0.95"),
        _T("1.0"),  _T("1.05"), _T("1.10"), _T("1.15"), _T("1.20"), _T("1.25"),
        _T("1.30"), _T("1.35"), _T("1.40"), _T("1.45"), _T("1.50"), _T("1.55"),
        _T("1.60"), _T("1.75"), _T("1.80"), _T("1.90"), _T("2.0"),  _T("2.07"),
        _T("2.10"), _T("2.15"), _T("2.20"), _T("2.30"), _T("2.40"), _T("2.50"),
        _T("2.80"), _T("3.0"),  _T("3.20"), _T("3.30"), _T("5.0"),  _T("10.20"),
        _T("15.30"), _T("Belirtilmemiş")
        });

    // 6. Gabari
    Fill(IDC_FLD_GABARI, {
        _T("Seçiniz"), _T("3.50"), _T("6.50"), _T("7.50"), _T("8.50"), _T("9.50"),
        _T("10.50"), _T("11.50"), _T("12.50"), _T("14.50"), _T("15.50"),
        _T("17.50"), _T("18.50"), _T("21.50"), _T("24.50"), _T("27.50"),
        _T("30.50"), _T("36.00"), _T("Serbest"), _T("Belirtilmemiş")
        });

    // 7. Kimden (Seller)
    Fill(IDC_FLD_SELLER, {
        _T("Seçiniz"), _T("Sahibinden"), _T("Emlak Ofisinden")
        });

    // 8. Takas (Swap)
    Fill(IDC_FLD_SWAP, {
        _T("Seçiniz"), _T("Evet"), _T("Hayır")
        });

    // 9. Krediye Uygunluk (Credit)
    Fill(IDC_FLD_CREDIT, {
        _T("Evet"), _T("Hayır"), _T("Bilinmiyor")
        });
}

void CFieldDialog::OnOpenInEarth()
{
    // Eski OnOpenInEarth kodu buraya aynen gelebilir,
    // sadece m_data.RawKML yerine m_rawKmlData kullanın.
    if (m_rawKmlData.IsEmpty()) return;
    // ... temp dosya oluştur ve ShellExecute yap ...
}

CString CFieldDialog::ReadFileContent(const CString& path)
{
    // UTF-8 okuma fonksiyonu (eski LoadTextFile ile aynı)
    // ...
    return _T(""); // Placeholder
}


















//std::map<CString, CString> CFieldDialog::ParseSahibindenText(const CString& rawText)
//{
//    std::map<CString, CString> out;
//    CString text = rawText;
//
//    // satır sonlarını normalize et
//    text.Replace(_T("\r\n"), _T("\n"));
//    text.Replace(_T("\r"), _T("\n"));
//    text.Trim();
//
//    std::wistringstream iss((LPCWSTR)text);
//    std::vector<CString> lines;
//    for (std::wstring line; std::getline(iss, line); )
//    {
//        CString s(line.c_str()); s.Trim();
//        if (!s.IsEmpty()) lines.push_back(s);
//    }
//
//    // alias (etiket varyasyonlarını tek anahtara indir)
//    static const std::map<CString, CString> alias = {
//        { _T("Emlak Tipi"),     _T("PropertyType") },
//        { _T("Tapu Durumu"),    _T("DeedStatus")   },
//        { _T("Kimden"),         _T("SellerType")   },
//        { _T("Krediye Uygun"),  _T("CreditEligible") },
//        { _T("Takas"),          _T("Swap")         },
//        { _T("Kaks (E)"),       _T("Kaks")         },
//        { _T("Gabari"),         _T("Gabari")       },
//        { _T("İmar Durumu"),    _T("ImarDurumu")   },
//        { _T("Toprak"),         _T("ToprakTuru")   },
//        { _T("Sulama"),         _T("Sulama")       },
//        { _T("Ulaşım"),         _T("Ulasim")       }
//    };
//
//    auto FormatWithDots = [](const CString& digits)->CString {
//        CString out, t = digits; int len = t.GetLength(), c = 0;
//        for (int i = len - 1; i >= 0; --i) {
//            out.Insert(0, t[i]); if (++c == 3 && i != 0) { out.Insert(0, _T('.')); c = 0; }
//        }
//        return out;
//        };
//
//    for (auto& cLine : lines)
//    {
//        // 1) Adres (şehir/ilçe/mahalle çizgisi gibi tek satır)
//        if (cLine.Find(_T('/')) != -1 && cLine.Find(_T("İlan No")) == -1 && cLine.Find(_T("TL")) == -1)
//        {
//            CString addr = cLine; addr.Replace(_T(" / "), _T("/")); addr.Replace(_T("  "), _T(" ")); addr.Trim();
//            out[_T("Adres")] = addr;
//            continue;
//        }
//
//        // 2) İlan No (kolonlu/kolonsuz her iki formatı yakala, sadece sayıları al)
//        if (cLine.Left(7).CompareNoCase(_T("İlan No")) == 0 || cLine.Left(8).CompareNoCase(_T("İlan No:")) == 0)
//        {
//            CString v = cLine.Mid((cLine[7] == _T(':')) ? 8 : 7); v.Trim();
//            CString digits; for (int i = 0; i < v.GetLength(); ++i) if (_istdigit(v[i])) digits += v[i];
//            if (!digits.IsEmpty()) out[_T("İlan No")] = digits;
//            continue;
//        }
//
//        // 3) İlan Tarihi (kolonlu/kolonsuz)
//        if (cLine.Left(11).CompareNoCase(_T("İlan Tarihi")) == 0)
//        {
//            CString v = cLine.Mid(11); v.Trim();
//            if (v.Left(1) == _T(":")) { v = v.Mid(1); v.Trim(); }
//            out[_T("İlan Tarihi")] = v;
//            continue;
//        }
//
//        // 4) TL / USD / EUR içeren fiyat satırları (m² içermeyen)
//        if ((cLine.Find(_T(" TL")) != -1 || cLine.Find(_T("USD")) != -1 || cLine.Find(_T("EUR")) != -1) &&
//            cLine.Find(_T("m²")) == -1 && cLine.Find(_T("m2")) == -1)
//        {
//            CString clean = cLine; clean.Replace(_T("."), _T("")); clean.Replace(_T(","), _T("")); clean.Trim();
//            CString digits; for (int i = 0; i < clean.GetLength(); ++i) if (_istdigit(clean[i])) digits += clean[i];
//            if (!digits.IsEmpty()) out[_T("Fiyat")] = digits; // sade rakam
//            continue;
//        }
//
//        // 5) m² Fiyatı (TL/m²)
//// 5) m² Fiyatı (TL/m²)
//// 5) m² Fiyatı (TL/m²)
//// 5) m² Fiyatı (TL/m²)
//        // 5) m² Fiyatı (TL/m²)
//        //if (cLine.Find(_T("m² fiyat")) != -1 || cLine.Find(_T("m2 fiyat")) != -1)
//        //{
//        //    CString val = cLine;
//        //    val.MakeLower();
//
//        //    // ":" varsa sonrasını al
//        //    int colon = val.Find(_T(":"));
//        //    if (colon != -1)
//        //        val = val.Mid(colon + 1);
//
//        //    // Gereksiz semboller ve boşluklar
//        //    val.Replace(_T("tl"), _T(""));
//        //    val.Replace(_T("₺"), _T(""));
//        //    val.Replace(_T("/m2"), _T(""));
//        //    val.Replace(_T("/m²"), _T(""));
//        //    val.Replace(_T("m2"), _T(""));
//        //    val.Replace(_T("m²"), _T(""));
//        //    val.Replace(_T("fiyat"), _T(""));
//        //    val.Replace(_T(":"), _T(""));
//        //    val.Replace(_T(" "), _T(""));
//        //    val.Replace(_T("\t"), _T(""));
//        //    val.Trim();
//
//        //    // 🔥 Bazı ilanlarda nokta öncesinde boşluk kalabiliyor (ör: 26. 585)
//        //    while (val.Find(_T(". ")) != -1)
//        //        val.Replace(_T(". "), _T("."));
//
//        //    // Sadece rakam ve tek nokta kalsın
//        //    CString digits; bool dotUsed = false;
//        //    for (int i = 0; i < val.GetLength(); ++i)
//        //    {
//        //        TCHAR ch = val[i];
//        //        if (_istdigit(ch)) digits += ch;
//        //        else if (ch == _T('.') && !dotUsed) { digits += _T('.'); dotUsed = true; }
//        //    }
//
//        //    // Noktayı kaldır (bizim sistem WithDots kullanıyor)
//        //    digits.Replace(_T("."), _T(""));
//
//        //    // 🔧 Güvenlik: 3 haneli kalmış veya 9'dan fazla basamak varsa kısalt
//        //    if (digits.GetLength() > 8)
//        //        digits = digits.Left(8);
//
//        //    if (!digits.IsEmpty())
//        //        out[_T("PricePerM2")] = digits;
//
//        //    continue;
//        //}
//
//
//
//        if (cLine.Find(_T("m² fiyat")) != -1 || cLine.Find(_T("m2 fiyat")) != -1)
//        {
//            CString val = cLine;
//            val.MakeLower();
//
//            // ":" varsa ayır
//            int colon = val.Find(_T(":"));
//            if (colon != -1)
//                val = val.Mid(colon + 1);
//
//            // gereksiz ifadeleri temizle
//            val.Replace(_T("tl"), _T(""));
//            val.Replace(_T("₺"), _T(""));
//            val.Replace(_T("tl/m2"), _T(""));
//            val.Replace(_T("tl/m²"), _T(""));
//            val.Replace(_T("/m2"), _T(""));
//            val.Replace(_T("/m²"), _T(""));
//            val.Replace(_T("m2"), _T(""));
//            val.Replace(_T("m²"), _T(""));
//            val.Replace(_T("."), _T(""));
//            val.Replace(_T(","), _T(""));
//            val.Replace(_T(" "), _T(""));
//            val.Replace(_T("\t"), _T(""));
//            val.Replace(_T("m2 fiyatı: "), _T(""));
//            val.Replace(_T(" tl/m2"), _T(""));
//            val.Replace(_T("m² "), _T(""));
//
//            val.Replace(_T("m² fiyatı "), _T(""));
//
//            // sadece rakamları topla
//            CString digits;
//            for (int i = 0; i < val.GetLength(); ++i)
//            {
//                TCHAR ch = val[i];
//                if (_istdigit(ch))
//                    digits += ch;
//            }
//
//            // 🔥 bazen başa veya sona kaçan "2" karakteri varsa onu kırp
//            while (!digits.IsEmpty() && digits[0] == _T('2') && digits.GetLength() > 5)
//                digits.Delete(0);
//            while (!digits.IsEmpty() && digits[digits.GetLength() - 1] == _T('2') && digits.GetLength() > 5)
//                digits.Delete(digits.GetLength() - 1);
//
//            if (!digits.IsEmpty())
//                out[_T("PricePerM2")] = digits;
//
//            continue;
//        }
//
//
//        // 6) Metrekare (m²/m2 geçen ve 'fiyat' geçmeyen)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//// 6️⃣ m² (alan)
//        if ((cLine.Find(_T("m²")) != -1 || cLine.Find(_T("m2")) != -1) && cLine.Find(_T("Fiyat")) == -1)
//        {
//            CString val = cLine;
//            val.Trim();
//
//            // 🔥 UTF-8 kirli karakterleri ve sembolleri tamamen temizle
//            CString clean;
//            for (int i = 0; i < val.GetLength(); ++i)
//            {
//                TCHAR ch = val[i];
//                // sadece rakam ve nokta karakterleri kalsın
//                if ((ch >= _T('0') && ch <= _T('9')) || ch == _T('.'))
//                    clean += ch;
//            }
//
//            clean.Trim();
//            CString m2val = clean;
//
//            // 🧯 bazı panolarda başta "²" ya da fazladan "2" bug'ı olabiliyor
//            if (m2val.GetLength() > 6)
//            {
//                TCHAR c0 = m2val[0];
//                TCHAR c1 = (m2val.GetLength() >= 2) ? m2val[1] : _T('\0');
//                if ((c0 == _T('2') || c0 == L'\u00B2') && !_istdigit((unsigned short)c1))
//                {
//                    m2val = m2val.Mid(1);
//                    m2val.Trim();
//                }
//            }
//
//            out[_T("Metrekare")] = m2val;
//            continue;
//        }
//
//        // 7) Ada / Parsel / Pafta (Win32++ güvenli kullanım — Trim dönüşünü atama ile)
//        if (cLine.Find(_T("Ada No")) != -1) { CString v = cLine.Mid(cLine.Find(_T("Ada No")) + 7);  v.Trim(); out[_T("Ada")] = v; continue; }
//        if (cLine.Find(_T("Parsel No")) != -1) { CString v = cLine.Mid(cLine.Find(_T("Parsel No")) + 10); v.Trim(); out[_T("Parsel")] = v; continue; }
//        if (cLine.Find(_T("Pafta No")) != -1) { CString v = cLine.Mid(cLine.Find(_T("Pafta No")) + 9);  v.Trim(); out[_T("Pafta")] = v; continue; }
//
//        // 8) Genel key:value (":" ile)
//// 8) Genel key:value (":" ile)
//// 8️⃣ Genel key:value (":" VARSA klasik eşleşme, YOKSA boşluk tabanlı ayrıştırma)
//        int pos = cLine.Find(_T(":"));
//        CString key, val;
//
//        if (pos > 0)
//        {
//            // Standart format: "İmar Durumu: Konut"
//            key = cLine.Left(pos); key.Trim();
//            val = cLine.Mid(pos + 1); val.Trim();
//        }
//        else
//        {
//            // Alternatif format: "İmar Durumu Konut" veya "Tapu Durumu Müstakil Tapulu"
//            int spacePos = cLine.Find(_T(" "));
//            if (spacePos > 0)
//            {
//                key = cLine.Left(spacePos);
//                val = cLine.Mid(spacePos + 1);
//                key.Trim(); val.Trim();
//
//                // Eğer anahtar çok kelimeliyse (örneğin "İmar Durumu")
//                // İlk iki kelimeyi kontrol et
//                if (val.Find(_T("Durumu")) != -1 || key.CompareNoCase(_T("İmar")) == 0)
//                {
//                    int space2 = cLine.Find(_T(" "), spacePos + 1);
//                    if (space2 != -1)
//                    {
//                        key = cLine.Left(space2);
//                        val = cLine.Mid(space2 + 1);
//                        key.Trim(); val.Trim();
//                    }
//                }
//            }
//        }
//
//        // alias eşleştirmesi (önce key normalize)
//        auto itAlias = alias.find(key);
//        if (itAlias != alias.end())
//            key = itAlias->second;
//
//        // özel durum: "Krediye Uygunluk" → "Krediye Uygun"
//        if (key.CompareNoCase(_T("Krediye Uygunluk")) == 0)
//            key = _T("Krediye Uygun");
//
//        val.Replace(_T("(TL)"), _T(""));
//        val.Trim();
//        if (!key.IsEmpty() && !val.IsEmpty())
//            out[key] = val;
//
//    }
//
//    // 9) Eksikse Fiyat/m² hesapla
//// 9) Eksikse Fiyat/m² hesapla
//    if (out.find(_T("PricePerM2")) == out.end() &&
//        out.find(_T("Fiyat")) != out.end() &&
//        out.find(_T("Metrekare")) != out.end())
//    {
//        // CString'den double'a çevirirken _tstof kullanımı doğru
//        double fiyat = _tstof(out[_T("Fiyat")]);
//        double m2 = _tstof(out[_T("Metrekare")]);
//
//        if (fiyat > 0 && m2 > 0) {
//            // 🔥 DÜZELTME: Toplam fiyat yerine birim fiyatı (Fiyat / m2) hesapla.
//            double pricePerM2 = fiyat / m2;
//            CString s;
//            s.Format(_T("%.0f"), pricePerM2); // Virgülden sonraki kısmın yuvarlanması isteğe bağlıdır.
//            out[_T("PricePerM2")] = s;
//        }
//    }
//
//    out[_T("Website")] = _T("sahibinden.com");
//
//    //──────────────────────────────────────────────
//    // 10️⃣ Normalize "Kimden" ve "Krediye Uygun" alanları
//    //──────────────────────────────────────────────
//    if (out.find(_T("Kimden")) != out.end())
//    {
//        CString v = out[_T("Kimden")];
//        v.Trim();
//        v.MakeLower();
//
//        if (v.Find(_T("emlak ofis")) != -1)
//            out[_T("SellerType")] = _T("Emlak Ofisi");
//        else if (v.Find(_T("sahib")) != -1)
//            out[_T("SellerType")] = _T("Sahibinden");
//        else
//            out[_T("SellerType")] = _T("Seçiniz");
//    }
//
//    // 🔧 Bazı ilanlarda "Krediye Uygunluk" olarak geçer
//    CString kredikey = _T("");
//    if (out.find(_T("Krediye Uygun")) != out.end()) kredikey = _T("Krediye Uygun");
//    else if (out.find(_T("Krediye Uygunluk")) != out.end()) kredikey = _T("Krediye Uygunluk");
//
//    if (!kredikey.IsEmpty())
//    {
//        CString v = out[kredikey];
//        v.Trim();
//        v.MakeLower();
//
//        if (v.Find(_T("evet")) != -1)
//            out[_T("CreditEligible")] = _T("Evet");
//        else if (v.Find(_T("hay")) != -1)
//            out[_T("CreditEligible")] = _T("Hayır");
//        else if (v.Find(_T("bilin")) != -1)
//            out[_T("CreditEligible")] = _T("Bilinmiyor");
//        else
//            out[_T("CreditEligible")] = _T("Seçiniz");
//    }
//
//    return out;
//}
