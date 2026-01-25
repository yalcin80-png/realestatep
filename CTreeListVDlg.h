#pragma once


#include "stdafx.h"
#include "dataIsMe.h" 
#include "resource.h" // Dialog ve Kontrol ID'leri için
#include <unordered_map>
#include <windows.h> // En genel Win32 API başlığı
#include "vVillaDlg.h"
#include "vLandDlg.h"
#include "vFieldDlg.h"
#include "vHomeDlg.h"
#include "vCommercialDlg.h"
#include "vCarDlg.h"
#include "vVineyardDlg.h"
#include "PreviewPanel.h"
#include "OfferDialog.h"
#include "OfferListDlg.h"
#include "coldlg.h"
#include <map>
#include <set>
#include <vector>
#include <regex> // dosyanın en üstüne ekle
#include <functional>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


// Basit tablo adları (kategori anahtarları)
#define TABLE_NAME_HOME        _T("HomeTbl")
#define TABLE_NAME_LAND        _T("Land")
#define TABLE_NAME_FIELD       _T("Field")
#define TABLE_NAME_VINEYARD    _T("Vineyard")
#define TABLE_NAME_VILLA       _T("Villa")
#define TABLE_NAME_COMMERCIAL  _T("Commercial")
#define TABLE_NAME_CAR          _T("Car")


#define IDC_HEADER_TREE_LISTVIEW  3012

#define FAKE_HEADER_MARK 0xDEADBEEF

#ifndef IMAGE_HOME
#define IMAGE_HOME            2
#define IMAGE_LAND            3
#define IMAGE_FIELD           4
#define IMAGE_VINEYARD        5
#define IMAGE_VILLA           6
#define IMAGE_COMMERCIAL      7
#define IMAGE_CAR             8
#define IMAGE_CONTAINER_ARROW 9
#endif

struct ItemColorInfo
{
    COLORREF textColor;
    COLORREF bkColor;
};

struct SubColumnInfo
{
    CString title;
    int width;
    int align;
};


#define IDC_TREEHEADER 30128


static const PropertyTypeInfo HOME_INFO = { TABLE_NAME_HOME,      _T("HomeTbl"),       IMAGE_HOME };
static const PropertyTypeInfo LAND_INFO = { TABLE_NAME_LAND,      _T("Land"),           IMAGE_LAND };
static const PropertyTypeInfo FIELD_INFO = { TABLE_NAME_FIELD,     _T("Field"),         IMAGE_FIELD };
static const PropertyTypeInfo VINEYARD_INFO = { TABLE_NAME_VINEYARD,  _T("Vineyard"),  IMAGE_VINEYARD };
static const PropertyTypeInfo VILLA_INFO = { TABLE_NAME_VILLA,     _T("Villa"),                 IMAGE_VILLA };
static const PropertyTypeInfo COMM_INFO = { TABLE_NAME_COMMERCIAL,_T("Commercial"),   IMAGE_COMMERCIAL };
static const PropertyTypeInfo CAR_INFO  = { TABLE_NAME_CAR,       _T("Car"),          IMAGE_CAR };


// --------------------------------------------------------
//  Satır meta bilgisi – her TreeItem için DB bağlamı
// --------------------------------------------------------
struct TLV_RowMeta
{
    CString tableName;   // HomeTbl, Land, Field, ...
    CString pkField;     // Home_Code, Land_Code, ...
    CString pkValue;     // Home001234, ...
};

#include "ColorMenu.h"
#include "RadialMenu.h"
#include "MatrixMenu.h"

// --------------------------------------------------------
//  Ana TreeListView Sınıfı
// --------------------------------------------------------

class CMyTreeListView : public CTreeListView
{
private :
    HTREEITEM m_hLastContextItem;
public:
    BOOL OnPreviewCommand();
    //std::vector<Win32xx::PropertyColumnInfo> GetTableDefinition(const Win32xx::CString& tableName);

    CColorMenu m_contextMenuManager; // Menü yöneticisi
    CColorMenu m_menuDrawer;
    //CRadialMenu m_radialMenu;
    std::unique_ptr<CRadialMenu> m_pRadialMenu;
    std::unique_ptr<CMatrixMenu> m_pMatrixMenu; // Yeni sınıf
public:
    //CMyTreeListView() {}
    CMyTreeListView() : m_hoverItem(nullptr), m_hotButton(0) {}
    virtual ~CMyTreeListView() { SaveSettings(); }

    void OnActionButtonClick(int btnID, HTREEITEM hItem)
    {
        if (btnID == 1) {
            OnEditCommand(hItem);
        }
        else if (btnID == 2) {
            OnPreviewCommand();
        }
        else if (btnID == 3) {
            if (MessageBox(L"Silmek istiyor musunuz?", L"Onay", MB_YESNO) == IDYES) {
                // Silme işlemi...
            }
        }
    }
    virtual void OnActionBarClick(int btnIdx, HTREEITEM hItem) {
        if (!hItem) return;

        if (btnIdx == 1) { // DÜZENLE (Kalem ikonu)
            OnEditCommand(hItem);
        }
        else if (btnIdx == 2) { // YAZDIR/ÖNİZLE (Yazıcı ikonu)
            OnPreviewCommand();
        }
    }
    LRESULT OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam);
    bool HasSelection() const { return GetSelectedItem() != nullptr; }

    struct ButtonArea {
        CRect rcEdit;
        CRect rcPrint;
        HTREEITEM hItem;
    };
    ButtonArea m_btnArea;
    HTREEITEM m_hoverItem; // Mouse o an hangi satırda
    int m_hotButton;       // 0: Yok, 1: Edit, 2: Print
    void UpdateButtonRects(HTREEITEM hItem, CRect rcItem);
    void TriggerAction(int btnType, HTREEITEM hItem);

    LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);


    int ShowContextMenu(const CPoint& ptScreen, HTREEITEM hItem);


    CString GetSettingsFilePath()
    {
        TCHAR szPath[MAX_PATH];
        ::GetModuleFileName(NULL, szPath, MAX_PATH);
        CString path = szPath;
        int pos = path.ReverseFind('\\');
        if (pos > 0) path = path.Left(pos);
        path += _T("\\ListViewSettings.ini");
        return path;
    }
    COLORREF GetColorForStatus(const CString& status);
    std::vector<Win32xx::PropertyColumnInfo> GetTableDefinition(const CString& tableName);


    void ChangePropertyStatus(HTREEITEM hItem, UINT cmdId);

    // MyTreeListView.h içinde:

    void SaveSettings()
    {
        // 1. Kategori listesi boşsa döngüye girme
        if (m_categories.empty())
            return;

        CString file = GetSettingsFilePath();

        for (const auto& cat : m_categories)
        {
            // 2. Kategori işaretçisi geçerli mi?
            if (!cat) continue;

            CString section = cat->name;

            for (size_t i = 0; i < cat->columns.size(); ++i)
            {
                // 1. Genişlik Kaydı
                CString keyW; keyW.Format(_T("Col_%d_Width"), (int)i);
                CString valW; valW.Format(_T("%d"), cat->columns[i].width);
                ::WritePrivateProfileString(section, keyW, valW, file);

                // 2. Görünürlük Kaydı
                CString keyV; keyV.Format(_T("Col_%d_Visible"), (int)i);
                CString valV; valV.Format(_T("%d"), cat->columns[i].visible ? 1 : 0);
                ::WritePrivateProfileString(section, keyV, valV, file);
            }
        }
    }
    // Override
    void OnEditColumns()
    {
        OpenColumnManagerForSelected();
    }


    void OpenColumnManagerForSelected()
    {
        // Seçili öğeyi al
        HTREEITEM hItem = GetSelection();
        if (!hItem)
        {
            MessageBox(_T("Lütfen bir kategori veya satır seçin."), _T("Uyarı"), MB_ICONWARNING);
            return;
        }

        // Hangi kategoriye ait olduğunu bul
        TLV_Category* cat = GetCategoryForRow(hItem);

        // Eğer doğrudan bir Header'a (Kategoriye) tıkladıysa
        if (!cat && IsHeaderItem(hItem)) {
            cat = FindCategoryByHeader(hItem);
        }

        if (cat)
        {
            // Diyaloğu oluştur ve kolon referansını ver
            CColumnManagerDlg dlg(cat->columns);

            // Diyalog "Tamam" ile kapanırsa
            // MyTreeListView.h içinde:
            if (dlg.DoModal(GetHwnd()) == IDOK) // <-- Sadece IDOK dönerse kaydet
            {
                SaveSettings();
                Invalidate();
                UpdateWindow();
            }
        }
    }
    void LoadSettings()
    {
        CString file = GetSettingsFilePath();

        for (const auto& cat : m_categories)
        {
            CString section = cat->name;

            for (size_t i = 0; i < cat->columns.size(); ++i)
            {
                // 1. Genişlik Yükle
                CString keyW; keyW.Format(_T("Col_%d_Width"), (int)i);
                int defaultW = cat->columns[i].width;
                int savedW = ::GetPrivateProfileInt(section, keyW, defaultW, file);
                cat->columns[i].width = savedW;

                // 2. Görünürlük Yükle (YENİ EKLENDİ)
                CString keyV; keyV.Format(_T("Col_%d_Visible"), (int)i);
                // Varsayılan olarak true (1) kabul edelim
                int defaultV = cat->columns[i].visible ? 1 : 1;
                int savedV = ::GetPrivateProfileInt(section, keyV, defaultV, file);
                cat->columns[i].visible = (savedV != 0);
            }
        }
    }    // Sınıf başlatıldığında kategorileri ve sütunları oluşturur
    void Initialize()
    {
        // 1. TEMA AYARLARI
            // Başlık satırında hem Kategori Adı hem de Kolon Başlıkları olacağı için
            // yüksekliği artırmamız lazım. (Standart 24px yetmez, 50px yapalım)
        GetTheme().categoryHeaderHeight = 24;
        GetTheme().headerHeight = 24;
        GetTheme().rowHeight = 24;
        TreeView_SetItemHeight(GetHwnd(), 52);


        SetActionButtonSize(48, 48, 5);
        ClearActionButtons();
        AddActionButton(1, L"✎", RGB(230, 230, 230), RGB(0, 120, 215));   // Düzenle (Mavi Hover)
        AddActionButton(2, L"🖨", RGB(230, 230, 230), RGB(34, 139, 34));   // Yazdır (Yeşil Hover)
        AddActionButton(3, L"🗑", RGB(230, 230, 230), RGB(200, 0, 0));     // Sil (Kırmızı Hover)

        LoadSettings();
    }



    // Veritabanından verileri çekip listeyi doldurur
// MyTreeListView.h dosyasının içinde

    void LoadAllData()
    {
        SetRedraw(FALSE); // Çizimi dondur

        // Temizle
        DeleteAllItems();
        m_categories.clear();
        m_catByHeader.clear();
        m_rowByHandle.clear();

        Initialize(); // Sadece tema ayarlarını yap

        auto& db = DatabaseManager::GetInstance();
        if (!db.EnsureConnection()) {
            SetRedraw(TRUE);
            MessageBox(_T("Veritabanı bağlantısı yok!"), _T("Hata"), MB_ICONERROR);
            return;
        }

        // --- B) KONUTLAR ---
        std::vector<Home_cstr> listHome = db.GetAllGlobal<Home_cstr>();
        if (!listHome.empty()) // <--- KONTROL BURADA
        {
            SetupHomeColumns(); // Header ve Kolonları ŞİMDİ oluştur
            HTREEITEM hCatHome = FindHeaderByCategoryName(TABLE_NAME_HOME);
            if (hCatHome) {
                for (const auto& item : listHome) AddHomeRow(hCatHome, item);
                Expand(hCatHome, TVE_EXPAND);
            }
        }

        // --- C) ARSALAR ---
        std::vector<Land_cstr> listLand = db.GetAllGlobal<Land_cstr>();
        if (!listLand.empty())
        {
            SetupLandColumns();
            HTREEITEM hCatLand = FindHeaderByCategoryName(TABLE_NAME_LAND);
            if (hCatLand) {
                for (const auto& item : listLand) AddLandRow(hCatLand, item);
                Expand(hCatLand, TVE_EXPAND);
            }
        }

        // --- D) TARLALAR ---
        std::vector<Field_cstr> listField = db.GetAllGlobal<Field_cstr>();
        if (!listField.empty())
        {
            SetupFieldColumns();
            HTREEITEM hCatField = FindHeaderByCategoryName(TABLE_NAME_FIELD);
            if (hCatField) {
                for (const auto& item : listField) AddFieldRow(hCatField, item);
                Expand(hCatField, TVE_EXPAND);
            }
        }

        // --- E) BAĞ/BAHÇE ---
        std::vector<Vineyard_cstr> listVineyard = db.GetAllGlobal<Vineyard_cstr>();
        if (!listVineyard.empty())
        {
            SetupVineyardColumns();
            HTREEITEM hCatVineyard = FindHeaderByCategoryName(TABLE_NAME_VINEYARD);
            if (hCatVineyard) {
                for (const auto& item : listVineyard) AddVineyardRow(hCatVineyard, item);
                Expand(hCatVineyard, TVE_EXPAND);
            }
        }

        // --- F) VİLLALAR ---
        std::vector<Villa_cstr> listVilla = db.GetAllGlobal<Villa_cstr>();
        if (!listVilla.empty())
        {
            SetupVillaColumns();
            HTREEITEM hCatVilla = FindHeaderByCategoryName(TABLE_NAME_VILLA);
            if (hCatVilla) {
                for (const auto& item : listVilla) AddVillaRow(hCatVilla, item);
                Expand(hCatVilla, TVE_EXPAND);
            }
        }

        // --- G) TİCARİ ---
        std::vector<Commercial_cstr> listComm = db.GetAllGlobal<Commercial_cstr>();
        if (!listComm.empty())
        {
            SetupCommercialColumns();
            HTREEITEM hCatComm = FindHeaderByCategoryName(TABLE_NAME_COMMERCIAL);
            if (hCatComm) {
                for (const auto& item : listComm) AddCommercialRow(hCatComm, item);
                Expand(hCatComm, TVE_EXPAND);
            }
        }
        // --- H) ARABA ---
        std::vector<Car_cstr> listCar = db.GetAllGlobal<Car_cstr>();
        if (!listCar.empty()) // ✅ DÜZELTİLDİ: Artık listCar kontrol ediliyor
        {
            SetupCarColumns();
            HTREEITEM hCatCar = FindHeaderByCategoryName(TABLE_NAME_CAR); // ✅ Değişken adı düzeltildi
            if (hCatCar) {
                for (const auto& item : listCar) AddCarRow(hCatCar, item);
                Expand(hCatCar, TVE_EXPAND);
            }
        }
        // 🔥 ÖNEMLİ: Sütunlar yeni oluşturulduğu için ayarları (genişlikleri) şimdi yüklüyoruz.
        LoadSettings();

        SetRedraw(TRUE); // Çizimi tekrar aç
        Invalidate();
        UpdateWindow();
    }
    // MyTreeListView.h içine, public bölümüne ekleyin:

// -------------------------------------------------------------------------
// 3. MÜŞTERİ PORTFÖYÜNÜ YÜKLE (FİLTRELİ GÖRÜNÜM)
// -------------------------------------------------------------------------
// 
    // CTreeListVDlg.cpp dosyasının içinde:

 
    void LoadPortfolioByCariKod(const CString& cariKod)
    {
        SetRedraw(FALSE);

        DeleteAllItems();
        m_categories.clear();
        m_catByHeader.clear();
        m_rowByHandle.clear();

        Initialize(); // Sadece tema

        auto& db = DatabaseManager::GetInstance();
        if (!db.EnsureConnection()) {
            SetRedraw(TRUE);
            return;
        }

        // --- B) KONUTLAR ---
        std::vector<Home_cstr> listHome = db.GetHomesByCariKod(cariKod);
        if (!listHome.empty())
        {
            SetupHomeColumns();
            HTREEITEM hCatHome = FindHeaderByCategoryName(TABLE_NAME_HOME);
            if (hCatHome) {
                for (const auto& item : listHome) AddHomeRow(hCatHome, item);
                Expand(hCatHome, TVE_EXPAND);
            }
        }

        // --- C) ARSALAR ---
        std::vector<Land_cstr> listLand = db.GetLandsByCariKod(cariKod);
        if (!listLand.empty())
        {
            SetupLandColumns();
            HTREEITEM hCatLand = FindHeaderByCategoryName(TABLE_NAME_LAND);
            if (hCatLand) {
                for (const auto& item : listLand) AddLandRow(hCatLand, item);
                Expand(hCatLand, TVE_EXPAND);
            }
        }

        // --- D) TARLALAR ---
        std::vector<Field_cstr> listField = db.GetFieldsByCariKod(cariKod);
        if (!listField.empty())
        {
            SetupFieldColumns();
            HTREEITEM hCatField = FindHeaderByCategoryName(TABLE_NAME_FIELD);
            if (hCatField) {
                for (const auto& item : listField) AddFieldRow(hCatField, item);
                Expand(hCatField, TVE_EXPAND);
            }
        }

        // --- E) BAĞ/BAHÇE ---
        std::vector<Vineyard_cstr> listVineyard = db.GetVineyardsByCariKod(cariKod);
        if (!listVineyard.empty())
        {
            SetupVineyardColumns();
            HTREEITEM hCatVineyard = FindHeaderByCategoryName(TABLE_NAME_VINEYARD);
            if (hCatVineyard) {
                for (const auto& item : listVineyard) AddVineyardRow(hCatVineyard, item);
                Expand(hCatVineyard, TVE_EXPAND);
            }
        }

        // --- F) VİLLALAR ---
        std::vector<Villa_cstr> listVilla = db.GetVillasByCariKod(cariKod);
        if (!listVilla.empty())
        {
            SetupVillaColumns();
            HTREEITEM hCatVilla = FindHeaderByCategoryName(TABLE_NAME_VILLA);
            if (hCatVilla) {
                for (const auto& item : listVilla) AddVillaRow(hCatVilla, item);
                Expand(hCatVilla, TVE_EXPAND);
            }
        }

        // --- G) TİCARİ ---
        std::vector<Commercial_cstr> listComm = db.GetCommercialsByCariKod(cariKod);
        if (!listComm.empty())
        {
            SetupCommercialColumns();
            HTREEITEM hCatComm = FindHeaderByCategoryName(TABLE_NAME_COMMERCIAL);
            if (hCatComm) {
                for (const auto& item : listComm) AddCommercialRow(hCatComm, item);
                Expand(hCatComm, TVE_EXPAND);
            }
        }

        // --- H) ARABA ---
        std::vector<Car_cstr> listCar = db.GetCarsByCariKod(cariKod);
        if (!listCar.empty()) // ✅ DÜZELTİLDİ
        {
            SetupCarColumns();
            HTREEITEM hCatCar = FindHeaderByCategoryName(TABLE_NAME_CAR);
            if (hCatCar) {
                for (const auto& item : listCar) AddCarRow(hCatCar, item);
                Expand(hCatCar, TVE_EXPAND);
            }
        }

        // Ayarları yükle (genişlikler vs.)
        LoadSettings();

        // Varsa ilk kategoriyi görünür yap
        if (!m_categories.empty()) {
            TreeView_SelectSetFirstVisible(GetHwnd(), m_categories[0]->hHeader);
        }

        SetRedraw(TRUE);
        Invalidate();
    }


protected:
    // =============================================================
    // 1. SÜTUN TANIMLAMALARI (INIT COLUMN SET)
    // =============================================================
        void SetupCustomerColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Müşteri Listesi"), TABLE_NAME_CUSTOMER, TVI_ROOT, TVI_LAST, TLV_ICON_CATEGORY);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;
            cat->headerColor = RGB(75, 102, 204);
            AddColumn(cat, _T("Cari_Kod"), _T("Cari Kod"), 100);
            AddColumn(cat, _T("AdSoyad"), _T("Adı Soyadı"), 180, DT_LEFT, true, Edit_TextBox);
            AddColumn(cat, _T("Telefon"), _T("Telefon"), 100, DT_LEFT, true, Edit_TextBox);
            AddColumn(cat, _T("Email"), _T("E-Posta"), 150, DT_LEFT, true, Edit_TextBox);
            AddColumn(cat, _T("Sehir"), _T("Şehir"), 90);
            AddColumn(cat, _T("Ilce"), _T("İlçe"), 90);
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);
            AddColumn(cat, _T("Calisma_Durumu"), _T("Çalışma"), 80);
            AddColumn(cat, _T("KayitTarihi"), _T("Kayıt Tarihi"), 110);
            AddColumn(cat, _T("Adres"), _T("Adres"), 250);
            AddColumn(cat, _T("Notlar"), _T("Notlar"), 300);
        }

        void SetupHomeColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Konut Portföyü"), TABLE_NAME_HOME, TVI_ROOT, TVI_LAST, TLV_ICON_HOME);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;
            cat->headerColor = RGB(0, 102, 204);
            AddColumn(cat, _T("Cari_Kod"), _T("Mülk Kodu"), 100);
            //AddColumn(cat, _T("Home_Code"), _T("Ev Kodu"), 135);
            AddColumn(cat, _T("Listing_No"), _T("İlan Numarası"), 180);
            AddColumn(cat, _T("Listing_Date"), _T("İlan Tarihi"), 180);
            AddColumn(cat, _T("Property_Type"), _T("Mülk Tipi"), 180);
            AddColumn(cat, _T("Address"), _T("Adres"), 180);
            AddColumn(cat, _T("City"), _T("Şehir"), 180);
            AddColumn(cat, _T("District"), _T("İlçe"), 180);
            AddColumn(cat, _T("Neighborhood"), _T("Mahalle"), 180);
            AddColumn(cat, _T("Location"), _T("Konum"), 180);
            AddColumn(cat, _T("Rooms"), _T("Oda Sayısı"), 180);
            AddColumn(cat, _T("Area_Net"), _T("Net Alan"), 180);
            AddColumn(cat, _T("Area_Gross"), _T("Brüt Alan"), 180);
            AddColumn(cat, _T("Floor"), _T("Kat"), 180);
            AddColumn(cat, _T("Total_Floors"), _T("Toplam Kat Sayısı"), 180);
            AddColumn(cat, _T("Building_Age"), _T("Bina Yaşı"), 180);
            AddColumn(cat, _T("Heating_Type"), _T("Isıtma Tipi"), 180);
            AddColumn(cat, _T("Bathrooms"), _T("Banyo Sayısı"), 180);
            AddColumn(cat, _T("Kitchen_Type"), _T("Mutfak Tipi"), 180);
            AddColumn(cat, _T("Balcony"), _T("Balkon"), 180);
            AddColumn(cat, _T("Elevator"), _T("Asansör"), 180);
            AddColumn(cat, _T("Parking"), _T("Otopark"), 180);
            AddColumn(cat, _T("Furnished"), _T("Eşyalı"), 180);
            AddColumn(cat, _T("Usage_Status"), _T("Kullanım Durumu"), 180);
            AddColumn(cat, _T("In_Site"), _T("Site İçinde"), 180);
            AddColumn(cat, _T("Site_Name"), _T("Site Adı"), 180);
            AddColumn(cat, _T("Maintenance"), _T("Aidat"), 180);
            AddColumn(cat, _T("Credit_Valid"), _T("Krediye Uygunluk"), 180);
            AddColumn(cat, _T("Deed_Status"), _T("Tapu Durumu"), 180);
            AddColumn(cat, _T("Seller_Type"), _T("Satıcı Tipi"), 180);
            AddColumn(cat, _T("Exchange"), _T("Takas"), 180);
            AddColumn(cat, _T("Price"), _T("Fiyat"), 180, HDF_RIGHT, RGB(180, 0, 0));
            AddColumn(cat, _T("Currency"), _T("Para Birimi"), 180);
            AddColumn(cat, _T("Price_m2"), _T("Metrekare Başına Fiyat"), 180);
            AddColumn(cat, _T("Website_Name"), _T("Web Sitesi Adı"), 180);
            AddColumn(cat, _T("Listing_URL"), _T("İlan URL'si"), 180);
            AddColumn(cat, _T("Public_Note"), _T("Genel Not"), 180);
            AddColumn(cat, _T("Private_Note"), _T("İç Not"), 180);
            AddColumn(cat, _T("Purchase_Date"), _T("Satın Alma Tarihi"), 180);
            AddColumn(cat, _T("Purchase_Price"), _T("Satın Alma Fiyatı"), 180);
            AddColumn(cat, _T("Sale_Date"), _T("Satış Tarihi"), 180);
            AddColumn(cat, _T("Sale_Price"), _T("Satış Fiyatı"), 180);
            AddColumn(cat, _T("Energy_Class"), _T("Enerji Sınıfı"), 180);
            //AddColumn(cat, _T("Geo_Lat"), _T("Coğrafi Enlem"), 180);
            //AddColumn(cat, _T("Geo_Lon"), _T("Coğrafi Boylam"), 180);
            //AddColumn(cat, _T("Photo_Count"), _T("Fotoğraf Sayısı"), 180);
            //AddColumn(cat, _T("Status"), _T("Durum"), 180);
            //AddColumn(cat, _T("Updated_At"), _T("Güncellenme Zamanı"), 180);
            //AddColumn(cat, _T("Sync_ID"), _T("Senkronizasyon Kimliği"), 180);
            //AddColumn(cat, _T("Deleted"), _T("Silindi"), 180);
        }

        void SetupLandColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Arsa Portföyü"), TABLE_NAME_LAND, TVI_ROOT, TVI_LAST, TLV_ICON_LAND);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            cat->headerColor = RGB(34, 139, 34); // Forest Green (Yeşil Başlık)

            // 0. Sütun: Arsa Kodu (Tree Label)
            AddColumn(cat, _T("Land_Code"), _T("Arsa Kodu"), 110);

            // 1. Adres (Eksikti)
            AddColumn(cat, _T("Adres"), _T("Adres"), 200);

            // 2. Lokasyon
            AddColumn(cat, _T("Lokasyon"), _T("Lokasyon"), 150);

            // 3. Ada
            AddColumn(cat, _T("Ada"), _T("Ada"), 60);

            // 4. Parsel
            AddColumn(cat, _T("Parsel"), _T("Parsel"), 60);

            // 5. Pafta (Eksikti)
            AddColumn(cat, _T("Pafta"), _T("Pafta"), 60);

            // 6. Metrekare (Sayısal)
            AddColumn(cat, _T("Metrekare"), _T("Alan (m²)"), 80, DT_RIGHT);

            // 7. İmar Durumu
            AddColumn(cat, _T("ImarDurumu"), _T("İmar Durumu"), 120);

            // 8. Fiyat (Sayısal)
            AddColumn(cat, _T("Fiyat"), _T("Fiyat"), 100, DT_RIGHT);

            // 9. Teklifler (Eksikti)
            AddColumn(cat, _T("Teklifler"), _T("Teklifler"), 150);

            // 10. Durum (Eksikti - Renklendirme için önemli)
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 11. Sync ID (Genelde gizli tutulabilir ama veri orada olsun)
            AddColumn(cat, _T("Sync_ID"), _T("Sync ID"), 0);

            // 12. Güncelleme Zamanı
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 13. Silindi Bilgisi
            AddColumn(cat, _T("Deleted"), _T("Silindi"), 0);

            // 14. Mal Sahibi (Cari Kod) - EN SONA
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }

        void SetupFieldColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Tarla Portföyü"), TABLE_NAME_FIELD, TVI_ROOT, TVI_LAST, TLV_ICON_FIELD);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            // Başlık Rengi: Toprak/Turuncu tonu
            cat->headerColor = RGB(210, 105, 30);

            // 0. Sütun: Tarla Kodu (Tree Label)
            AddColumn(cat, _T("Field_Code"), _T("Tarla Kodu"), 110);

            // 1. Adres
            AddColumn(cat, _T("Adres"), _T("Adres"), 200);

            // 2. Lokasyon
            AddColumn(cat, _T("Lokasyon"), _T("Lokasyon"), 150);

            // 3. Metrekare (Sayısal - Sağa dayalı)
            AddColumn(cat, _T("Metrekare"), _T("m²"), 80, DT_RIGHT);

            // 4. Fiyat (Sayısal - Sağa dayalı)
            AddColumn(cat, _T("Fiyat"), _T("Fiyat"), 100, DT_RIGHT);

            // 5. Ada
            AddColumn(cat, _T("Ada"), _T("Ada"), 60);

            // 6. Parsel
            AddColumn(cat, _T("Parsel"), _T("Parsel"), 60);

            // 7. Pafta
            AddColumn(cat, _T("Pafta"), _T("Pafta"), 60);

            // 8. İmar Durumu
            AddColumn(cat, _T("ImarDurumu"), _T("İmar Durumu"), 120);

            // 9. Tapu Durumu
            AddColumn(cat, _T("DeedStatus"), _T("Tapu Durumu"), 100);

            // 10. Toprak Türü
            AddColumn(cat, _T("ToprakTuru"), _T("Toprak"), 100);

            // 11. Sulama
            AddColumn(cat, _T("Sulama"), _T("Sulama"), 80);

            // 12. Ulaşım
            AddColumn(cat, _T("Ulasim"), _T("Ulaşım"), 100);

            // 13. Kaks
            AddColumn(cat, _T("Kaks"), _T("Kaks"), 60);

            // 14. Gabari
            AddColumn(cat, _T("Gabari"), _T("Gabari"), 60);

            // 15. m² Fiyatı
            AddColumn(cat, _T("PricePerM2"), _T("m² Fiyatı"), 90);

            // 16. İlan No
            AddColumn(cat, _T("ListingNo"), _T("İlan No"), 90);

            // 17. İlan Tarihi
            AddColumn(cat, _T("ListingDate"), _T("İlan Tarihi"), 100);

            // 18. Emlak Tipi
            AddColumn(cat, _T("PropertyType"), _T("Emlak Tipi"), 100);

            // 19. Krediye Uygunluk
            AddColumn(cat, _T("CreditEligible"), _T("Kredi"), 70);

            // 20. Kimden
            AddColumn(cat, _T("SellerType"), _T("Kimden"), 90);

            // 21. Takas
            AddColumn(cat, _T("Swap"), _T("Takas"), 60);

            // 22. Teklifler
            AddColumn(cat, _T("Teklifler"), _T("Teklifler"), 150);

            // 23. Durum (Status)
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 24. Sync ID (Genelde gizli olabilir ama veri tutarlılığı için ekliyoruz)
            AddColumn(cat, _T("Sync_ID"), _T("Sync ID"), 0);

            // 25. Güncelleme Zamanı
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 26. Silindi Bilgisi
            AddColumn(cat, _T("Deleted"), _T("Silindi"), 0);

            // 27. Mal Sahibi (Cari Kod) - EN SONA
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }
        void SetupVineyardColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Bağ & Bahçe"), TABLE_NAME_VINEYARD, TVI_ROOT, TVI_LAST, TLV_ICON_VINEYARD);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            cat->headerColor = RGB(218, 165, 32); // Goldenrod (Sarı/Altın)

            // 0. Sütun: Bağ Kodu (Tree Label)
            AddColumn(cat, _T("Vineyard_Code"), _T("Bağ Kodu"), 110);

            // 1. Adres
            AddColumn(cat, _T("Adres"), _T("Adres"), 200);

            // 2. Lokasyon
            AddColumn(cat, _T("Lokasyon"), _T("Lokasyon"), 150);

            // 3. Metrekare (Sayısal)
            AddColumn(cat, _T("Metrekare"), _T("Alan (m²)"), 80, DT_RIGHT);

            // 4. Ağaç Sayısı
            AddColumn(cat, _T("AgacSayisi"), _T("Ağaç Sayısı"), 70, DT_RIGHT);

            // 5. Ürün Türü
            AddColumn(cat, _T("UrunTuru"), _T("Ürün Türü"), 100);

            // 6. Sulama
            AddColumn(cat, _T("Sulama"), _T("Sulama"), 80);

            // 7. Fiyat (Sayısal)
            AddColumn(cat, _T("Fiyat"), _T("Fiyat"), 100, DT_RIGHT);

            // 8. Teklifler
            AddColumn(cat, _T("Teklifler"), _T("Teklifler"), 150);

            // 9. Durum
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 10. Sync ID
            AddColumn(cat, _T("Sync_ID"), _T("Sync ID"), 0);

            // 11. Güncelleme Zamanı
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 12. Silindi
            AddColumn(cat, _T("Deleted"), _T("Silindi"), 0);

            // 13. Mal Sahibi (Cari Kod) - EN SONA
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }




        void SetupVillaColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Villa Portföyü"), TABLE_NAME_VILLA, TVI_ROOT, TVI_LAST, TLV_ICON_VILLA);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            cat->headerColor = RGB(70, 130, 180); // Steel Blue

            // 0. Sütun: Villa Kodu (Tree Label)
            AddColumn(cat, _T("Villa_Code"), _T("Villa Kodu"), 110);

            // 1. Adres
            AddColumn(cat, _T("Adres"), _T("Adres"), 200);

            // 2. Lokasyon
            AddColumn(cat, _T("Lokasyon"), _T("Lokasyon"), 150);

            // 3. Oda Sayısı
            AddColumn(cat, _T("OdaSayisi"), _T("Oda"), 60);

            // 4. Net m²
            AddColumn(cat, _T("NetMetrekare"), _T("Net m²"), 70, DT_RIGHT);

            // 5. Brüt m²
            AddColumn(cat, _T("BrutMetrekare"), _T("Brüt m²"), 70, DT_RIGHT);

            // 6. Kat Sayısı
            AddColumn(cat, _T("KatSayisi"), _T("Kat"), 50);

            // 7. Havuz
            AddColumn(cat, _T("Havuz"), _T("Havuz"), 60);

            // 8. Bahçe
            AddColumn(cat, _T("Bahce"), _T("Bahçe"), 60);

            // 9. Garaj
            AddColumn(cat, _T("Garaj"), _T("Garaj"), 60);

            // 10. Fiyat
            AddColumn(cat, _T("Fiyat"), _T("Fiyat"), 100, DT_RIGHT);

            // 11. Teklifler
            AddColumn(cat, _T("Teklifler"), _T("Teklifler"), 150);

            // 12. Durum
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 13. Sync ID
            AddColumn(cat, _T("Sync_ID"), _T("Sync ID"), 0);

            // 14. Güncelleme Zamanı
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 15. Silindi
            AddColumn(cat, _T("Deleted"), _T("Silindi"), 0);

            // 16. Mal Sahibi (Cari Kod - EN SONA)
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }
        void SetupCommercialColumns()
        {
            HTREEITEM hHeader = InsertHeader(_T("Ticari Mülkler"), TABLE_NAME_COMMERCIAL, TVI_ROOT, TVI_LAST, TLV_ICON_COMMERCIAL);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            cat->headerColor = RGB(105, 105, 105); // Dim Gray (Kurumsal)

            // 0. Sütun: Ticari Kod
            AddColumn(cat, _T("Commercial_Code"), _T("Ticari Kod"), 110);

            // 1. Adres
            AddColumn(cat, _T("Adres"), _T("Adres"), 200);

            // 2. Lokasyon
            AddColumn(cat, _T("Lokasyon"), _T("Lokasyon"), 150);

            // 3. Alan
            AddColumn(cat, _T("Alan"), _T("Alan (m²)"), 80, DT_RIGHT);

            // 4. Kat
            AddColumn(cat, _T("Kat"), _T("Kat"), 50);

            // 5. Cephe
            AddColumn(cat, _T("Cephe"), _T("Cephe"), 80);

            // 6. Kullanım
            AddColumn(cat, _T("Kullanim"), _T("Kullanım"), 120);

            // 7. Fiyat
            AddColumn(cat, _T("Fiyat"), _T("Fiyat"), 100, DT_RIGHT);

            // 8. Teklifler
            AddColumn(cat, _T("Teklifler"), _T("Teklifler"), 150);

            // 9. Durum
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 10. Sync ID
            AddColumn(cat, _T("Sync_ID"), _T("Sync ID"), 0);

            // 11. Güncelleme
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 12. Silindi
            AddColumn(cat, _T("Deleted"), _T("Silindi"), 0);

            // 13. Mal Sahibi (Cari Kod - EN SONA)
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }
        
        void SetupCarColumns()
        {
            // Kategori Başlığı: "Araç Listesi" veya "Otomobiller"
// ✅ DÜZELTİLDİ: TABLE_NAME_CAR ve IMAGE_CAR kullanıldı
            HTREEITEM hHeader = InsertHeader(_T("Araç Listesi"), TABLE_NAME_CAR, TVI_ROOT, TVI_LAST, IMAGE_CAR);
            TLV_Category* cat = FindCategoryByHeader(hHeader);
            if (!cat) return;

            cat->headerColor = RGB(105, 105, 105); // Dim Gray

            // --- Sütun Tanımları (GetProperties yapısına göre) ---

            // 0. Sütun: Araç Kodu (Tree yapısının ana elemanı)
            AddColumn(cat, _T("Car_Code"), _T("Araç Kodu"), 110);

            // 1. İlan No
            AddColumn(cat, _T("ListingNo"), _T("İlan No"), 90);

            // 2. Tarih
            AddColumn(cat, _T("ListingDate"), _T("Tarih"), 90);

            // 3. Başlık
            AddColumn(cat, _T("Title"), _T("Başlık"), 200);

            // 4. Marka
            AddColumn(cat, _T("Brand"), _T("Marka"), 100);

            // 5. Seri
            AddColumn(cat, _T("Series"), _T("Seri"), 100);

            // 6. Model
            AddColumn(cat, _T("Model"), _T("Model"), 100);

            // 7. Yıl
            AddColumn(cat, _T("Year"), _T("Yıl"), 60);

            // 8. Km (Sağa yaslı genelde sayısal olduğu için)
            AddColumn(cat, _T("Km"), _T("Km"), 80, DT_RIGHT);

            // 9. Fiyat (Sağa yaslı)
            AddColumn(cat, _T("Price"), _T("Fiyat"), 100, DT_RIGHT);

            // 10. Para Birimi
            AddColumn(cat, _T("Currency"), _T("Birim"), 50);

            // 11. Yakıt
            AddColumn(cat, _T("FuelType"), _T("Yakıt"), 80);

            // 12. Vites
            AddColumn(cat, _T("Transmission"), _T("Vites"), 80);

            // 13. Durum
            AddColumn(cat, _T("Durum"), _T("Durum"), 80);

            // 14. Güncelleme Zamanı
            AddColumn(cat, _T("Updated_At"), _T("Güncelleme"), 120);

            // 15. Cari Kod (Mal Sahibi) - Genelde en sonda durur
            AddColumn(cat, _T("Cari_Kod"), _T("Mal Sahibi"), 90);
        }

        // =============================================================
    // 2. VERİ EKLEME YARDIMCILARI (ROW POPULATION)
    // =============================================================

    void AddCustomerRow(HTREEITEM hHeader, const Customer_cstr& c)
    {
        // 🚀 InsertDataRow: Hem Windows'a ekler hem Model'e kaydeder (Map)
        HTREEITEM hItem = InsertDataRow(hHeader, c.Cari_Kod, c.Cari_Kod);
        if (!hItem) return;

        // Artık modelde var olduğu için SetSubItemText güvenlidir
        SetSubItemText(hItem, 1, c.AdSoyad);
        SetSubItemText(hItem, 2, c.Telefon);
        SetSubItemText(hItem, 3, c.Email);
        SetSubItemText(hItem, 4, c.Sehir);
        SetSubItemText(hItem, 5, c.Ilce);
        SetSubItemText(hItem, 6, c.Durum);
        SetSubItemText(hItem, 7, c.Calisma_Durumu);
        SetSubItemText(hItem, 8, c.KayitTarihi);
        SetSubItemText(hItem, 9, c.Adres);
        SetSubItemText(hItem, 10, c.Notlar);

        if (c.Durum == _T("Pasif"))
            SetRowColor(hItem, RGB(150, 150, 150), CLR_NONE);
    }

    void AddHomeRow(HTREEITEM hHeader, const Home_cstr& h)
    {
        // Home_Code key olarak kullanılır, 0. sütun metni de Home_Code
        HTREEITEM hItem = InsertDataRow(hHeader, h.Home_Code, h.Home_Code);
        if (!hItem) return;

        //SetSubItemText(hItem, 1, h.Cari_Kod);
        SetSubItemText(hItem, 1, h.ListingNo);
        SetSubItemText(hItem, 2, h.ListingDate);
        SetSubItemText(hItem, 3, h.PropertyType);
        SetSubItemText(hItem, 4, h.Address);
        SetSubItemText(hItem, 5, h.City);
        SetSubItemText(hItem, 6, h.District);
        SetSubItemText(hItem, 7, h.Neighborhood);
        SetSubItemText(hItem, 8, h.Location);
        SetSubItemText(hItem, 9, h.RoomCount);
        SetSubItemText(hItem, 10, h.NetArea);
        SetSubItemText(hItem, 11, h.GrossArea);
        SetSubItemText(hItem, 12, h.Floor);
        SetSubItemText(hItem, 13, h.TotalFloor);
        SetSubItemText(hItem, 14, h.BuildingAge);
        SetSubItemText(hItem, 15, h.HeatingType);
        SetSubItemText(hItem, 16, h.BathroomCount);
        SetSubItemText(hItem, 17, h.KitchenType);
        SetSubItemText(hItem, 18, h.Balcony);
        SetSubItemText(hItem, 19, h.Elevator);
        SetSubItemText(hItem, 20, h.Parking);
        SetSubItemText(hItem, 21, h.Furnished);
        SetSubItemText(hItem, 22, h.UsageStatus);
        SetSubItemText(hItem, 23, h.InSite);
        SetSubItemText(hItem, 24, h.SiteName);
        SetSubItemText(hItem, 25, h.Dues);
        SetSubItemText(hItem, 26, h.CreditEligible);
        SetSubItemText(hItem, 27, h.DeedStatus);
        SetSubItemText(hItem, 28, h.SellerType);
        SetSubItemText(hItem, 29, h.Swap);
        SetSubItemText(hItem, 30, h.Price);
        SetSubItemText(hItem, 31, h.Currency);
        SetSubItemText(hItem, 32, h.PricePerM2);
        SetSubItemText(hItem, 33, h.WebsiteName);
        SetSubItemText(hItem, 34, h.ListingURL);
        SetSubItemText(hItem, 35, h.NoteGeneral);
        SetSubItemText(hItem, 36, h.NoteInternal);
        SetSubItemText(hItem, 37, h.PurchaseDate);
        SetSubItemText(hItem, 38, h.PurchasePrice);
        SetSubItemText(hItem, 39, h.SaleDate);
        SetSubItemText(hItem, 40, h.SalePrice);
        SetSubItemText(hItem, 41, h.EnergyClass);
        SetSubItemText(hItem, 42, h.GeoLatitude);
        SetSubItemText(hItem, 43, h.GeoLongitude);
        SetSubItemText(hItem, 44, h.PhotoCount);
        SetSubItemText(hItem, 45, h.Status);
        SetSubItemText(hItem, 46, h.Updated_At);
        SetSubItemText(hItem, 47, h.sync_id);
        SetSubItemText(hItem, 48, h.Deleted);
        // 🔥 RENKLENDİRME MANTIĞI (YENİ) 🔥
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (h.Status.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220); // Gri
            txtColor = RGB(128, 128, 128);
        }
        else if (h.Status.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220); // Yeşil
        }
        else if (h.Status.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230); // Kırmızı
        }
        else if (h.Status.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    
    }

    void AddLandRow(HTREEITEM hHeader, const Land_cstr& l)
    {
        // 0. Sütun: Land_Code (InsertDataRow ile atanır)
        HTREEITEM hItem = InsertDataRow(hHeader, l.Land_Code, l.Land_Code);
        if (!hItem) return;

        // --- Sütunları Doldur (SetupLandColumns sırasına birebir uymalı) ---

        SetSubItemText(hItem, 1, l.Adres);        // Adres
        SetSubItemText(hItem, 2, l.Lokasyon);     // Lokasyon
        SetSubItemText(hItem, 3, l.Ada);          // Ada
        SetSubItemText(hItem, 4, l.Parsel);       // Parsel
        SetSubItemText(hItem, 5, l.Pafta);        // Pafta
        SetSubItemText(hItem, 6, l.Metrekare);    // Metrekare
        SetSubItemText(hItem, 7, l.ImarDurumu);   // İmar Durumu
        SetSubItemText(hItem, 8, l.Fiyat);        // Fiyat
        SetSubItemText(hItem, 9, l.Teklifler);    // Teklifler
        SetSubItemText(hItem, 10, l.Durum);       // Durum
        SetSubItemText(hItem, 11, l.sync_id);     // Sync ID
        SetSubItemText(hItem, 12, l.Updated_At);   // Updated_At
        SetSubItemText(hItem, 13, l.Deleted);     // Deleted

        // En Son: Cari Kod (Mal Sahibi)
        SetSubItemText(hItem, 14, l.Cari_Kod);

        // --- Renklendirme Mantığı ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (l.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220); // Gri
            txtColor = RGB(128, 128, 128);
        }
        else if (l.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220); // Yeşil
        }
        else if (l.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230); // Kırmızı
        }
        else if (l.Durum.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }

    void AddFieldRow(HTREEITEM hHeader, const Field_cstr& f)
    {
        // 0. Sütun: Field_Code
        HTREEITEM hItem = InsertDataRow(hHeader, f.Field_Code, f.Field_Code);
        if (!hItem) return;

        // --- Sütunları Doldur ---
        SetSubItemText(hItem, 1, f.Adres);
        SetSubItemText(hItem, 2, f.Lokasyon);
        SetSubItemText(hItem, 3, f.Metrekare);
        SetSubItemText(hItem, 4, f.Fiyat);
        SetSubItemText(hItem, 5, f.Ada);
        SetSubItemText(hItem, 6, f.Parsel);
        SetSubItemText(hItem, 7, f.Pafta);
        SetSubItemText(hItem, 8, f.ImarDurumu);
        SetSubItemText(hItem, 9, f.DeedStatus);
        SetSubItemText(hItem, 10, f.ToprakTuru);
        SetSubItemText(hItem, 11, f.Sulama);
        SetSubItemText(hItem, 12, f.Ulasim);
        SetSubItemText(hItem, 13, f.Kaks);
        SetSubItemText(hItem, 14, f.Gabari);
        SetSubItemText(hItem, 15, f.PricePerM2);
        SetSubItemText(hItem, 16, f.ListingNo);
        SetSubItemText(hItem, 17, f.ListingDate);
        SetSubItemText(hItem, 18, f.PropertyType);
        SetSubItemText(hItem, 19, f.CreditEligible);
        SetSubItemText(hItem, 20, f.SellerType);
        SetSubItemText(hItem, 21, f.Swap);
        SetSubItemText(hItem, 22, f.Teklifler);
        SetSubItemText(hItem, 23, f.Durum);
        SetSubItemText(hItem, 24, f.sync_id);
        SetSubItemText(hItem, 25, f.Updated_At);
        SetSubItemText(hItem, 26, f.Deleted);

        // 27. Cari Kod (En Sonda)
        SetSubItemText(hItem, 27, f.Cari_Kod);

        // --- Renklendirme Mantığı (Field_cstr içinde 'Durum' kullanılır) ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (f.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220); // Gri
            txtColor = RGB(128, 128, 128);
        }
        else if (f.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220); // Yeşil
        }
        else if (f.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230); // Kırmızı
        }
        else if (f.Durum.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }
    void AddVineyardRow(HTREEITEM hHeader, const Vineyard_cstr& v)
    {
        // 0. Sütun: Vineyard_Code
        HTREEITEM hItem = InsertDataRow(hHeader, v.Vineyard_Code, v.Vineyard_Code);
        if (!hItem) return;

        // --- Sütunları Doldur ---
        SetSubItemText(hItem, 1, v.Adres);
        SetSubItemText(hItem, 2, v.Lokasyon);
        SetSubItemText(hItem, 3, v.Metrekare);
        SetSubItemText(hItem, 4, v.AgacSayisi);
        SetSubItemText(hItem, 5, v.UrunTuru);
        SetSubItemText(hItem, 6, v.Sulama);
        SetSubItemText(hItem, 7, v.Fiyat);
        SetSubItemText(hItem, 8, v.Teklifler);
        SetSubItemText(hItem, 9, v.Durum);
        SetSubItemText(hItem, 10, v.sync_id);
        SetSubItemText(hItem, 11, v.Updated_At);
        SetSubItemText(hItem, 12, v.Deleted);

        // 13. Cari Kod (En Sonda)
        SetSubItemText(hItem, 13, v.Cari_Kod);

        // --- Renklendirme Mantığı ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (v.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220); // Gri
            txtColor = RGB(128, 128, 128);
        }
        else if (v.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220); // Yeşil
        }
        else if (v.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230); // Kırmızı
        }
        else if (v.Durum.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }
    void AddVillaRow(HTREEITEM hHeader, const Villa_cstr& v)
    {
        // 0. Sütun
        HTREEITEM hItem = InsertDataRow(hHeader, v.Villa_Code, v.Villa_Code);
        if (!hItem) return;

        // --- Sütunları Doldur ---
        SetSubItemText(hItem, 1, v.Adres);
        SetSubItemText(hItem, 2, v.Lokasyon);
        SetSubItemText(hItem, 3, v.OdaSayisi);
        SetSubItemText(hItem, 4, v.NetMetrekare);
        SetSubItemText(hItem, 5, v.BrutMetrekare);
        SetSubItemText(hItem, 6, v.KatSayisi);
        SetSubItemText(hItem, 7, v.Havuz);
        SetSubItemText(hItem, 8, v.Bahce);
        SetSubItemText(hItem, 9, v.Garaj);
        SetSubItemText(hItem, 10, v.Fiyat);
        SetSubItemText(hItem, 11, v.Teklifler);
        SetSubItemText(hItem, 12, v.Durum);
        SetSubItemText(hItem, 13, v.sync_id);
        SetSubItemText(hItem, 14, v.Updated_At);
        SetSubItemText(hItem, 15, v.Deleted);

        // 16. Cari Kod (En Sonda)
        SetSubItemText(hItem, 16, v.Cari_Kod);

        // --- Renklendirme ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (v.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220);
            txtColor = RGB(128, 128, 128);
        }
        else if (v.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220);
        }
        else if (v.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230);
        }
        else if (v.Durum.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }
    void AddCommercialRow(HTREEITEM hHeader, const Commercial_cstr& c)
    {
        // 0. Sütun
        HTREEITEM hItem = InsertDataRow(hHeader, c.Commercial_Code, c.Commercial_Code);
        if (!hItem) return;

        // --- Sütunları Doldur ---
        SetSubItemText(hItem, 1, c.Adres);
        SetSubItemText(hItem, 2, c.Lokasyon);
        SetSubItemText(hItem, 3, c.Alan);
        SetSubItemText(hItem, 4, c.Kat);
        SetSubItemText(hItem, 5, c.Cephe);
        SetSubItemText(hItem, 6, c.Kullanim);
        SetSubItemText(hItem, 7, c.Fiyat);
        SetSubItemText(hItem, 8, c.Teklifler);
        SetSubItemText(hItem, 9, c.Durum);
        SetSubItemText(hItem, 10, c.sync_id);
        SetSubItemText(hItem, 11, c.Updated_At);
        SetSubItemText(hItem, 12, c.Deleted);

        // 13. Cari Kod (En Sonda)
        SetSubItemText(hItem, 13, c.Cari_Kod);

        // --- Renklendirme ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (c.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220);
            txtColor = RGB(128, 128, 128);
        }
        else if (c.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220);
        }
        else if (c.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230);
        }
        else if (c.Durum.CompareNoCase(_T("Pasif")) == 0)
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE)
        {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }

    void AddCarRow(HTREEITEM hHeader, const Car_cstr& c)
    {
        // 0. Sütun: Araç Kodu ile satırı oluştur
        HTREEITEM hItem = InsertDataRow(hHeader, c.Car_Code, c.Car_Code);
        if (!hItem) return;

        // --- Sütunları Doldur (SetupCarColumns sırasına göre) ---

        // 1. İlan No
        SetSubItemText(hItem, 1, c.ListingNo);

        // 2. Tarih
        // Not: Eğer c.ListingDate tarih objesiyse stringe çevrilmeli, metinse direkt verilebilir.
        // Burada metin olduğunu varsayıyoruz, değilse FormatDate(c.ListingDate) kullanılmalı.
        SetSubItemText(hItem, 2, c.ListingDate);

        // 3. Başlık
        SetSubItemText(hItem, 3, c.Title);

        // 4. Marka
        SetSubItemText(hItem, 4, c.Brand);

        // 5. Seri
        SetSubItemText(hItem, 5, c.Series);

        // 6. Model
        SetSubItemText(hItem, 6, c.Model);

        // 7. Yıl
        SetSubItemText(hItem, 7, c.Year);

        // 8. Km
        SetSubItemText(hItem, 8, c.Km);

        // 9. Fiyat
        // Price struct'ta DOUBLE görünüyor. CString formatı gerekebilir.
        CString strPrice;
        strPrice.Format(_T("%.2f"), c.Price); // Veya "%.0f" kuruşsuz isterseniz
        SetSubItemText(hItem, 9, strPrice);

        // 10. Para Birimi
        SetSubItemText(hItem, 10, c.Currency);

        // 11. Yakıt
        SetSubItemText(hItem, 11, c.FuelType);

        // 12. Vites
        SetSubItemText(hItem, 12, c.Transmission);

        // 13. Durum
        SetSubItemText(hItem, 13, c.Durum);

        // 14. Güncelleme
        SetSubItemText(hItem, 14, c.Updated_At);

        // 15. Cari Kod (Mal Sahibi)
        SetSubItemText(hItem, 15, c.Cari_Kod);


        // --- Renklendirme Mantığı ---
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (c.Durum.CompareNoCase(_T("Satıldı")) == 0)
        {
            rowColor = RGB(220, 220, 220); // Gri Arkaplan
            txtColor = RGB(128, 128, 128); // Koyu Gri Yazı
        }
        else if (c.Durum.CompareNoCase(_T("Fiyat Düştü")) == 0)
        {
            rowColor = RGB(220, 255, 220); // Açık Yeşil Arkaplan
        }
        else if (c.Durum.CompareNoCase(_T("Acil")) == 0)
        {
            rowColor = RGB(255, 230, 230); // Açık Kırmızı Arkaplan
        }
        else if (c.Durum.CompareNoCase(_T("Pasif")) == 0) // Statü veya Durum kontrol edilebilir
        {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        // Opsiyonel: Silinmiş kayıtlar için özel renk
        if (c.Deleted)
        {
            txtColor = RGB(255, 0, 0); // Kırmızı yazı (örnek)
        }

        if (rowColor != CLR_NONE || txtColor != CLR_NONE)
        {
            // Eğer txtColor atanmamışsa varsayılan siyah kalsın diye kontrol eklenebilir
            // Ancak SetRowColor fonksiyonunuzun yapısına göre ikisini de göndermek gerekebilir.
            SetRowColor(hItem, txtColor == CLR_NONE ? RGB(0, 0, 0) : txtColor, rowColor);
        }
    }

    void OnCreateAuthorizationContract();
    void OnCreateDepositContract();
    void OnCreateShowingAgreement();
    void OnCreatePreemptionWaiver(); // <--- YENİ
    void OnCreateRentalContract();
    void OnCreateOfferLetter();
    void OnCreateRentalAuthContract();
    void OnCreateOffer();
    public:
    BOOL OnCommand(WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        const UINT id = LOWORD(wparam);
        switch (id)
        {
        case IDM_SAVECOLUMNWITH:
            break;
        case IDM_ONOFFER_MENU:
        {
            OnCreateOffer();
            return TRUE;
		}
        case IDM_TREELIST_PREVIEW:
        {
            OnPreviewCommand();
            return TRUE;
        }
        case IDM_FONT_CHANGE: // Menü ID'si varsayımıyla
        {
            OnCreateAuthorizationContract();
           // OnFontChangeCommand();
            return TRUE;
        }
        case IDM_KIRALIK_TEK_YETKI:
        {
            OnCreateRentalAuthContract();
            return TRUE;
        }
        case IDM_SATILIK_YETKI:
        {
            OnCreateAuthorizationContract();
            // OnFontChangeCommand();
            return TRUE;
        }
        case IDM_ONALIM_FERAGAT:
        {
            OnCreatePreemptionWaiver();
            return TRUE;
        }
        case IDM_TEKLIF_FORMU:
        {
            OnCreateOfferLetter();
            return TRUE;
        }
        case IDM_TREELIST_EDIT: // Düzenle
            OnEditCommand(GetSelectedItem());
            // MessageBox(L"Düzenle tıklandı", L"Bilgi", MB_OK);
            return TRUE;

        case IDM_TREELIST_DELETE: // Sil
            //OnDeleteCommand(GetSelectedItem());
            return TRUE;
        case IDM_KIRA_KONTRATOSU: // resource.h içinde IDM_KIRA_KONTRATOSU tanımlı olmalı
        {
            OnCreateRentalContract();
            return TRUE;
        }

        case IDM_VIEW_OFFERS: // (IDM_ONOFFER_MENU değil, Görüntüleme ID'si)
        {
            HTREEITEM hItem = GetSelectedItem();
            if (!hItem) return TRUE;

            CString table, codeField, code;
            if (ResolveTableAndCode(hItem, table, codeField, code))
            {
                // Teklif Listesini Aç
                COfferListDlg dlg(code); // Mülk kodunu gönderiyoruz
                dlg.DoModal(*this);
            }
            return TRUE;
        }


        case ID_CONTEXT_EDIT_COLUMNS:
            OnEditColumns();
            break;
        case IDM_YER_GOSTER:OnCreateShowingAgreement();
            break;
        case IDM_STATUS_ACTIVE:
        case IDM_STATUS_SOLD:
        case IDM_STATUS_PRICE_DOWN:
        case IDM_STATUS_URGENT:
        case IDM_STATUS_PASSIVE:
        {
            HTREEITEM hItem = GetSelectedItem();
            if (hItem && GetItemData(hItem) != (DWORD_PTR)-1) // Header değilse
            {
                ChangePropertyStatus(hItem, id);
            }
        }
        return TRUE;
        case IDM_CAYMA_AKCESI: // Özellikler
            OnCreateDepositContract();
            return TRUE;
        }

        return FALSE; // işlem yapılmadıysa false döndür
    }
    void OnSetFocus();



    // ============================================================================
// YER GÖSTERME BELGESİ OLUŞTURMA
// ============================================================================










    void ApplyStatusColor(HTREEITEM hItem, const CString& status)
    {
        COLORREF rowColor = CLR_NONE;
        COLORREF txtColor = CLR_NONE;

        if (status.CompareNoCase(_T("Satıldı")) == 0) {
            rowColor = RGB(220, 220, 220); // Gri
            txtColor = RGB(128, 128, 128);
        }
        else if (status.CompareNoCase(_T("Fiyat Düştü")) == 0) {
            rowColor = RGB(220, 255, 220); // Yeşil
        }
        else if (status.CompareNoCase(_T("Acil")) == 0) {
            rowColor = RGB(255, 230, 230); // Kırmızı
        }
        else if (status.CompareNoCase(_T("Pasif")) == 0) {
            rowColor = RGB(240, 240, 240);
            txtColor = RGB(160, 160, 160);
        }

        if (rowColor != CLR_NONE) {
            SetRowColor(hItem, txtColor, rowColor);
        }
    }


    // Tek ve Dev Fonksiyon: Her türlü mülk tipini ekler
    //template <typename T>
    //void AddGenericRow(HTREEITEM hHeader, const T& data)
    //{
    //    // 1. Tablo Adını ve PK'yı Trait'ten al
    //    CString tableName = PropertyTrait<T>::GetTableName();
    //    CString pkValue = PropertyTrait<T>::GetCodeValue(data);

    //    // 2. Satırı Oluştur (InsertDataRow ile)
    //    HTREEITEM hItem = InsertDataRow(hHeader, pkValue, pkValue);
    //    if (!hItem) return;

    //    // 3. Struct verisini Map'e çevir (Reflection benzeri yapı)
    //    // dataIsMe.h içindeki DatabaseManager::GetPropertyFieldMap fonksiyonunu kullanıyoruz
    //    auto fieldMap = DatabaseManager::GetInstance().GetPropertyFieldMap(data);

    //    // 4. Tablonun Kolon Tanımlarını Getir
    //    // (InitColumnSets ile doldurulan m_localTableColumns'dan)
    //    auto itCols = m_localTableColumns.find(tableName);
    //    if (itCols == m_localTableColumns.end()) return;
    //    const auto& cols = itCols->second;

    //    // 5. Kolon sırasına göre verileri yerleştir
    //    // i=0 TreeLabel olduğu için 1'den başlıyoruz (veya yapınıza göre 0'dan)
    //    // Sizin yapınızda 0. kolon InsertDataRow ile doluyor.
    //    for (size_t i = 0; i < cols.size(); ++i)
    //    {
    //        // Kolonun "key" değeri (örn: "Fiyat", "Adres") ile map'ten veriyi çek
    //        CString key = cols[i].key;

    //        // Map'te bu anahtar varsa değerini yaz
    //        if (fieldMap.count(key))
    //        {
    //            SetSubItemText(hItem, (int)i, fieldMap[key]);
    //        }
    //    }

    //    // 6. Otomatik Renklendirme
    //    // Map içinde "Status" veya "Durum" alanını bul
    //    CString status;
    //    if (fieldMap.count(_T("Status"))) status = fieldMap[_T("Status")];
    //    else if (fieldMap.count(_T("Durum"))) status = fieldMap[_T("Durum")];

    //    ApplyStatusColor(hItem, status);
    //}





    //void SetupAllColumnsGeneric()
    //{
    //    // m_localTableColumns zaten InitColumnSets() içinde doldurulmuş olmalı.

    //    // Her tablo tanımı için dön
    //    for (auto& kv : m_localTableColumns)
    //    {
    //        CString tableName = kv.first;
    //        auto& colList = kv.second;

    //        // Tabloya uygun ikonu bul
    //        int iconIndex = IconForCategory(tableName);

    //        // Kategori Başlığını (Header) Oluştur
    //        // Not: DisplayName'i bulmak için g_propertyTypes'da arama yapabiliriz
    //        CString displayName = tableName;
    //        for (const auto& pt : g_propertyTypes) {
    //            if (pt.TableName == tableName) { displayName = pt.DisplayName; break; }
    //        }

    //        HTREEITEM hHeader = InsertHeader(displayName, tableName, TVI_ROOT, TVI_LAST, iconIndex);
    //        TLV_Category* cat = FindCategoryByHeader(hHeader);
    //        if (!cat) continue;

    //        // Renkleri Ata (Bunu da bir map veya switch ile otomatize edebilirsin)
    //        if (tableName == TABLE_NAME_HOME) cat->headerColor = RGB(0, 102, 204);
    //        else if (tableName == TABLE_NAME_LAND) cat->headerColor = RGB(34, 139, 34);
    //        else if (tableName == TABLE_NAME_FIELD) cat->headerColor = RGB(210, 105, 30);
    //        // ... vb ...
    //        else cat->headerColor = RGB(100, 100, 100);

    //        // Sütunları TreeListView'e Ekle
    //        for (const auto& colInfo : colList)
    //        {
    //            AddColumn(cat, colInfo.key, colInfo.title, colInfo.width, colInfo.format,
    //                colInfo.editType != Edit_None, colInfo.editType);
    //        }
    //    }
    //}

























    // CTreeListVDlg.cpp içine yapıştırın (Eski OnEditCommand yerine)

// CTreeListVDlg.cpp

    void OnEditCommand(HTREEITEM hItem)
    {
        // 1. Seçim Kontrolü
        if (!hItem || GetItemData(hItem) == (DWORD_PTR)-1) // Header kontrolü
        {
            MessageBox(L"Lütfen düzenlemek istediğiniz kaydı seçin.", L"Uyarı", MB_ICONWARNING);
            return;
        }

        // 2. Tablo ve Kodu Çözümle
        CString table, codeField, code;
        if (!ResolveTableAndCode(hItem, table, codeField, code))
        {
            MessageBox(L"Kayıt bilgisi çözümlenemedi.", L"Hata", MB_ICONERROR);
            return;
        }

        // 3. Cari Kodu Bul
        CString cariKod;

        // Müşteri tablosuysa, kodun kendisi Cari Kod'dur.
        if (table.CompareNoCase(TABLE_NAME_CUSTOMER) == 0)
        {
            cariKod = code;
        }
        else
        {
            // Mülk ise, Cari Kod'u bulmamız lazım.
            // Yöntem 1: Modelden (row->cells içinde Cari_Kod var mı?)
            TLV_Row* row = GetRow(hItem);
            TLV_Category* cat = GetCategoryForRow(hItem);

            if (row && cat)
            {
                for (size_t i = 0; i < cat->columns.size(); ++i) {
                    if (cat->columns[i].key.CompareNoCase(_T("Cari_Kod")) == 0) {
                        if (i < row->cells.size()) cariKod = row->cells[i].text;
                        break;
                    }
                }
            }

            // Yöntem 2: Hiyerarşiden (Parent -> Customer Root)
            if (cariKod.IsEmpty())
            {
                HTREEITEM hParent = GetParentItem(hItem);
                HTREEITEM hRoot = GetParentItem(hParent);
                // Root'un verisine bak (InsertDataRow ile eklendiği için row->key doludur)
                TLV_Row* rootRow = GetRow(hRoot);
                if (rootRow) cariKod = rootRow->key;
            }
        }

        if (cariKod.IsEmpty())
        {
            MessageBox(L"Müşteri kodu bulunamadı.", L"Hata", MB_ICONERROR);
            return;
        }

        // 4. Diyaloğu Aç
        DatabaseManager& db = DatabaseManager::GetInstance();
        std::unique_ptr<CDialog> pDlg = nullptr;

        if (table.CompareNoCase(TABLE_NAME_HOME) == 0)
            pDlg = std::make_unique<CHomeDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_LAND) == 0)
            pDlg = std::make_unique<CLandDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_FIELD) == 0)
            pDlg = std::make_unique<CFieldDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_VINEYARD) == 0)
            pDlg = std::make_unique<CVineyardDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_VILLA) == 0)
            pDlg = std::make_unique<CVillaDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_COMMERCIAL) == 0)
            pDlg = std::make_unique<CCommercialDialog>(db, IUPDATEUSER, cariKod, code);
        // ✅ BU BLOĞU EKLE
        else if (table.CompareNoCase(TABLE_NAME_CAR) == 0)
            pDlg = std::make_unique<CCarDialog>(db, IUPDATEUSER, cariKod, code);
        else if (table.CompareNoCase(TABLE_NAME_CUSTOMER) == 0)
            pDlg = std::make_unique<CCustomerDialog>(db, IUPDATEUSER, cariKod); // Varsayım

        if (pDlg)
        {
            if (pDlg->DoModal(*this) == IDOK)
            {
                // Yenile
                LoadPortfolioByCariKod(cariKod);
            }
        }
    }    // CTreeListVDlg.cpp içine ekleyin:

    //const CString* FindHeaderCategory(HTREEITEM hHeader) const
    //{
    //    if (!hHeader) return nullptr;

    //    // Temel sınıfın (CTreeListView) yeteneklerini kullanıyoruz.
    //    // Bu fonksiyon const olduğu için, non-const temel fonksiyonu çağırmak için cast yapıyoruz.
    //    auto* pThis = const_cast<CMyTreeListView*>(this);

    //    // Base class'taki haritadan kategoriyi bul
    //    TLV_Category* cat = pThis->FindCategoryByHeader(hHeader);

    //    if (cat)
    //    {
    //        // Kategorinin ismini (Örn: "HomeTbl") döndür
    //        return &cat->name;
    //    }

    //    return nullptr;
    //}

    // CTreeListVDlg.cpp

    const CString* FindHeaderCategory(HTREEITEM hHeader) const
    {
        if (!hHeader) return nullptr;
        auto* pThis = const_cast<CMyTreeListView*>(this);
        TLV_Category* cat = pThis->FindCategoryByHeader(hHeader);
        return cat ? &cat->name : nullptr;
    }
// CTreeListVDlg.cpp
    bool ResolveTableAndCode(HTREEITEM hItem,
        CString& outTable,
        CString& outCodeField,
        CString& outCode) const;

// CTreeListVDlg.cpp

    void OnRowCellDoubleClick(HTREEITEM hItem, int subIndex)
    {
        // Güvenlik: Header (Fake Header) tıklanırsa işlem yapma
        if (GetItemData(hItem) == (DWORD_PTR)-1)
            return;

        // 1. Eğer ilk sütuna (Kod/Ad) tıklandıysa -> Detay Penceresini Aç (Edit Command)
        if (subIndex == 0)
        {
            OnEditCommand(hItem);
            return;
        }

        // 2. Diğer sütunlara tıklandıysa -> Inline Edit Başlat
        // Temel sınıfın fonksiyonunu çağırarak editörü tetikle
        CTreeListView::OnRowCellDoubleClick(hItem, subIndex);
    }








    LRESULT HandleContextMenu(LPARAM lParam)
    {
        // lParam: ekran (screen) koordinatı
        CPoint ptScreen(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        CPoint ptClient = ptScreen;
        ::ScreenToClient(GetHwnd(), &ptClient);

        // Hangi item?
        TVHITTESTINFO hit{};
        hit.pt = ptClient;
        HTREEITEM hItem = TreeView_HitTest(GetHwnd(), &hit);

        // Sağ tarafta boşluğa tıklanmışsa, yine “hileli” testle satırı yakala
        if (!hItem)
        {
            TVHITTESTINFO htFix{};
            htFix.pt = ptClient;
            htFix.pt.x = 5;
            hItem = TreeView_HitTest(GetHwnd(), &htFix);
            hit = htFix;
        }

        if (hItem && (hit.flags & (TVHT_ONITEMLABEL | TVHT_ONITEMICON | TVHT_ONITEMRIGHT)))
        {
            // Seçimi garanti altına al (WM_RBUTTONDOWN’da da yapıyoruz ama sorun değil)
            SelectItem(hItem);
            SetFocus();

            // Popup menü
            int cmd = ShowContextMenu(ptScreen, hItem);
            if (cmd != 0)
            {
                // Tüm komutları tek yerden yöneten fonksiyon
                OnCommand(cmd, 0);
            }
        }

        return 0;
    }

        // CMyTreeListView sınıfının içine:
    //LRESULT HandleContextMenu(LPARAM lParam)
    //{
    //    // lParam ekran koordinatıyla gelir
    //    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    //    ::ScreenToClient(*this, pt);

    //    TVHITTESTINFO hit{};
    //    hit.pt = pt;
    //    HTREEITEM hItem = TreeView_HitTest(GetHwnd(), &hit);

    //    if (hItem && (hit.flags & (TVHT_ONITEMLABEL | TVHT_ONITEMICON | TVHT_ONITEMRIGHT)))
    //    {
    //        SelectItem(hItem);
    //        SetFocus();

    //        ::ClientToScreen(*this, pt);

    //        // 🔥 KRİTİK DEĞİŞİKLİK: ShowContextMenu'dan dönen komut ID'sini yakala
    //        int cmd = ShowContextMenu(pt, hItem);

    //        // Eğer geçerli bir komut seçilmişse, onu işle
    //        if (cmd != 0)
    //        {
    //            // Direkt olarak OnCommand'u çağır veya switch-case ile doğrudan işle
    //            // OnCommand çağırmak, komutun merkezi olarak yönetilmesini sağlar:
    //            OnCommand(cmd, 0);
    //        }
    //    }
    //    return 0;
    //}
    virtual void PreCreate(CREATESTRUCT& cs) override 
    {
        cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_FULLROWSELECT;

        // ❗ TreeView kendi drag-drop'unu devre dışı bırak (yoksa kolon taşıma çalışmaz)
        //cs.style &= ~TVS_DISABLEDRAGDROP;

        CTreeView::PreCreate(cs);
    }
// CMyTreeListView sınıfının içine:

    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
            // ---------------------------------------------------------------------
            // 1. SAĞ TUŞA BASILDIĞI AN (Seçim ve Odaklanma) - KRİTİK DÜZELTME
            // ---------------------------------------------------------------------

        //case WM_PAINT:
        //{
        //    // 1. Önce temel sınıf her şeyi (satırları, metinleri) çizsin
        //    LRESULT lr = CTreeListView::WndProc(msg, wParam, lParam);

        //    // 2. Hemen ardından biz butonları üzerine çizelim
        //    HTREEITEM hSel = GetSelection();
        //    if (hSel)
        //    {
        //        CClientDC dc(*this);
        //        CRect rcItem;
        //        if (TreeView_GetItemRect(GetHwnd(), hSel, &rcItem, TRUE))
        //        {
        //            // Satırın tam genişliğini al
        //            CRect rcFull = GetClientRect();
        //            rcItem.right = rcFull.right;

        //            UpdateButtonRects(hSel, rcItem);

        //            // Butonları çiz (Görünür olması için belirgin renkler)
        //            CBrush brEdit(RGB(0, 120, 215));
        //            CBrush brPrint(RGB(34, 139, 34));

        //            dc.FillRect(m_btnArea.rcEdit, brEdit);
        //            dc.FillRect(m_btnArea.rcPrint, brPrint);

        //            dc.SetTextColor(RGB(255, 255, 255));
        //            dc.SetBkMode(TRANSPARENT);
        //            dc.DrawText(L"Edit", -1, m_btnArea.rcEdit, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        //            dc.DrawText(L"Print", -1, m_btnArea.rcPrint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        //        }
        //    }
        //    return lr;
        //}

        case WM_CONTEXTMENU:
        {
            HandleContextMenu(lParam);
            return 0;
        }

        case WM_MEASUREITEM: // Menü boyutunu ayarla
        {
            LPMEASUREITEMSTRUCT lpMIS = (LPMEASUREITEMSTRUCT)lParam;
            if (lpMIS->CtlType == ODT_MENU) {
                m_menuDrawer.OnMeasureItem(lpMIS);
                return TRUE;
            }
            break;
        }
        case WM_DRAWITEM: // Menüyü çiz
        {
            LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
            if (lpDIS->CtlType == ODT_MENU) {
                m_menuDrawer.OnDrawItem(lpDIS);
                return TRUE;
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

            // Butonların üzerinde miyiz?
            int oldHot = m_hotButton;
            if (m_btnArea.rcEdit.PtInRect(pt)) m_hotButton = 1;
            else if (m_btnArea.rcPrint.PtInRect(pt)) m_hotButton = 2;
            else m_hotButton = 0;

            if (oldHot != m_hotButton) Invalidate(); // Görseli güncelle (Hover efekti)
            break;
        }

        case WM_LBUTTONDOWN:
        {
            CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

            // Eğer butonlardan birine tıklandıysa işlemi yap
            if (m_hotButton > 0)
            {
                TriggerAction(m_hotButton, GetSelection());
                return 0; // TreeView'ın seçim değiştirmesini engelle (aksiyon öncelikli)
            }
            break;
        }

        case WM_RBUTTONDOWN:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ::ClientToScreen(*this, &pt);
            return HandleContextMenu(MAKELPARAM(pt.x, pt.y));
        }
        //case WM_RBUTTONDOWN:
        //{
        //    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        //    // Normal hit test
        //    TVHITTESTINFO ht{};
        //    ht.pt = pt;
        //    HTREEITEM hHit = TreeView_HitTest(GetHwnd(), &ht);

        //    // Sağ tarafta null dönebilir → “hileli” hit test
        //    if (!hHit)
        //    {
        //        ht.pt.x = 5;        // en sol kolon
        //        hHit = TreeView_HitTest(GetHwnd(), &ht);
        //    }

        //    if (hHit)
        //    {
        //        SelectItem(hHit);
        //        SetFocus();
        //    }
        //    break;  // Base'e geç, ama sol tık logic’ini etkilemez
        //}

        //case WM_RBUTTONDOWN:
        //{
        //    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        //    // Tıklanan satırı bul
        //    TVHITTESTINFO ht = { 0 };
        //    ht.pt = pt;
        //    HTREEITEM hHit = TreeView_HitTest(GetHwnd(), &ht);

        //    // Eğer boşluğa tıklandıysa (sağ tarafa), Hileli HitTest yap
        //    if (hHit == nullptr)
        //    {
        //        ht.pt.x = 5; // En sol
        //        hHit = TreeView_HitTest(GetHwnd(), &ht);
        //    }

        //    // Satır varsa hemen seç ve odağı al
        //    if (hHit)
        //    {
        //        SelectItem(hHit); // Satırı Mavi Yap
        //        SetFocus();       // ⚡ Çift tıklama sorununu çözen sihirli komut
        //    }

        //    // "break" diyerek Windows'un varsayılan işlemlerini yapmasına izin ver
        //    // (Böylece sağ tuş basılıyken başka şeyler bozulmaz)
        //    break;
        //}

        // ---------------------------------------------------------------------
        // 2. SAĞ TUŞ BIRAKILDIĞINDA (Menüyü Aç)
        // ---------------------------------------------------------------------
        case WM_RBUTTONUP:
        {
            CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

            // Menü için seçili öğeyi kontrol et (Zaten DOWN olayında seçmiştik)
            HTREEITEM hHit = GetSelection();

            // Eğer bir seçim yoksa menüyü açma
            if (!hHit) return 0;

            // --- MENÜYÜ YÜKLE VE GÖSTER ---
            HMENU hMenu = ::LoadMenu(GetApp()->GetResourceHandle(), MAKEINTRESOURCE(IDR_TREELIST_MENU));

            if (hMenu)
            {
                // Popup menüyü al (Resource dosyasındaki ilk alt menü)
                HMENU hSubMenu = ::GetSubMenu(hMenu, 0);

                if (hSubMenu)
                {
                    ::ClientToScreen(GetHwnd(), &pt);

                    // TPM_RETURNCMD: Seçilen ID'yi int olarak döndürür
                    int cmd = ::TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                        pt.x, pt.y, 0, GetHwnd(), NULL);

                    // --- KOMUTLARI İŞLE ---
                    if (cmd == ID_CONTEXT_EDIT_COLUMNS) // "Başlıklar"
                    {
                        OpenColumnManagerForSelected();
                    }
                    else if (cmd == IDM_TREELIST_DELETE) // "Sil"
                    {
                        // Silme kodunu buraya yaz veya fonksiyon çağır
                        // DeleteSelectedRow(); 
                    }
                    else if (cmd == IDM_TREELIST_EDIT) // "Düzenle"
                    {
                        // Düzenleme kodunu buraya yaz
                        // EditSelectedRow();
                    }
                    // ... Diğer case'ler ...
                }
                ::DestroyMenu(hMenu);
            }
            return 0;
        }


        }

        // Diğer mesajlar temel sınıfa
        return CTreeListView::WndProc(msg, wParam, lParam);
    }
};





// --------------------------------------------------------
// Dock/Container sınıfları
// --------------------------------------------------------
class CContainAllRestate : public Win32xx::CDockContainer
{
public:
    // Ctor: iç TreeListView’i oluşturur
    CContainAllRestate();
    virtual ~CContainAllRestate() override = default;

    // Ana TreeList kontrolüne erişim
    CMyTreeListView& GetDockAllRestateTreeListView() { return mView; }


    template <typename TRecord>
    void LoadRecordVectorToCategory(
        CTreeListView* pView,
        TLV_Category* pCategory,
        const CString& tableName,
        const std::vector<PropertyColumnInfo>& cols,
        const std::vector<TRecord>& records,
        int imageIndex);


    // ✅ YENİ: Layout Hesaplama ve Kurulum
    void RecalcLayout();
    void OnAttach();

    // ✅ YENİ: Komutları MainFrame'e iletmek için
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;

    // ✅ YENİ: Toolbar Elemanları
    CToolBar m_ToolBar;
    CImageList m_ToolBarImages;
    void SetupToolBar();

protected:
    // Dock container mesaj işleme
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    CMyTreeListView mView;

    CContainAllRestate(const CContainAllRestate&) = delete;
    CContainAllRestate& operator=(const CContainAllRestate&) = delete;
};

class CDockAllRestate : public Win32xx::CDocker
{
public:
    // Ctor: container’ı bağlar
    CDockAllRestate();
    virtual ~CDockAllRestate() override = default;

    // İç container’a erişim
    CContainAllRestate& GetCDockFiles() { return m_contain; }

protected:
    // Docker mesaj işleme
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    CContainAllRestate m_contain;

    CDockAllRestate(const CDockAllRestate&) = delete;
    CDockAllRestate& operator=(const CDockAllRestate&) = delete;
};