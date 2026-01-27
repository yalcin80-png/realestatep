#include "stdafx.h"
#include "RibbonApp.h"       // CRibbonFrameApp, CMainFrame, GetDBManager
#include "Mainfrm.h"
#include "CTreeListVDlg.h"
#include <algorithm>
#include <shlwapi.h>
#include "PreviewItem.h"
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Msimg32.lib") // GradientFill
#include "DocumentDataManager.h" 
#include "PreviewPanel.h"
#include "resource.h"

using namespace std;
//int CMyTreeListView::ShowContextMenu(const CPoint& ptScreen, HTREEITEM hItem)
//{
//    m_hLastContextItem = hItem;
//
//    // 1. Menü Kaynağını Yükle
//    CMenu TopMenu;
//    if (!TopMenu.LoadMenu(IDR_TREELIST_MENU)) return 0;
//
//    CMenu SubMenu = TopMenu.GetSubMenu(0);
//    if (!SubMenu.GetHandle()) return 0;
//
//    // 2. İKONLARI TANIMLA (Resource.rc içindeki ikon/bitmap ID'leri)
//    // Hangi menü maddesinde hangi ikon çıkacak?
//    m_menuDrawer.AddIcon(IDM_TREELIST_EDIT, IDC_CMD_OPEN_LargeImages_RESID);      // Örnek ikon ID
//    m_menuDrawer.AddIcon(IDM_TREELIST_DELETE, IDB_BITMAP25);    // Örnek bitmap ID
//    m_menuDrawer.AddIcon(IDM_TREELIST_SATILDI, IDB_BITMAP22);// Varsa ikon ID
//    m_menuDrawer.AddIcon(IDM_VIEW_OFFERS, IDB_FILEVIEW);   // Teklifler ikonu
//    // ... Diğerlerini buraya ekle ...
//
//    // 3. DURUMLARI AYARLA (Check / Enable / Disable)
//    // Örneğin: Seçili öğe "Satıldı" ise menüdeki tik işaretini koy
//    /*
//    TLV_Row* row = GetRow(hItem);
//    if (row && row->GetStatus() == _T("Satıldı")) {
//        SubMenu.CheckMenuItem(IDM_TREELIST_SATILDI, MF_BYCOMMAND | MF_CHECKED);
//    }
//    */
//
//    // 4. MENÜYÜ DÖNÜŞTÜR (Sihirli Dokunuş)
//    // Bu işlem standart Windows menüsünü, bizim çizdiğimiz renkli menüye çevirir.
//    m_menuDrawer.ConvertToOwnerDraw(SubMenu.GetHandle());
//
//    // 5. MENÜYÜ GÖSTER
//    // Not: Win32++ CMenu::TrackPopupMenu, TPM_RETURNCMD ile seçilen ID'yi int döner.
//    int cmd = SubMenu.TrackPopupMenu(
//        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
//        ptScreen.x, ptScreen.y, *this);
//
//    return cmd;
//}
// CTreeListVDlg.cpp
//int CMyTreeListView::ShowContextMenu(const CPoint& ptScreen, HTREEITEM hItem)
//{
//    m_hLastContextItem = hItem;
//    if (m_pMatrixMenu && m_pMatrixMenu->IsWindow()) m_pMatrixMenu->Destroy();
//
//    m_pMatrixMenu = std::make_unique<CMatrixMenu>();
//
//    // --- AYARLAR ---
//    m_pMatrixMenu->SetGrid(3, 110, 10) // 3 Sütun, 110px kareler, 10px boşluk
//        .SetColors(RGB(30, 30, 30), RGB(60, 60, 60), RGB(0, 122, 204), RGB(255, 255, 255))
//        .SetCornerRadius(12);
//
//    // --- İÇERİK ---
//    // 1. Temel İşlemler
//    m_pMatrixMenu->AddItem(IDM_TREELIST_EDIT, _T("Düzenle\n(Edit)"));
//    m_pMatrixMenu->AddItem(IDM_TREELIST_DELETE, _T("Sil\n(Trash)"));
//    m_pMatrixMenu->AddItem(IDM_TREELIST_PREVIEW, _T("Önizle\n(Eye)"));
//
//    // 2. Alt Menülü Öğe (Sözleşmeler)
//    // Önce ana başlık (IDM_SATILIK_YETKI, sadece klasör görevi görür)
//    m_pMatrixMenu->AddItem(IDM_SATILIK_YETKI, _T("Sözleşmeler\n(Docs)"));
//
//    // Alt öğeleri ekle (Parent ID aynı olmalı)
//    m_pMatrixMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_SATILIK_YETKI, _T("Satılık Yetki"));
//    m_pMatrixMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_KIRALIK_TEK_YETKI, _T("Kiralık Yetki"));
//    m_pMatrixMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_CAYMA_AKCESI, _T("Cayma Akçesi"));
//
//    // 3. Diğerleri
//    m_pMatrixMenu->AddItem(IDM_ONOFFER_MENU, _T("Teklif Ver\n(Offer)"));
//    m_pMatrixMenu->AddItem(IDM_VIEW_OFFERS, _T("Gelenler\n(History)"));
//
//    // Callback
//    m_pMatrixMenu->OnCommandSelected = [this](UINT cmdID) {
//        this->OnCommand(cmdID, 0);
//        };
//
//    m_pMatrixMenu->Show(ptScreen, GetHwnd());
//    return 0;
//}
// 
// 
// 

int CMyTreeListView::ShowContextMenu(const CPoint& ptScreen, HTREEITEM hItem)
{
    m_hLastContextItem = hItem; // 💾 son sağ tıklanan item
    CMenu TopMenu(IDR_TREELIST_MENU);
    CMenu pPopupMenu;
    pPopupMenu = TopMenu.GetSubMenu(0);

    return pPopupMenu.TrackPopupMenu(
        TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        ptScreen.x, ptScreen.y, *this);
}

std::vector<Win32xx::PropertyColumnInfo>
CMyTreeListView::GetTableDefinition(const Win32xx::CString& tableName)
{
    std::vector<Win32xx::PropertyColumnInfo> cols;

    // TODO: burada gerçek kolon mapping'in gelecek.
    // Deneme/stabil olması için en azından boş dönmek yerine
    // 1-2 temel kolon ekleyebilirsin (aşağıdaki örnek gibi).

    Win32xx::PropertyColumnInfo c1{};
    c1.key = _T("code");
    c1.title = _T("Kod");
    c1.width = 120;
    cols.push_back(c1);

    Win32xx::PropertyColumnInfo c2{};
    c2.key = _T("title");
    c2.title = _T("Başlık");
    c2.width = 260;
    cols.push_back(c2);

    return cols;
}

bool CMyTreeListView::ResolveTableAndCode(HTREEITEM hItem,
    CString& outTable,
    CString& outCodeField,
    CString& outCode) const
{
    // 1. Temel Kontroller
    if (!IsWindow() || !hItem) return false;

    // Const cast (Helper fonksiyonlar const olmayabilir)
    auto* pThis = const_cast<CMyTreeListView*>(this);

    // 2. Kategori ve Satır Verisini Modelden Al
    TLV_Category* cat = pThis->GetCategoryForRow(hItem);
    TLV_Row* row = pThis->GetRow(hItem);

    // Eğer kategori yoksa, bu bir Root Item (Müşteri) veya Header olabilir.
    if (!cat)
    {
        // Belki Müşteri Root satırıdır?
        const CString* pRootCat = pThis->FindHeaderCategory(hItem);
        if (pRootCat && pRootCat->CompareNoCase(TABLE_NAME_CUSTOMER) == 0)
        {
            // Müşteri satırı düzenlenmek isteniyor
            outTable = TABLE_NAME_CUSTOMER;
            outCodeField = _T("Cari_Kod");
            if (row) outCode = row->key; // InsertDataRow ile eklenmişse key doludur
            else outCode = pThis->GetSubItemText(hItem, 1); // Fallback
            return !outCode.IsEmpty();
        }
        return false;
    }

    // 3. Normal Mülk Satırı (Home, Land vb.)
    outTable = cat->name; // Tablo adı (HomeTbl vb.)

    if (row)
    {
        outCode = row->key; // InsertDataRow ile eklenen Key (ID)
    }
    else
    {
        // Çok düşük ihtimal ama fallback (UI'dan oku)
        outCode = pThis->GetSubItemText(hItem, 1);
    }

    // 4. Kod Alan Adını (PK) Eşleştir
    static const std::map<CString, CString> mapPK = {
        { TABLE_NAME_HOME,       _T("Home_Code") },
        { TABLE_NAME_LAND,       _T("Land_Code") },
        { TABLE_NAME_FIELD,      _T("Field_Code") },
        { TABLE_NAME_VINEYARD,   _T("Vineyard_Code") },
        { TABLE_NAME_VILLA,      _T("Villa_Code") },
        { TABLE_NAME_COMMERCIAL, _T("Commercial_Code") },
        { TABLE_NAME_CAR,        _T("Car_Code") }, // ✅ BU SATIRI EKLE
        { TABLE_NAME_CUSTOMER,   _T("Cari_Kod") }
    };

    bool found = false;
    for (const auto& pair : mapPK) {
        if (outTable.CompareNoCase(pair.first) == 0) {
            outCodeField = pair.second;
            found = true;
            break;
        }
    }

    if (!found) outCodeField = _T("ID"); // Varsayılan

    return !outCode.IsEmpty();
}



//int CMyTreeListView::ShowContextMenu(const CPoint& ptScreen, HTREEITEM hItem)
//{
//    m_hLastContextItem = hItem;
//
//    if (m_pRadialMenu && m_pRadialMenu->IsWindow()) {
//        m_pRadialMenu->Destroy();
//    }
//
//    m_pRadialMenu = std::make_unique<CRadialMenu>();
//
//    // --- ANA MENÜLER ---
//    m_pRadialMenu->AddItem(IDM_TREELIST_EDIT, _T("Düzenle"));
//    m_pRadialMenu->AddItem(IDM_TREELIST_DELETE, _T("Sil"));
//    m_pRadialMenu->AddItem(ID_CONTEXT_EDIT_COLUMNS, _T("Kolonlar"));
//
//
//    // --- "SÖZLEŞME" ALT MENÜSÜ OLUŞTURMA ---
//    // Önce Ana Başlık Eklenir (ID'si 0 olabilir çünkü kendi başına işlevi yok)
//    // Ama kolaylık olsun diye IDM_SATILIK_YETKI verelim, tıklanınca bir şey yapmasın.
//    m_pRadialMenu->AddItem(IDM_SATILIK_YETKI, _T("Sözleşmeler"));
//
//    // Sonra Alt Öğeler Eklenir (Parent ID = IDM_SATILIK_YETKI)
//    m_pRadialMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_SATILIK_YETKI, _T("Satılık Yetki"));
//    m_pRadialMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_KIRALIK_TEK_YETKI, _T("Kiralık Yetki"));
//    m_pRadialMenu->AddSubItem(IDM_SATILIK_YETKI, IDM_CAYMA_AKCESI, _T("Cayma Akçesi"));
//
//    // --- DİĞER ANA MENÜLER ---
//    m_pRadialMenu->AddItem(IDM_TREELIST_PREVIEW, _T("Önizle"));
//    m_pRadialMenu->AddItem(IDM_ONOFFER_MENU, _T("Teklif Ver"));
//
//    m_pRadialMenu->OnCommandSelected = [this](UINT cmdID) {
//        this->OnCommand(cmdID, 0);
//        };
//
//    m_pRadialMenu->Show(ptScreen, GetHwnd());
//
//    return 0;
//}// CMyTreeListView.cpp dosyasının en üstüne (include'ların altına) ekleyin

// Veritabanı kolon adlarını İnsanların anlayacağı Türkçeye çevirir
void TranslateFieldNames(std::vector<std::pair<CString, CString>>& fields)
{
    // Çeviri Sözlüğü (Sol: DB Adı, Sağ: Görünen Ad)
    // Buraya veritabanındaki tüm alanları ve karşılıklarını ekleyebilirsin.
    static std::map<CString, CString> dictionary;

    if (dictionary.empty()) {
        // --- TEMEL BİLGİLER ---
        dictionary[_T("Cari_Kod")] = _T("Müşteri Kodu");
        dictionary[_T("Home_Code")] = _T("Emlak Kodu");
        dictionary[_T("ListingNo")] = _T("İlan Numarası");
        dictionary[_T("ListingDate")] = _T("İlan Tarihi");
        dictionary[_T("PropertyType")] = _T("Emlak Tipi");
        dictionary[_T("Status")] = _T("Durum");

        // --- ADRES ---
        dictionary[_T("City")] = _T("İl");
        dictionary[_T("District")] = _T("İlçe");
        dictionary[_T("Neighborhood")] = _T("Mahalle");
        dictionary[_T("Address")] = _T("Açık Adres");
        dictionary[_T("Location")] = _T("Konum / Mevki");

        // --- FİZİKSEL ---
        dictionary[_T("RoomCount")] = _T("Oda Sayısı");
        dictionary[_T("NetArea")] = _T("Net Alan (m²)");
        dictionary[_T("GrossArea")] = _T("Brüt Alan (m²)");
        dictionary[_T("MapArea")] = _T("Tapu Alanı (m²)");
        dictionary[_T("Floor")] = _T("Bulunduğu Kat");
        dictionary[_T("TotalFloor")] = _T("Kat Sayısı");
        dictionary[_T("BuildingAge")] = _T("Bina Yaşı");
        dictionary[_T("HeatingType")] = _T("Isıtma Tipi");
        dictionary[_T("BathroomCount")] = _T("Banyo Sayısı");
        dictionary[_T("Balcony")] = _T("Balkon");
        dictionary[_T("Elevator")] = _T("Asansör");
        dictionary[_T("Parking")] = _T("Otopark");
        dictionary[_T("Furnished")] = _T("Eşyalı");

        // --- FİNANSAL ---
        dictionary[_T("Price")] = _T("Fiyat");
        dictionary[_T("Currency")] = _T("Para Birimi");
        dictionary[_T("Dues")] = _T("Aidat");
        dictionary[_T("PricePerM2")] = _T("m² Birim Fiyatı");
        dictionary[_T("CreditEligible")] = _T("Krediye Uygun");
        dictionary[_T("Swap")] = _T("Takas");

        // --- TAPU & DİĞER ---
        dictionary[_T("DeedStatus")] = _T("Tapu Durumu");
        dictionary[_T("Pafta")] = _T("Pafta No");
        dictionary[_T("Ada")] = _T("Ada No");
        dictionary[_T("Parsel")] = _T("Parsel No");
        dictionary[_T("InSite")] = _T("Site İçerisinde");
        dictionary[_T("SiteName")] = _T("Site Adı");
        dictionary[_T("UsageStatus")] = _T("Kullanım Durumu");

        // --- MÜŞTERİ (JOIN ile geliyorsa) ---
        dictionary[_T("AdSoyad")] = _T("Mülk Sahibi");
        dictionary[_T("TCKN")] = _T("TC Kimlik No");
        dictionary[_T("Telefon")] = _T("Telefon");
        dictionary[_T("Email")] = _T("E-Posta");
        // --- YENİ ALIAS İSİMLERİ (Veritabanından Gelenler) ---
        dictionary[_T("OwnerName")] = _T("Mülk Sahibi");
        dictionary[_T("OwnerTC")] = _T("TC Kimlik No");
        dictionary[_T("OwnerPhone")] = _T("Telefon");
        dictionary[_T("OwnerEmail")] = _T("E-Posta");
        dictionary[_T("OwnerAddress")] = _T("Müşteri Adresi");
        dictionary[_T("OwnerCompany")] = _T("Firma Adı");

        dictionary[_T("FullAddress")] = _T("Mülk Açık Adresi");

        // --- ESKİLER (Hala gelebilecekler) ---
        dictionary[_T("Cari_Kod")] = _T("Müşteri Kodu");
        dictionary[_T("ListingNo")] = _T("İlan No");
        dictionary[_T("Price")] = _T("Fiyat");
        dictionary[_T("Currency")] = _T("Para Birimi");
        dictionary[_T("City")] = _T("İl");
        dictionary[_T("District")] = _T("İlçe");
        dictionary[_T("Neighborhood")] = _T("Mahalle");
        dictionary[_T("RoomCount")] = _T("Oda Sayısı");
        dictionary[_T("NetArea")] = _T("Net Alan");
        dictionary[_T("GrossArea")] = _T("Brüt Alan");
        dictionary[_T("Floor")] = _T("Bulunduğu Kat");
        dictionary[_T("BuildingAge")] = _T("Bina Yaşı");
        dictionary[_T("HeatingType")] = _T("Isıtma");
    }

    // Listeyi dön ve eşleşen varsa değiştir
    for (auto& field : fields)
    {
        // Büyük/Küçük harf duyarsız arama yapmak için DB'den gelen anahtarı kontrol edelim
        // Ancak map string eşleşmesi yaptığı için tam adını yazmalısınız.
        // Veritabanından gelen isim (key) dictionary'de var mı?

        // Not: Eğer DB'den gelen kolon adları değişken ise (büyük/küçük harf), 
        // buraya bir normalizasyon eklenebilir. Şimdilik birebir eşleşme varsayıyoruz.

        auto it = dictionary.find(field.first);
        if (it != dictionary.end())
        {
            field.first = it->second; // Key'i Türkçe yap! (Örn: "Price" -> "Fiyat")
        }
    }
}



// TreeList sınıfı (CTreeListVDlg veya benzeri)
LRESULT CMyTreeListView::OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // MainFrame'e ben aktif oldum de
    // 0: Tree Modu
    GetContainerApp()->GetMainFrame().SetRibbonContext(0);
    return FinalWindowProc(msg, wparam, lparam);
}



BOOL CMyTreeListView::OnPreviewCommand()
{
    DatabaseManager& db = DatabaseManager::GetInstance();

    HTREEITEM hSel = GetSelectedItem();
    if (!hSel) {
        MessageBox(L"Lütfen bir öğe seçiniz.", L"Baskı Önizleme", MB_ICONINFORMATION);
        return TRUE;
    }

    CString table, codeField, code;
    if (!ResolveTableAndCode(hSel, table, codeField, code)) {
        MessageBox(L"Tablo veya kod çözümlenemedi.", L"Baskı Önizleme", MB_ICONERROR);
        return TRUE;
    }

    // Verileri Çek (Burası Ham Veritabanı İsimleriyle Gelir: Price, RoomCount vb.)
    std::vector<std::pair<CString, CString>> fields;
    if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields)) {
        MessageBox(L"Kayıt bulunamadı veya veritabanı hatası.", L"Baskı Önizleme", MB_ICONERROR);
        return TRUE;
    }

    // --- PREVIEW ITEM HAZIRLIĞI ---
    PreviewItem pv;
    pv.tableName = table;
    pv.recordCode = code;
    pv.title.Format(L"%s – %s", table, code);

    // --- BELGE TİPİNİ BELİRLE ---
    if (table.CompareNoCase(L"Sozlesmeler") == 0 || table.CompareNoCase(L"Contracts") == 0)
    {
        pv.docType = DOC_CONTRACT_SALES_AUTH;
    }
    else if (table.CompareNoCase(L"Kapora") == 0 || table.CompareNoCase(L"Deposits") == 0)
    {
        pv.docType = DOC_CONTRACT_DEPOSIT;
    }
    else
    {
        // Varsayılan: Standart İlan Listesi
        pv.docType = DOC_LISTING_DETAILS;
    }

    // 🔥 KRİTİK NOKTA: ÇEVİRİ İŞLEMİ 🔥
    // Sadece "Liste" görünümü için Türkçeleştirme yapıyoruz.
    // Sözleşmeler (Contract Layouts) kod içinde İngilizce anahtar (Price, OwnerName vb.) beklediği için
    // onlara dokunmuyoruz.
    if (pv.docType == DOC_LISTING_DETAILS)
    {
        TranslateFieldNames(fields); // Price -> Fiyat, RoomCount -> Oda Sayısı olur.
    }

    // İşlenmiş (veya ham) veriyi pakete koy
    pv.fields = std::move(fields);

    // --- VERİYİ GÖNDER VE GÖRÜNÜMÜ AÇ ---
    CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
    mainFrame.GetPreviewDlg().SetPreviewData(pv);
    mainFrame.SetPreviewView();

    return TRUE;
}
// ... Diğer include'lar ...
// ============================================================================
// YETKİ BELGESİ OLUŞTURMA (GÜVENLİ)
// ============================================================================
void CMyTreeListView::OnCreateAuthorizationContract()
{
    try
    {
        // 1. Seçim Kontrolü
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) {
            MessageBox(_T("Lütfen işlem yapılacak mülkü seçiniz."), _T("Uyarı"), MB_ICONWARNING);
            return;
        }

        // 2. Tablo ve Kod Çözümleme
        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) {
            MessageBox(_T("Mülk bilgisi çözümlenemedi."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 3. Veriyi Çek
        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;

        // Bu fonksiyon veritabanı hatası fırlatabilir, try-catch bunu yakalar
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields))
        {
            MessageBox(_T("Veritabanından kayıt okunamadı."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 4. PreviewItem Hazırla
        PreviewItem item;
        item.docType = DOC_CONTRACT_SALES_AUTH; // Sözleşme Tipi
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("YETKİ BELGESİ - %s"), code);
        item.fields = std::move(fields);

        // 5. Panele Gönder
        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();

        // Veriyi güvenli bir şekilde yükle
        mainFrame.GetPreviewDlg().SetPreviewData(item);

        // Görünümü değiştir
        mainFrame.SetPreviewView();
    }
    catch (const std::exception& e)
    {
        // Standart C++ Hataları (std::vector, string dönüşüm vb.)
        CString msg; msg.Format(L"İşlem sırasında bir hata oluştu:\n%S", e.what());
        MessageBox(msg, L"Hata", MB_ICONERROR);
    }
    catch (...)
    {
        // Bilinmeyen Hatalar (SEH, ADO vb.)
        MessageBox(L"Beklenmeyen bir veritabanı veya sistem hatası oluştu.", L"Kritik Hata", MB_ICONERROR);
    }
}

// ============================================================================
// KAPORA / ALIM SATIM SÖZLEŞMESİ OLUŞTURMA (GÜVENLİ)
// ============================================================================
void CMyTreeListView::OnCreateDepositContract()
{
    try
    {
        // 1. Seçim Kontrolü
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) {
            MessageBox(_T("Lütfen işlem yapılacak mülkü seçiniz."), _T("Uyarı"), MB_ICONWARNING);
            return;
        }

        // 2. Tablo ve Kod Çözümleme
        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) {
            MessageBox(_T("Mülk bilgisi çözümlenemedi."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 3. Veriyi Çek
        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;

        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields))
        {
            MessageBox(_T("Veritabanından kayıt okunamadı."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 4. PreviewItem Hazırla
        PreviewItem item;
        item.docType = DOC_CONTRACT_DEPOSIT; // Kapora Sözleşmesi Tipi
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("ALIM SATIM SÖZLEŞMESİ - %s"), code);
        item.fields = std::move(fields);

        // 5. Panele Gönder
        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (const std::exception& e)
    {
        CString msg; msg.Format(L"İşlem sırasında bir hata oluştu:\n%S", e.what());
        MessageBox(msg, L"Hata", MB_ICONERROR);
    }
    catch (...)
    {
        MessageBox(L"Beklenmeyen bir veritabanı veya sistem hatası oluştu.", L"Kritik Hata", MB_ICONERROR);
    }
}


void CMyTreeListView::OnCreateShowingAgreement()
{
    try
    {
        // 1. Seçim Kontrolü
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) {
            MessageBox(_T("Lütfen işlem yapılacak mülkü seçiniz."), _T("Uyarı"), MB_ICONWARNING);
            return;
        }

        // 2. Tablo ve Kod Çözümleme
        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) {
            MessageBox(_T("Mülk bilgisi çözümlenemedi."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 3. Veriyi Çek
        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;

        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields))
        {
            MessageBox(_T("Veritabanından kayıt okunamadı."), _T("Hata"), MB_ICONERROR);
            return;
        }

        // 4. PreviewItem Hazırla
        PreviewItem item;
        item.docType = DOC_SHOWING_AGREEMENT; // <--- TİPİ BURADA BELİRLİYORUZ
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("YER GÖSTERME BELGESİ - %s"), code);
        item.fields = std::move(fields);

        // 5. Panele Gönder
        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (const std::exception& e)
    {
        CString msg; msg.Format(L"Hata: %S", e.what());
        MessageBox(msg, L"Hata", MB_ICONERROR);
    }
}



// ============================================================================
// ÖNALIM FERAGATNAMESİ OLUŞTURMA
// ============================================================================
void CMyTreeListView::OnCreatePreemptionWaiver()
{
    try
    {
        // 1. Seçim ve Veri Çözümleme (Standart)
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) { MessageBox(_T("Mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING); return; }

        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) return;

        // 2. Veritabanından Çek
        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields)) return;

        // --- ÖNEMLİ EKLEME ---
        // Bu belge için bir "Alıcı Adayı" ismine ihtiyaç var.
        // Şimdilik veritabanında olmadığı için elle veya boş ekliyoruz.
        // İleride bunu bir diyalog kutusuyla kullanıcıya sordurabilirsin.
        fields.push_back({ _T("BuyerName"), _T(".......................") }); // Boşluk bırakıyoruz

        // 3. Önizlemeye Gönder
        PreviewItem item;
        item.docType = DOC_WAIVER_PREEMPTION; // <--- Yeni türü kullanıyoruz
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("ÖNALIM FERAGATNAMESİ - %s"), code);
        item.fields = std::move(fields);

        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (const std::exception& e) { /* Hata yönetimi */ }
}


void CMyTreeListView::OnCreateRentalContract()
{
    try
    {
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) { MessageBox(_T("Mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING); return; }

        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) return;

        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields)) return;

        // Ekstra hesaplamalar (Yıllık Kira vb.)
        // fields içinde "Price" var, bunu alıp 12 ile çarpıp "AnnualRent" ekleyebilirsin.
        // Şimdilik basit tutuyoruz.

        PreviewItem item;
        item.docType = DOC_RENTAL_CONTRACT; // <--- TİP
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("KİRA KONTRATOSU - %s"), code);
        item.fields = std::move(fields);

        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (...) {}
}

void CMyTreeListView::OnCreateOfferLetter()
{
    try
    {
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) { MessageBox(_T("Mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING); return; }

        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) return;

        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields)) return;

        // Alıcı Bilgileri İçin Boş Alanlar Ekleyelim (Elle doldurulacak)
        fields.push_back({ _T("BuyerName"), _T(".......................") });
        fields.push_back({ _T("BuyerTC"), _T(".......................") });
        fields.push_back({ _T("OfferPrice"), _T(".......................") });

        PreviewItem item;
        item.docType = DOC_OFFER_LETTER; // <--- TİP
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("TEKLİF FORMU - %s"), code);
        item.fields = std::move(fields);

        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (...) {}
}

void CMyTreeListView::OnCreateRentalAuthContract()
{
    try
    {
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) { MessageBox(_T("Mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING); return; }

        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) return;

        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> fields;
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, fields)) return;

        PreviewItem item;
        item.docType = DOC_CONTRACT_RENTAL_AUTH; // <--- TİP
        item.tableName = table;
        item.recordCode = code;
        item.title.Format(_T("KİRALIK YETKİ BELGESİ - %s"), code);
        item.fields = std::move(fields);

        CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
        mainFrame.GetPreviewDlg().SetPreviewData(item);
        mainFrame.SetPreviewView();
    }
    catch (...) {}
}


void CMyTreeListView::OnCreateOffer()
{
    try
    {
        // 1. Mülk Seçim Kontrolü
        HTREEITEM hItem = GetSelectedItem();
        if (!hItem) { MessageBox(_T("Lütfen bir mülk seçiniz."), _T("Uyarı"), MB_ICONWARNING); return; }

        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code)) return;

        // 2. Veritabanından Mülk Bilgilerini Çek
        auto& db = DatabaseManager::GetInstance();
        std::vector<std::pair<CString, CString>> propertyFields;
        if (!db.FetchRecordFieldsWithCustomer(table, codeField, code, propertyFields)) return;

        // Mülk Başlığını Hazırla (Diyalogda göstermek için)
        // propertyFields içinden adres veya tip bulalım
        CString propInfo = _T("Mülk Kodu: ") + code;

        // 3. TEKLİF DİYALOĞUNU AÇ
        COfferDialog dlg(db, propInfo);

        if (dlg.DoModal() == IDOK) // Kullanıcı "Kaydet" dedi
        {
            // 4. Diyalogdan Verileri Al
            Offer_cstr offer = dlg.GetOfferData();
            Customer_cstr cust = dlg.GetCustomerData();

            // 5. Veritabanına Teklifi Kaydet (Offers Tablosu)
            offer.PropertyID = code;
            // offer.CustomerID = cust.Cari_Kod; // Eğer müşteri sistemden seçildiyse ID'si vardır

            // db.AddOffer(offer); // (DatabaseManager'a AddOffer fonksiyonu eklendiğinde açarsın)

            // 6. Önizlemeye Gönder
            PreviewItem item;
            item.docType = DOC_OFFER_LETTER;
            item.tableName = table;
            item.recordCode = code;
            item.title.Format(_T("TEKLİF MEKTUBU - %s"), code);

            // Verileri Birleştir (Mülk + Müşteri + Teklif)
            item.fields = propertyFields; // Mülk bilgileri taban

            // Müşteri
            item.fields.push_back({ _T("BuyerName"), cust.AdSoyad });
            item.fields.push_back({ _T("BuyerTC"), cust.TCKN });
            item.fields.push_back({ _T("BuyerPhone"), cust.Telefon });

            // Teklif
            item.fields.push_back({ _T("OfferPrice"), offer.OfferPrice });
            item.fields.push_back({ _T("DepositAmount"), offer.DepositAmount });
            item.fields.push_back({ _T("PaymentMethod"), offer.PaymentMethod });
            item.fields.push_back({ _T("Validity"), offer.ValidityDate });

            CMainFrame& mainFrame = GetContainerApp()->GetMainFrame();
            mainFrame.GetPreviewDlg().SetPreviewData(item);
            mainFrame.SetPreviewView();
        }
    }
    catch (...) {}
}





COLORREF CMyTreeListView::GetColorForStatus(const CString& status)
{
    if (status.CompareNoCase(_T("Satıldı")) == 0)
        return RGB(200, 200, 200); // Gri (Pasif)

    if (status.CompareNoCase(_T("Fiyat Düştü")) == 0)
        return RGB(220, 255, 220); // Açık Yeşil (Fırsat)

    if (status.CompareNoCase(_T("Acil")) == 0)
        return RGB(255, 230, 230); // Açık Kırmızı (Dikkat)

    if (status.CompareNoCase(_T("Pasif")) == 0)
        return RGB(245, 245, 245); // Çok Soluk Gri

    if (status.CompareNoCase(_T("Opsiyonlu")) == 0)
        return RGB(255, 255, 200); // Açık Sarı

    return CLR_NONE; // Varsayılan (Beyaz)
}

// --------------------------------------------------------
// Modüler Yardımcı Fonksiyon: Tablo adına göre PK alan adını döndür
// --------------------------------------------------------
CString GetCodeFieldForTable(const CString& tableName)
{
    static const std::map<CString, CString, CStringLessNoCase> TABLE_CODE_FIELDS = {
        { TABLE_NAME_HOME,       _T("Home_Code") },
        { TABLE_NAME_LAND,       _T("Land_Code") },
        { TABLE_NAME_FIELD,      _T("Field_Code") },
        { TABLE_NAME_VINEYARD,   _T("Vineyard_Code") },
        { TABLE_NAME_VILLA,      _T("Villa_Code") },
        { TABLE_NAME_COMMERCIAL, _T("Commercial_Code") },
        { TABLE_NAME_CAR,        _T("Car_Code") },
        { TABLE_NAME_CUSTOMER,   _T("Cari_Kod") }
    };

    auto it = TABLE_CODE_FIELDS.find(tableName);
    if (it != TABLE_CODE_FIELDS.end()) {
        return it->second;
    }
    return _T("ID"); // Varsayılan
}


//void CMyTreeListView::ChangePropertyStatus(HTREEITEM hItem, UINT cmdId)
//{
//    // 1. Bilgileri Al
//    TLV_Row* row = GetRow(hItem);
//    TLV_Category* cat = GetCategoryForRow(hItem);
//    if (!row || !cat) return;
//
//    CString table = cat->name;
//    CString code = row->key;
//
//    // Tabloya göre durum kolonu adı (HomeTbl'de 'Status', diğerlerinde 'Durum')
//    //CString statusField = (table.CompareNoCase(TABLE_NAME_HOME) == 0) ? _T("Status") : _T("Durum");
//    CString statusField = _T("Status");
//    // 2. Yeni Durumu Belirle
//    CString newStatus;
//    switch (cmdId)
//    {
//    case IDM_STATUS_ACTIVE:     newStatus = _T("Aktif");       break;
//    case IDM_STATUS_SOLD:       newStatus = _T("Satıldı");     break;
//    case IDM_STATUS_PRICE_DOWN: newStatus = _T("Fiyat Düştü"); break;
//    case IDM_STATUS_URGENT:     newStatus = _T("Acil");        break;
//    case IDM_STATUS_PASSIVE:    newStatus = _T("Pasif");       break;
//    default: return;
//    }
//
//    // 3. Veritabanını Güncelle
//    // UpdateFieldGlobal fonksiyonu Updated_At'i de günceller.
//    DatabaseManager& db = DatabaseManager::GetInstance();
//    // Cari Kod'u satırdan veya root'tan bulmamız lazım (önceki düzeltmelerdeki gibi)
//    // Ama UpdateFieldGlobal sadece PK (code) ile çalışıyor, bu harika.
//
//    // Dikkat: UpdateFieldGlobal 5 parametre istiyor, codeField'ı bulmamız lazım.
//    CString codeField; // (Bunu ResolveTableAndCode içindeki haritadan veya basit if-else ile alabilirsin)
//    if (table.CompareNoCase(TABLE_NAME_HOME) == 0)
//        codeField = _T("Home_Code");
//    else if (table.CompareNoCase(TABLE_NAME_LAND) == 0)
//        codeField = _T("Land_Code");
//    else if (table.CompareNoCase(TABLE_NAME_FIELD) == 0)
//        codeField = _T("Field_Code");
//    else if (table.CompareNoCase(TABLE_NAME_VINEYARD) == 0)
//        codeField = _T("Vineyard_Code");
//    else if (table.CompareNoCase(TABLE_NAME_VILLA) == 0)
//        codeField = _T("Villa_Code");
//    else if (table.CompareNoCase(TABLE_NAME_COMMERCIAL) == 0)
//        codeField = _T("Commercial_Code");
//    else if (table.CompareNoCase(TABLE_NAME_CUSTOMER) == 0)
//        codeField = _T("Cari_Kod");
//    // ... diğerleri ...
//    else codeField = table + _T("_Code"); // Basit varsayım
//
//    if (db.UpdateFieldGlobal(table, codeField, code, statusField, newStatus))
//    {
//        // 4. Görünümü Anında Güncelle (Reload yapmadan!)
//        COLORREF newColor = GetColorForStatus(newStatus);
//        COLORREF newTextColor = (newStatus == _T("Satıldı")) ? RGB(100, 100, 100) : CLR_DEFAULT;
//
//        SetRowColor(hItem, newTextColor, newColor);
//
//        // Durum metnini de güncellemek istersen (Hangi kolonda olduğunu bilmek lazım)
//        // Örneğin Status kolonu 46. sıradaysa:
//        // SetSubItemText(hItem, 46, newStatus);
//
//        Invalidate(); // Boyamayı tetikle
//    }
//    else
//    {
//        MessageBox(_T("Durum güncellenemedi."), _T("Hata"), MB_ICONERROR);
//    }
//}





void CMyTreeListView::ChangePropertyStatus(HTREEITEM hItem, UINT cmdId)
{
    // 1. Bilgileri Al
    TLV_Row* row = GetRow(hItem);
    TLV_Category* cat = GetCategoryForRow(hItem);
    if (!row || !cat) return;

    CString table = cat->name;
    CString code = row->key;

    // ✅ DÜZELTME BURADA: Artık tüm tablolarda alan adı "Status"
    CString statusField = _T("Status");

    // 2. Yeni Durumu Belirle (Modüler Yaklaşım ile Refactor Edildi)
    CString newStatus;
    COLORREF rowColor = GetTheme().clrWnd;
    COLORREF txtColor = GetTheme().clrText;

    switch (cmdId)
    {
    case IDM_STATUS_ACTIVE:     newStatus = _T("Aktif");       break;
    case IDM_STATUS_SOLD:
        newStatus = _T("Satıldı");
        rowColor = RGB(220, 220, 220); // Gri
        txtColor = RGB(128, 128, 128);
        break;
    case IDM_STATUS_PRICE_DOWN:
        newStatus = _T("Fiyat Düştü");
        rowColor = RGB(220, 255, 220); // Yeşil
        break;
    case IDM_STATUS_URGENT:
        newStatus = _T("Acil");
        rowColor = RGB(255, 230, 230); // Kırmızı
        break;
    case IDM_STATUS_PASSIVE:
        newStatus = _T("Pasif");
        rowColor = RGB(240, 240, 240);
        txtColor = RGB(160, 160, 160);
        break;
    // Yeni Kurumsal Durum Seçenekleri - Modüler Renk Yönetimi ile
    case IDM_STATUS_SOLD_NEW:
    {
        StatusColorInfo info = GetStatusColorInfoByCode(1);
        newStatus = info.statusName;
        rowColor = info.backgroundColor;
        txtColor = info.textColor;
        break;
    }
    case IDM_STATUS_WAITING:
    {
        StatusColorInfo info = GetStatusColorInfoByCode(2);
        newStatus = info.statusName;
        rowColor = info.backgroundColor;
        txtColor = info.textColor;
        break;
    }
    case IDM_STATUS_PRICE_TRACKING:
    {
        StatusColorInfo info = GetStatusColorInfoByCode(3);
        newStatus = info.statusName;
        rowColor = info.backgroundColor;
        txtColor = info.textColor;
        break;
    }
    case IDM_STATUS_PROBLEMATIC:
    {
        StatusColorInfo info = GetStatusColorInfoByCode(4);
        newStatus = info.statusName;
        rowColor = info.backgroundColor;
        txtColor = info.textColor;
        break;
    }
    default: return;
    }

    // 3. Veritabanını Güncelle - Modüler helper kullanılarak
    DatabaseManager& db = DatabaseManager::GetInstance();
    CString codeField = GetCodeFieldForTable(table);

    if (db.UpdateFieldGlobal(table, codeField, code, statusField, newStatus))
    {
        // 4. Görünümü Güncelle
        SetRowColor(hItem, txtColor, rowColor);
        Invalidate();
    }
    else
    {
        MessageBox(_T("Durum güncellenemedi."), _T("Hata"), MB_ICONERROR);
    }
}






// CTreeListVDlg.cpp

// ============================================================================
// 🔥 MERKEZİ KOLON FABRİKASI (SINGLE SOURCE OF TRUTH)
// ============================================================================
// Tüm tabloların sütun yapıları, genişlikleri, hizalamaları ve özellikleri
// SADECE BURADAN yönetilir. Başka bir yerde MakeCol/AddColumn çağırmayın.
// ============================================================================

//std::vector<Win32xx::PropertyColumnInfo> CMyTreeListView::GetTableDefinition(const CString& tableName)
//{
//    // --- HELPER LAMBDA: Hızlı Kolon Tanımlama ---
//    // Kullanım: COL(Key, Title, Width, [Align], [Color], [EditType])
//    auto COL = [&](LPCTSTR key, LPCTSTR title, int width,
//        int align = HDF_LEFT,
//        COLORREF txtColor = CLR_DEFAULT,
//        EditControlType editType = Edit_None) -> Win32xx::PropertyColumnInfo
//        {
//            // PropertyColumnInfo struct'ını dolduruyoruz
//            Win32xx::PropertyColumnInfo c;
//            c.key = key;
//            c.title = title;
//            c.width = width;
//            c.format = align;
//            c.textColor = txtColor;
//            c.editType = editType;
//            c.visible = true;
//            c.order = -1; // Varsayılan sıra (listenin sonuna eklenir)
//            return c;
//        };
//
//    // Sabit Renkler
//    const COLORREF CLR_PRICE = RGB(180, 0, 0);   // Fiyatlar için Koyu Kırmızı
//    const COLORREF CLR_AREA = RGB(0, 100, 0);   // Metrekareler için Koyu Yeşil
//    const COLORREF CLR_STATUS = RGB(0, 0, 128);   // Durum için Lacivert
//
//    std::vector<Win32xx::PropertyColumnInfo> cols;
//
//    // =========================================================
//    // 1. CUSTOMER (Müşteri Tablosu)
//    // =========================================================
//    if (tableName.CompareNoCase(TABLE_NAME_CUSTOMER) == 0)
//    {
//        cols = {
//            COL(_T("Cari_Kod"),       _T("Cari Kod"),       110),
//            COL(_T("AdSoyad"),        _T("Adı Soyadı"),     200, HDF_LEFT, CLR_DEFAULT, Edit_TextBox),
//            COL(_T("Telefon"),        _T("Telefon"),        110, HDF_LEFT, CLR_DEFAULT, Edit_TextBox),
//            COL(_T("Telefon2"),       _T("Telefon 2"),      110, HDF_LEFT, CLR_DEFAULT, Edit_TextBox),
//            COL(_T("Email"),          _T("E-Posta"),        160, HDF_LEFT, CLR_DEFAULT, Edit_TextBox),
//            COL(_T("Sehir"),          _T("Şehir"),          100),
//            COL(_T("Ilce"),           _T("İlçe"),           100),
//            COL(_T("Durum"),          _T("Durum"),          80,  HDF_CENTER, CLR_STATUS),
//            COL(_T("MusteriTipi"),    _T("Tip"),            90),
//            COL(_T("Calisma_Durumu"), _T("Çalışma"),        80),
//            COL(_T("KayitTarihi"),    _T("Kayıt Tarihi"),   120),
//            COL(_T("TCKN"),           _T("TCKN"),           0),   // 0 width = Gizli (ama veri var)
//            COL(_T("VergiNo"),        _T("Vergi No"),       0),
//            COL(_T("Adres"),          _T("Adres"),          250),
//            COL(_T("Notlar"),         _T("Notlar"),         300),
//            // Sistem Alanları
//            COL(_T("sync_id"),        _T("Sync ID"),        0),
//            COL(_T("Updated_At"),      _T("Güncelleme"),     0),
//            COL(_T("Deleted"),        _T("Silindi"),        0)
//        };
//    }
//
//    // =========================================================
//    // 2. HOME (Konut)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_HOME) == 0)
//    {
//        cols = {
//            // 0. Tree Label (Otomatik dolu ama tanım burada dursun)
//            COL(_T("TreeLabel"),     _T("Mülk"),            120),
//
//            // Temel Bilgiler
//            COL(_T("Home_Code"),     _T("Ev Kodu"),         110),
//            COL(_T("ListingNo"),     _T("İlan No"),         100),
//            COL(_T("ListingDate"),   _T("İlan Tarihi"),     100),
//            COL(_T("PropertyType"),  _T("Emlak Tipi"),      110),
//
//            // Konum
//            COL(_T("City"),          _T("Şehir"),           100),
//            COL(_T("District"),      _T("İlçe"),            100),
//            COL(_T("Neighborhood"),  _T("Mahalle"),         120),
//            COL(_T("Location"),      _T("Konum"),           150),
//            COL(_T("Address"),       _T("Adres"),           200),
//
//            // Özellikler
//            COL(_T("RoomCount"),     _T("Oda"),             60,  HDF_CENTER),
//            COL(_T("NetArea"),       _T("Net m²"),          70,  HDF_RIGHT, CLR_AREA),
//            COL(_T("GrossArea"),     _T("Brüt m²"),         70,  HDF_RIGHT, CLR_AREA),
//            COL(_T("Floor"),         _T("Kat"),             50,  HDF_CENTER),
//            COL(_T("TotalFloor"),    _T("Kat Sayısı"),      60,  HDF_CENTER),
//            COL(_T("BuildingAge"),   _T("Bina Yaşı"),       70),
//            COL(_T("HeatingType"),   _T("Isıtma"),          110),
//            COL(_T("BathroomCount"), _T("Banyo"),           50,  HDF_CENTER),
//            COL(_T("KitchenType"),   _T("Mutfak"),          90),
//            COL(_T("Balcony"),       _T("Balkon"),          60),
//            COL(_T("Elevator"),      _T("Asansör"),         60),
//            COL(_T("Parking"),       _T("Otopark"),         80),
//            COL(_T("Furnished"),     _T("Eşyalı"),          60),
//            COL(_T("InSite"),        _T("Site İçi"),        60),
//            COL(_T("SiteName"),      _T("Site Adı"),        120),
//
//            // Finansal & Durum
//            COL(_T("Price"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("Currency"),      _T("Para"),            50),
//            COL(_T("PricePerM2"),    _T("m² Fiyatı"),       90,  HDF_RIGHT),
//            COL(_T("Dues"),          _T("Aidat"),           60,  HDF_RIGHT),
//            COL(_T("CreditEligible"),_T("Kredi"),           60),
//            COL(_T("DeedStatus"),    _T("Tapu"),            100),
//            COL(_T("Swap"),          _T("Takas"),           60),
//            COL(_T("Status"),        _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//
//            // Diğer
//            COL(_T("UsageStatus"),   _T("Kullanım"),        90),
//            COL(_T("SellerType"),    _T("Kimden"),          90),
//            COL(_T("WebsiteName"),   _T("Kaynak"),          90),
//            COL(_T("ListingURL"),    _T("URL"),             0),
//
//            // Sistem
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//
//            // 🔥 Mal Sahibi (En Sonda)
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    // =========================================================
//    // 3. LAND (Arsa)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_LAND) == 0)
//    {
//        cols = {
//            COL(_T("Land_Code"),     _T("Arsa Kodu"),       110),
//            COL(_T("Adres"),         _T("Adres"),           200),
//            COL(_T("Lokasyon"),      _T("Lokasyon"),        150),
//            COL(_T("Ada"),           _T("Ada"),             60),
//            COL(_T("Parsel"),        _T("Parsel"),          60),
//            COL(_T("Pafta"),         _T("Pafta"),           60),
//            COL(_T("Metrekare"),     _T("Alan (m²)"),       90,  HDF_RIGHT, CLR_AREA),
//            COL(_T("ImarDurumu"),    _T("İmar Durumu"),     120),
//            COL(_T("Fiyat"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("Teklifler"),     _T("Teklifler"),       150),
//            COL(_T("Durum"),         _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//            // Sistem
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    // =========================================================
//    // 4. FIELD (Tarla)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_FIELD) == 0)
//    {
//        cols = {
//            COL(_T("Field_Code"),    _T("Tarla Kodu"),      110),
//            COL(_T("ListingNo"),     _T("İlan No"),         100),
//            COL(_T("ListingDate"),   _T("İlan Tarihi"),     100),
//            COL(_T("PropertyType"),  _T("Emlak Tipi"),      100),
//            COL(_T("Adres"),         _T("Adres"),           200),
//            COL(_T("Lokasyon"),      _T("Lokasyon"),        150),
//            COL(_T("Metrekare"),     _T("Alan (m²)"),       90,  HDF_RIGHT, CLR_AREA),
//            COL(_T("PricePerM2"),    _T("m² Fiyatı"),       90,  HDF_RIGHT),
//            COL(_T("Fiyat"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("ToprakTuru"),    _T("Toprak"),          100),
//            COL(_T("Sulama"),        _T("Sulama"),          80),
//            COL(_T("Ulasim"),        _T("Ulaşım"),          100),
//            COL(_T("ImarDurumu"),    _T("İmar"),            100),
//            COL(_T("Ada"),           _T("Ada"),             60),
//            COL(_T("Parsel"),        _T("Parsel"),          60),
//            COL(_T("Pafta"),         _T("Pafta"),           60),
//            COL(_T("DeedStatus"),    _T("Tapu"),            100),
//            COL(_T("Kaks"),          _T("Kaks"),            60),
//            COL(_T("Gabari"),        _T("Gabari"),          60),
//            COL(_T("CreditEligible"),_T("Kredi"),           70),
//            COL(_T("Swap"),          _T("Takas"),           60),
//            COL(_T("SellerType"),    _T("Kimden"),          90),
//            COL(_T("Teklifler"),     _T("Teklifler"),       150),
//            COL(_T("Durum"),         _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//            // Sistem
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    // =========================================================
//    // 5. VINEYARD (Bağ/Bahçe)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_VINEYARD) == 0)
//    {
//        cols = {
//            COL(_T("Vineyard_Code"), _T("Bağ Kodu"),        110),
//            COL(_T("Adres"),         _T("Adres"),           200),
//            COL(_T("Lokasyon"),      _T("Lokasyon"),        150),
//            COL(_T("Metrekare"),     _T("Alan (m²)"),       90,  HDF_RIGHT, CLR_AREA),
//            COL(_T("AgacSayisi"),    _T("Ağaç Sayısı"),     80,  HDF_RIGHT),
//            COL(_T("UrunTuru"),      _T("Ürün"),            100),
//            COL(_T("Sulama"),        _T("Sulama"),          80),
//            COL(_T("Fiyat"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("Teklifler"),     _T("Teklifler"),       150),
//            COL(_T("Durum"),         _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//            // Sistem
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    // =========================================================
//    // 6. VILLA (Villa)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_VILLA) == 0)
//    {
//        cols = {
//            COL(_T("Villa_Code"),    _T("Villa Kodu"),      110),
//            COL(_T("Adres"),         _T("Adres"),           200),
//            COL(_T("Lokasyon"),      _T("Lokasyon"),        150),
//            COL(_T("OdaSayisi"),     _T("Oda"),             60),
//            COL(_T("NetMetrekare"),  _T("Net m²"),          80,  HDF_RIGHT, CLR_AREA),
//            COL(_T("BrutMetrekare"), _T("Brüt m²"),         80,  HDF_RIGHT, CLR_AREA),
//            COL(_T("KatSayisi"),     _T("Kat"),             60),
//            COL(_T("Havuz"),         _T("Havuz"),           70),
//            COL(_T("Bahce"),         _T("Bahçe"),           70),
//            COL(_T("Garaj"),         _T("Garaj"),           70),
//            COL(_T("Fiyat"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("Teklifler"),     _T("Teklifler"),       150),
//            COL(_T("Durum"),         _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//            // Sistem
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    // =========================================================
//    // 7. COMMERCIAL (Ticari)
//    // =========================================================
//    else if (tableName.CompareNoCase(TABLE_NAME_COMMERCIAL) == 0)
//    {
//        cols = {
//            COL(_T("Commercial_Code"),_T("Ticari Kod"),     110),
//            COL(_T("Kullanim"),      _T("Kullanım"),        120),
//            COL(_T("Adres"),         _T("Adres"),           200),
//            COL(_T("Lokasyon"),      _T("Lokasyon"),        150),
//            COL(_T("Alan"),          _T("Alan (m²)"),       90,  HDF_RIGHT, CLR_AREA),
//            COL(_T("Kat"),           _T("Kat"),             60),
//            COL(_T("Cephe"),         _T("Cephe"),           80),
//            COL(_T("Fiyat"),         _T("Fiyat"),           110, HDF_RIGHT, CLR_PRICE),
//            COL(_T("Teklifler"),     _T("Teklifler"),       150),
//            COL(_T("Durum"),         _T("Durum"),           80,  HDF_CENTER, CLR_STATUS),
//            // Sistem
//            COL(_T("Sync_ID"),       _T("Sync ID"),         0),
//            COL(_T("Updated_At"),     _T("Güncelleme"),      120),
//            COL(_T("Deleted"),       _T("Silindi"),         0),
//            COL(_T("Cari_Kod"),      _T("Mal Sahibi"),      100)
//        };
//    }
//
//    return cols;
//}




// ---------------- Dock Containers ----------------

CContainAllRestate::CContainAllRestate()
{
    SetTabText(L"Mülk Listesi");
    SetTabIcon(IDI_FILEVIEW);
    SetDockCaption(L"Müşteriye Ait Mülklerin Listesi");
    SetView(mView);
}
void CContainAllRestate::OnAttach()
{
    // 1. Toolbar Oluştur
    if (!m_ToolBar.IsWindow())
    {
        m_ToolBar.Create(*this);
        SetupToolBar();
    }

    // 2. TreeView Oluştur (Mevcut kodunuzdaki create işlemi buraya taşınabilir veya kalabilir)
    if (!mView.IsWindow())
    {
        mView.Create(*this);
    }

    RecalcLayout();
}


void CContainAllRestate::SetupToolBar()
{
    // 1. Stilleri Ayarla
    DWORD style = m_ToolBar.GetStyle();
    m_ToolBar.SetStyle(style | TBSTYLE_FLAT | CCS_NODIVIDER | TBSTYLE_TRANSPARENT);

    // 2. İkonları Yükle
    m_ToolBarImages.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);
    CBitmap bm(IDB_TREEVIEWICONS);
    m_ToolBarImages.Add(bm, RGB(192, 192, 192));
    m_ToolBar.SetImageList(m_ToolBarImages);

    // 3. Buton Boyutları + Yazı (ikon altında)
    m_ToolBar.SetBitmapSize(32, 32);
    m_ToolBar.SendMessage(TB_SETBUTTONSIZE, 0, MAKELPARAM(78, 56));
    m_ToolBar.SendMessage(TB_SETMAXTEXTROWS, 1, 0);
    {
        DWORD ex = (DWORD)m_ToolBar.SendMessage(TB_GETEXTENDEDSTYLE, 0, 0);
        ex |= TBSTYLE_EX_MIXEDBUTTONS;
        m_ToolBar.SendMessage(TB_SETEXTENDEDSTYLE, 0, ex);
    }

    // --- GRUP 1: Mülk Düzenleme ---
    m_ToolBar.AddButton(IDM_TREELIST_EDIT);       // Düzenle
    m_ToolBar.AddButton(IDM_TREELIST_VIEW);       // Görüntüle
    m_ToolBar.AddButton(IDM_TREELIST_PREVIEW);    // Hızlı Önizleme (Kart)
    m_ToolBar.AddButton(IDM_TREELIST_DELETE);     // Sil

    //m_ToolBar.AddSeparator();

    // --- GRUP 2: Satış Yönetimi (Durum) ---
    m_ToolBar.AddButton(IDM_TREELIST_SATILDI);    // Satıldı Yap!
    m_ToolBar.AddButton(IDM_STATUS_PRICE_DOWN);   // Fiyat Düştü Etiketi
    m_ToolBar.AddButton(IDM_TREELIST_SATIS_DEVAM);// Satışa Geri Dön
    m_ToolBar.AddButton(IDM_TREELIST_WAIT);       // Beklemeye Al

    //m_ToolBar.AddSeparator();

    // --- GRUP 3: Teklifler ve Belgeler ---
    m_ToolBar.AddButton(IDM_ONOFFER_MENU);        // Teklif Ver
    m_ToolBar.AddButton(IDM_VIEW_OFFERS);         // Gelen Teklifleri Gör

    //m_ToolBar.AddSeparator();

    m_ToolBar.AddButton(IDM_TEKLIF_FORMU);        // Teklif Formu Yazdır
    m_ToolBar.AddButton(IDM_YER_GOSTER);          // Yer Gösterme Belgesi
    m_ToolBar.AddButton(IDM_KIRA_KONTRATOSU);     // Kira Kontratı (Eğer kiralıksa)

    // --- Opsiyonel: Yetki Belgeleri ---
    // Yer kalmazsa bunları sağ tık menüsüne saklayabilirsiniz
    // m_ToolBar.AddButton(IDM_SATILIK_YETKI);

    // --- Button captions (under the icons) ---
    m_ToolBar.SetButtonText(IDM_TREELIST_EDIT,    L"Düzenle");
    m_ToolBar.SetButtonText(IDM_TREELIST_VIEW,    L"Görüntüle");
    m_ToolBar.SetButtonText(IDM_TREELIST_PREVIEW, L"Formlar");
    m_ToolBar.SetButtonText(IDM_TREELIST_DELETE,  L"Sil");
    m_ToolBar.SetButtonText(IDM_TREELIST_SATILDI, L"Satıldı");
    m_ToolBar.SetButtonText(IDM_STATUS_PRICE_DOWN, L"Fiyat ↓");
    m_ToolBar.SetButtonText(IDM_TREELIST_SATIS_DEVAM, L"Devam");
    m_ToolBar.SetButtonText(IDM_TREELIST_WAIT,    L"Bekle");
    m_ToolBar.SetButtonText(IDM_ONOFFER_MENU,     L"Teklif");
    m_ToolBar.SetButtonText(IDM_VIEW_OFFERS,      L"Teklifler");
    m_ToolBar.SetButtonText(IDM_TEKLIF_FORMU,     L"Form");
    m_ToolBar.SetButtonText(IDM_YER_GOSTER,       L"Yer Göster");
    m_ToolBar.SetButtonText(IDM_KIRA_KONTRATOSU,  L"Kira");

    // Force SHOWTEXT flag on each button
    int btnCount = (int)m_ToolBar.SendMessage(TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < btnCount; ++i)
    {
        TBBUTTON b{};
        if (!m_ToolBar.SendMessage(TB_GETBUTTON, i, (LPARAM)&b)) continue;
        if (b.idCommand == 0 || (b.fsStyle & TBSTYLE_SEP)) continue;

        TBBUTTONINFO bi{};
        bi.cbSize = sizeof(bi);
        bi.dwMask = TBIF_STYLE;
        bi.fsStyle = (BYTE)(b.fsStyle | BTNS_SHOWTEXT);
        m_ToolBar.SendMessage(TB_SETBUTTONINFO, (WPARAM)b.idCommand, (LPARAM)&bi);
    }

    m_ToolBar.Autosize();
}
// CMyTreeListView sınıfı içinde:

void CMyTreeListView::OnSetFocus()
{
    CRibbonFrameApp* pApp = GetContainerApp();
    if (pApp && pApp->GetMainFrame().IsWindow())
        ::PostMessage(pApp->GetMainFrame().GetHwnd(), UWM_CONTEXT_CHANGED, 2, 0);
}


void CMyTreeListView::UpdateButtonRects(HTREEITEM hItem, CRect rcItem)
{
    // 1. Satırın tam genişliğini pencereden alalım
    CRect rcClient = GetClientRect();

    // Buton boyutları (DPI uyumlu olması için DpiScaleInt kullanılabilir)
    int btnW = 24;
    int btnH = rcItem.Height() - 4; // Satırdan 2px üstten 2px alttan pay bırak
    int margin = 5; // Sağ kenardan boşluk

    // 2. hItem'ı saklayalım (Tıklama anında hangi öğe olduğunu bilmek için)
    m_btnArea.hItem = hItem;

    // 3. Yazdır (Print) Butonu - En Sağda
    m_btnArea.rcPrint = CRect(
        rcClient.right - btnW - margin,
        rcItem.top + 2,
        rcClient.right - margin,
        rcItem.top + 2 + btnH
    );

    // 4. Düzenle (Edit) Butonu - Yazdır butonunun solunda
    m_btnArea.rcEdit = CRect(
        m_btnArea.rcPrint.left - btnW - 4, // 4px butonlar arası boşluk
        rcItem.top + 2,
        m_btnArea.rcPrint.left - 4,
        rcItem.top + 2 + btnH
    );
}
void CMyTreeListView::TriggerAction(int btnType, HTREEITEM hItem)
{
    if (!hItem) return;

    if (btnType == 1) // DÜZENLE
    {
        // Senin zaten yazdığın OnEditCommand fonksiyonunu kullanmak en güvenlisidir.
        // Çünkü o fonksiyon tablo tipini (HomeTbl, Land vb.) otomatik çözer.
        OnEditCommand(hItem);
    }
    else if (btnType == 2) // YAZDIR/ÖNİZLE
    {
        // Önizleme komutunu tetikle
        OnPreviewCommand();
    }
}// CMyTreeListView.cpp içinde
// CMyTreeListView.cpp içinde
LRESULT CMyTreeListView::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
{
    LPNMTVCUSTOMDRAW pNMTVCD = (LPNMTVCUSTOMDRAW)lparam;

    switch (pNMTVCD->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPOSTPAINT:
    {
        HTREEITEM hItem = (HTREEITEM)pNMTVCD->nmcd.dwItemSpec;

        // Sadece SEÇİLİ öğe için veya HOVER durumunda butonları göster
        if (hItem == GetSelection() || hItem == m_hoverItem)
        {
            CDC dc(pNMTVCD->nmcd.hdc);
            CRect rcItem;
            TreeView_GetItemRect(GetHwnd(), hItem, &rcItem, TRUE);

            // Buton koordinatlarını en sağa sabitle
            CRect rcFull = GetClientRect();
            rcItem.right = rcFull.right;

            UpdateButtonRects(hItem, rcItem);

            // --- GÖRSEL İYİLEŞTİRME ---
            // Edit Butonu (Mavi tonlu hover)
            COLORREF editColor = (m_hotButton == 1) ? RGB(0, 120, 215) : RGB(240, 240, 240);
            CBrush brEdit(editColor);
            dc.FillRect(m_btnArea.rcEdit, brEdit);
            dc.DrawEdge(m_btnArea.rcEdit, EDGE_RAISED, BF_RECT);
            dc.SetTextColor((m_hotButton == 1) ? RGB(255, 255, 255) : RGB(0, 0, 0));
            dc.DrawText(L"✎", -1, m_btnArea.rcEdit, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Print Butonu (Yeşil tonlu hover)
            COLORREF printColor = (m_hotButton == 2) ? RGB(34, 139, 34) : RGB(240, 240, 240);
            CBrush brPrint(printColor);
            dc.FillRect(m_btnArea.rcPrint, brPrint);
            dc.DrawEdge(m_btnArea.rcPrint, EDGE_RAISED, BF_RECT);
            dc.SetTextColor((m_hotButton == 2) ? RGB(255, 255, 255) : RGB(0, 0, 0));
            dc.DrawText(L"🖨", -1, m_btnArea.rcPrint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return CDRF_DODEFAULT;
    }
    }
    return CTreeListView::OnNotifyReflect(wparam, lparam);
}




void CContainAllRestate::RecalcLayout()
{
    CRect rcClient = GetClientRect();

    // 1. Toolbar Yerleşimi
    int toolbarHeight = 0;
    if (m_ToolBar.IsWindow())
    {
        CSize sz = m_ToolBar.GetMaxSize();
        toolbarHeight = sz.cy;

        // Garanti Yükseklik (Dar kalmasın)
        if (toolbarHeight < 28) toolbarHeight = 30;

        m_ToolBar.SetWindowPos(NULL, 0, 0, rcClient.Width(), toolbarHeight, SWP_NOZORDER);
    }

    // Kalan alanı ayarla
    rcClient.top += toolbarHeight;

    // 2. Liste Yerleşimi
    if (mView.IsWindow())
    {
        mView.SetWindowPos(NULL, rcClient, SWP_NOZORDER);
    }
}



LRESULT CContainAllRestate::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    CRibbonFrameApp* pApp = GetContainerApp();

    CMainFrame* pMainFrame = &pApp->GetMainFrame();

    try
    {
        switch (msg)
        {
            // ✅ Zemin Boyama (Siyahlık Çözümü)
        case WM_ERASEBKGND:
        {
            CDC dc((HDC)wparam);
            CRect rc = GetClientRect();
            dc.FillRect(rc, GetSysColorBrush(COLOR_3DFACE));
            return 1;
        }

        // ✅ Komut Yönlendirme (MainFrame'e)
        case WM_COMMAND:
            if (pApp && pApp->GetMainFrame().IsWindow())
                return pApp->GetMainFrame().SendMessage(WM_COMMAND, wparam, lparam);
            break;

            // Pencere boyutu değişince layout yenile
        case WM_SIZE:
            RecalcLayout();
            break;
        }

        return WndProcDefault(msg, wparam, lparam);
    }
    catch (...)
    {
        // Hata yakalama bloğunuz (Mevcut kodunuzdaki gibi kalabilir)
        return 0;
    }
}

BOOL CContainAllRestate::OnCommand(WPARAM wparam, LPARAM lparam)
{
    const UINT id = LOWORD(wparam);

    // 1) Bu docker'ın domain'i: MÜLK (TreeList)
    if (IsPropertyCmd(id))
        return mView.OnCommand(wparam, lparam);

    // 2) Global komutlar MainFrame
    CRibbonFrameApp* pApp = GetContainerApp();
    if (pApp && pApp->GetMainFrame().IsWindow())
        return (BOOL)pApp->GetMainFrame().SendMessage(WM_COMMAND, wparam, lparam);

    return FALSE;
}










CDockAllRestate::CDockAllRestate()
{
    SetView(m_contain);
    SetBarWidth(8);
}

LRESULT CDockAllRestate::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try {
        return WndProcDefault(msg, wparam, lparam);
    }
    catch (const CException& e) {
        CString str1; str1 << e.GetText() << L'\n' << e.GetErrorString();
        CString str2; str2 << "Error: " << e.what();
        TaskDialogBox(nullptr, str1, str2, TD_ERROR_ICON);
    }
    catch (const std::exception& e) {
        CString str1 = e.what();
        ::MessageBox(nullptr, str1, L"Error: std::exception", MB_ICONERROR);
    }
    return 0;
}

