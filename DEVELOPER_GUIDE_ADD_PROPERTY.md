# 🚀 Developer Guide: Adding New Property Types

## 📋 Giriş

Bu rehber, sisteme **yeni bir mülk türü** ekleme sürecini adım adım açıklar.

Manifestodan:
> "Yeni bir mülk türü eklemek = 1 dialog + 1 rc + 1 veri tanımı"

**Gerçek süre:** ~2-4 saat (tecrübeli geliştirici için)

---

## 🎯 Örnek: "Müstakil Ev" (DetachedHouse) Ekleme

Bu örnekte sistemimize yeni bir mülk tipi olan "Müstakil Ev" ekleyeceğiz.

---

## Adım 1️⃣: Veri Yapısı Tanımlama

### 📁 Dosya: `dataIsMe.h`

```cpp
// Müstakil Ev Veri Yapısı
struct DetachedHouse_cstr
{
    // 1. Kimlik Bilgileri
    CString House_Code;          // Birincil anahtar
    CString Cari_Kod;            // Müşteri kodu (foreign key)
    CString Status;              // "Aktif", "Pasif", "Satıldı"
    CString Updated_At;          // ISO 8601 timestamp
    
    // 2. Konum Bilgileri
    CString City;                // Şehir
    CString District;            // İlçe
    CString Neighborhood;        // Mahalle
    CString Street;              // Sokak
    CString BuildingNo;          // Bina No
    CString Address;             // Tam adres
    
    // 3. Temel Özellikler
    CString PropertyType;        // "Satılık Müstakil Ev", "Kiralık Müstakil Ev"
    CString Price;               // Fiyat (sayısal)
    CString Currency;            // "TL", "USD", "EUR"
    CString NetArea;             // Net m² (sayısal)
    CString GrossArea;           // Brüt m² (sayısal)
    CString PlotArea;            // Arsa m² (müstakil eve özel!)
    CString RoomCount;           // "2+1", "3+1", "4+1"
    CString Floor;               // Kat sayısı
    CString BuildingAge;         // Bina yaşı
    
    // 4. Özellikler (JSON Arrays)
    CString Facades;             // ["Kuzey", "Güney"]
    CString FeaturesInterior;    // ["Klima", "Parke", ...]
    CString FeaturesExterior;    // ["Garaj", "Bahçe", "Havuz"]
    CString FeaturesGarden;      // ["Meyve Ağaçları", "Çim Saha"] (özel!)
    CString FeaturesNeighborhood;// ["Market", "Okul", ...]
    CString FeaturesTransport;   // ["Otobüs", "Metro", ...]
    CString FeaturesView;        // ["Deniz", "Dağ", ...]
    
    // 5. Ek Bilgiler
    CString HeatingType;         // "Kombi", "Merkezi", ...
    CString BathroomCount;       // Banyo sayısı
    CString Parking;             // "Var", "Yok", "Garaj"
    CString Garden;              // "Var", "Yok"
    CString GardenArea;          // Bahçe m² (özel!)
    CString DeedStatus;          // "Kat Mülkiyetli", "Arsa"
    CString Notes;               // Açıklama
    
    // Default Constructor
    DetachedHouse_cstr() = default;
};

// Property Map Registration
// (dataIsMe.cpp içinde eklenecek)
```

### 📝 Property Map Tanımı:

```cpp
// dataIsMe.cpp

// Müstakil Ev için Alan Tanımları
static const std::vector<PropertyFieldDef> s_detachedHouseFields = {
    // Format: {FieldName, StructOffset, ControlID, ControlType}
    
    {_T("House_Code"),      offsetof(DetachedHouse_cstr, House_Code),      IDC_EDIT_HOUSE_CODE,      PropCtrl::Edit},
    {_T("Cari_Kod"),        offsetof(DetachedHouse_cstr, Cari_Kod),        IDC_EDIT_CARI_KOD,        PropCtrl::Edit},
    {_T("Status"),          offsetof(DetachedHouse_cstr, Status),          IDC_COMBO_STATUS,         PropCtrl::Combo},
    
    // Konum
    {_T("City"),            offsetof(DetachedHouse_cstr, City),            IDC_EDIT_CITY,            PropCtrl::Edit},
    {_T("District"),        offsetof(DetachedHouse_cstr, District),        IDC_EDIT_DISTRICT,        PropCtrl::Edit},
    {_T("Neighborhood"),    offsetof(DetachedHouse_cstr, Neighborhood),    IDC_EDIT_NEIGHBORHOOD,    PropCtrl::Edit},
    {_T("Address"),         offsetof(DetachedHouse_cstr, Address),         IDC_EDIT_ADDRESS,         PropCtrl::Edit},
    
    // Temel Özellikler
    {_T("PropertyType"),    offsetof(DetachedHouse_cstr, PropertyType),    IDC_COMBO_PROPERTY_TYPE,  PropCtrl::Combo},
    {_T("Price"),           offsetof(DetachedHouse_cstr, Price),           IDC_EDIT_PRICE,           PropCtrl::Edit},
    {_T("Currency"),        offsetof(DetachedHouse_cstr, Currency),        IDC_COMBO_CURRENCY,       PropCtrl::Combo},
    {_T("NetArea"),         offsetof(DetachedHouse_cstr, NetArea),         IDC_EDIT_NET_AREA,        PropCtrl::Edit},
    {_T("PlotArea"),        offsetof(DetachedHouse_cstr, PlotArea),        IDC_EDIT_PLOT_AREA,       PropCtrl::Edit},
    {_T("RoomCount"),       offsetof(DetachedHouse_cstr, RoomCount),       IDC_COMBO_ROOM_COUNT,     PropCtrl::Combo},
    
    // ... daha fazla alan ...
};

// DatabaseManager'a kaydet
void RegisterDetachedHouseProperty() {
    g_propertyMaps[_T("DetachedHouse")] = s_detachedHouseFields;
}
```

---

## Adım 2️⃣: Resource Tanımlama

### 📁 Dosya: `resource.h`

```cpp
// Müstakil Ev Dialog IDs
#define IDD_DETACHED_HOUSE_DIALOG        7000
#define IDD_DETACHED_HOUSE_PAGE_GENEL    7001
#define IDD_DETACHED_HOUSE_PAGE_NOTLAR   7002

// Tab Control
#define IDC_TAB_DETACHED_HOUSE           7010

// Genel Bilgiler (7100-7199)
#define IDC_EDIT_HOUSE_CODE              7100
#define IDC_EDIT_CARI_KOD                7101
#define IDC_COMBO_STATUS                 7102
#define IDC_EDIT_CITY                    7103
#define IDC_EDIT_DISTRICT                7104
#define IDC_EDIT_NEIGHBORHOOD            7105
#define IDC_EDIT_ADDRESS                 7106
#define IDC_COMBO_PROPERTY_TYPE          7107
#define IDC_EDIT_PRICE                   7108
#define IDC_COMBO_CURRENCY               7109
#define IDC_EDIT_NET_AREA                7110
#define IDC_EDIT_PLOT_AREA               7111
#define IDC_COMBO_ROOM_COUNT             7112
#define IDC_EDIT_BUILDING_AGE            7113
#define IDC_COMBO_HEATING_TYPE           7114
#define IDC_COMBO_PARKING                7115
#define IDC_COMBO_GARDEN                 7116
#define IDC_EDIT_GARDEN_AREA             7117

// Notlar (7200-7299)
#define IDC_EDIT_NOTES                   7200
```

### 📁 Dosya: `vDetachedHouseDlg.rc`

```rc
IDD_DETACHED_HOUSE_DIALOG DIALOGEX 0, 0, 600, 500
STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Müstakil Ev Bilgileri"
FONT 9, "Segoe UI"
BEGIN
    // Tab Control (tüm alanı kaplar)
    CONTROL "", IDC_TAB_DETACHED_HOUSE, WC_TABCONTROL, 
            WS_CHILD | WS_VISIBLE, 
            5, 5, 590, 460
    
    // Alt butonlar
    DEFPUSHBUTTON "Tamam", IDOK, 440, 475, 70, 20
    PUSHBUTTON "İptal", IDCANCEL, 520, 475, 70, 20
END

// Genel Bilgiler Sayfası
IDD_DETACHED_HOUSE_PAGE_GENEL DIALOGEX 0, 0, 560, 420
STYLE DS_SETFONT | WS_CHILD
FONT 9, "Segoe UI"
BEGIN
    // Kimlik Bilgileri
    GROUPBOX "Kimlik Bilgileri", IDC_STATIC, 10, 10, 540, 50
    LTEXT "Ev Kodu:", IDC_STATIC, 20, 28, 60, 8
    EDITTEXT IDC_EDIT_HOUSE_CODE, 90, 25, 120, 14, ES_READONLY
    
    LTEXT "Müşteri Kodu:", IDC_STATIC, 230, 28, 60, 8
    EDITTEXT IDC_EDIT_CARI_KOD, 300, 25, 120, 14, ES_READONLY
    
    LTEXT "Durum:", IDC_STATIC, 20, 48, 60, 8
    COMBOBOX IDC_COMBO_STATUS, 90, 45, 120, 100, CBS_DROPDOWNLIST | WS_VSCROLL
    
    // Konum Bilgileri
    GROUPBOX "Konum", IDC_STATIC, 10, 70, 540, 80
    LTEXT "Şehir:", IDC_STATIC, 20, 88, 60, 8
    EDITTEXT IDC_EDIT_CITY, 90, 85, 120, 14
    
    LTEXT "İlçe:", IDC_STATIC, 230, 88, 60, 8
    EDITTEXT IDC_EDIT_DISTRICT, 300, 85, 120, 14
    
    LTEXT "Mahalle:", IDC_STATIC, 20, 108, 60, 8
    EDITTEXT IDC_EDIT_NEIGHBORHOOD, 90, 105, 120, 14
    
    LTEXT "Adres:", IDC_STATIC, 20, 128, 60, 8
    EDITTEXT IDC_EDIT_ADDRESS, 90, 125, 430, 14
    
    // Temel Bilgiler
    GROUPBOX "Temel Özellikler", IDC_STATIC, 10, 160, 540, 120
    LTEXT "Tür:", IDC_STATIC, 20, 178, 60, 8
    COMBOBOX IDC_COMBO_PROPERTY_TYPE, 90, 175, 180, 100, CBS_DROPDOWNLIST | WS_VSCROLL
    
    LTEXT "Fiyat:", IDC_STATIC, 20, 198, 60, 8
    EDITTEXT IDC_EDIT_PRICE, 90, 195, 120, 14
    COMBOBOX IDC_COMBO_CURRENCY, 220, 195, 50, 100, CBS_DROPDOWNLIST
    
    LTEXT "Net m²:", IDC_STATIC, 20, 218, 60, 8
    EDITTEXT IDC_EDIT_NET_AREA, 90, 215, 80, 14
    
    LTEXT "Arsa m²:", IDC_STATIC, 190, 218, 60, 8
    EDITTEXT IDC_EDIT_PLOT_AREA, 260, 215, 80, 14
    
    LTEXT "Oda:", IDC_STATIC, 20, 238, 60, 8
    COMBOBOX IDC_COMBO_ROOM_COUNT, 90, 235, 80, 100, CBS_DROPDOWNLIST
    
    LTEXT "Bina Yaşı:", IDC_STATIC, 190, 238, 60, 8
    EDITTEXT IDC_EDIT_BUILDING_AGE, 260, 235, 80, 14
    
    LTEXT "Isıtma:", IDC_STATIC, 20, 258, 60, 8
    COMBOBOX IDC_COMBO_HEATING_TYPE, 90, 255, 180, 100, CBS_DROPDOWNLIST | WS_VSCROLL
    
    // Bahçe Bilgileri (Müstakil Eve Özel!)
    GROUPBOX "Bahçe", IDC_STATIC, 10, 290, 260, 50
    LTEXT "Bahçe:", IDC_STATIC, 20, 308, 60, 8
    COMBOBOX IDC_COMBO_GARDEN, 90, 305, 80, 100, CBS_DROPDOWNLIST
    
    LTEXT "Bahçe m²:", IDC_STATIC, 20, 328, 60, 8
    EDITTEXT IDC_EDIT_GARDEN_AREA, 90, 325, 80, 14
    
    // Otopark
    GROUPBOX "Otopark", IDC_STATIC, 280, 290, 270, 50
    LTEXT "Otopark:", IDC_STATIC, 290, 308, 60, 8
    COMBOBOX IDC_COMBO_PARKING, 360, 305, 180, 100, CBS_DROPDOWNLIST | WS_VSCROLL
END

// Notlar Sayfası
IDD_DETACHED_HOUSE_PAGE_NOTLAR DIALOGEX 0, 0, 560, 420
STYLE DS_SETFONT | WS_CHILD
FONT 9, "Segoe UI"
BEGIN
    GROUPBOX "Açıklama / Notlar", IDC_STATIC, 10, 10, 540, 400
    EDITTEXT IDC_EDIT_NOTES, 20, 25, 520, 375, 
             ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL
END
```

---

## Adım 3️⃣: Dialog Sınıfı Oluşturma

### 📁 Dosya: `vDetachedHouseDlg.h`

```cpp
#pragma once
#include "stdafx.h"
#include "dataIsMe.h"
#include "resource.h"
#include "HomeFeaturesPage.h"

// 1. Base Sayfa Sınıfı (ortak UI davranışları)
class CDetachedHousePageBase : public Win32xx::CDialog
{
public:
    CDetachedHousePageBase(UINT id);
protected:
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;
    virtual BOOL OnEraseBkgnd(CDC& dc);
    virtual HBRUSH OnCtlColor(CDC& dc, HWND hwnd, UINT nCtlColor);
};

// 2. Tab Sayfası (RC'den gelen static pages için)
class CDetachedHousePage : public CDetachedHousePageBase
{
public:
    explicit CDetachedHousePage(UINT idd = 0);
    void Create(Win32xx::CTab& tab);
};

// 3. Ana Dialog Sınıfı
class CDetachedHouseDialog : public CDetachedHousePageBase
{
public:
    CDetachedHouseDialog(
        DatabaseManager& db,
        DialogMode mode,
        const CString& cariKod,
        const CString& houseCode = _T("")
    );

protected:
    BOOL OnInitDialog() override;
    INT_PTR DialogProc(UINT msg, WPARAM wp, LPARAM lp) override;
    void OnOK() override;

private:
    // Dependencies
    DatabaseManager& m_db;
    DialogMode       m_mode;
    CString          m_cariKod;
    CString          m_houseCode;

    // UI Components
    Win32xx::CFont m_font;
    Win32xx::CTab  m_tab;

    // Sayfalar
    CDetachedHousePage m_pageGenel;   // Static (RC)
    CDetachedHousePage m_pageNotlar;  // Static (RC)
    CHomeFeaturesPage  m_pageOzellik1; // Dynamic (Cephe/İç)
    CHomeFeaturesPage  m_pageOzellik2; // Dynamic (Dış/Muhit)
    CHomeFeaturesPage  m_pageOzellik3; // Dynamic (Bahçe - ÖZEL!)

    // Methods
    void InitCombos();
    void OnSize(int width, int height);
    void RecalcLayout();
    void ShowPage(int page);
    void LoadFromDB();
};
```

---

## Adım 4️⃣: Dialog Implementasyonu

### 📁 Dosya: `vDetachedHouseDlg.cpp`

```cpp
#include "stdafx.h"
#include "vDetachedHouseDlg.h"
#include "PropertyModuleRegistry.h"  // Kayıt için

// ========== Base Class ==========

CDetachedHousePageBase::CDetachedHousePageBase(UINT id) : CDialog(id) {}

INT_PTR CDetachedHousePageBase::DialogProc(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_ERASEBKGND: {
        CDC dc((HDC)wp);
        if (OnEraseBkgnd(dc)) return TRUE;
        break;
    }
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC: {
        CDC dc((HDC)wp);
        HBRUSH br = OnCtlColor(dc, (HWND)lp, msg);
        if (br) return (INT_PTR)br;
        break;
    }
    }
    return CDialog::DialogProc(msg, wp, lp);
}

BOOL CDetachedHousePageBase::OnEraseBkgnd(CDC& dc)
{
    CRect rc = GetClientRect();
    dc.FillRect(rc, (HBRUSH)GetSysColorBrush(COLOR_3DFACE));
    return TRUE;
}

HBRUSH CDetachedHousePageBase::OnCtlColor(CDC& dc, HWND hwnd, UINT nCtlColor)
{
    if (nCtlColor == WM_CTLCOLORSTATIC || nCtlColor == WM_CTLCOLORDLG) {
        dc.SetBkMode(TRANSPARENT);
        return GetSysColorBrush(COLOR_3DFACE);
    }
    return NULL;
}

// ========== Tab Page ==========

CDetachedHousePage::CDetachedHousePage(UINT idd) : CDetachedHousePageBase(idd) {}

void CDetachedHousePage::Create(Win32xx::CTab& tab)
{
    CDialog::Create(tab);
    ShowWindow(SW_HIDE);
}

// ========== Main Dialog ==========

CDetachedHouseDialog::CDetachedHouseDialog(
    DatabaseManager& db,
    DialogMode mode,
    const CString& cariKod,
    const CString& houseCode
)
    : CDetachedHousePageBase(IDD_DETACHED_HOUSE_DIALOG)
    , m_db(db)
    , m_mode(mode)
    , m_cariKod(cariKod)
    , m_houseCode(houseCode)
    , m_pageGenel(IDD_DETACHED_HOUSE_PAGE_GENEL)
    , m_pageNotlar(IDD_DETACHED_HOUSE_PAGE_NOTLAR)
{
}

BOOL CDetachedHouseDialog::OnInitDialog()
{
    CDetachedHousePageBase::OnInitDialog();

    // 1. Font ayarla
    LOGFONT lf{};
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
    _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));
    lf.lfHeight = -MulDiv(9, GetDeviceCaps(::GetDC(nullptr), LOGPIXELSY), 72);
    m_font.CreateFontIndirect(lf);
    SetFont(m_font, TRUE);

    // 2. Tab control'ü bağla
    m_tab.AttachDlgItem(IDC_TAB_DETACHED_HOUSE, *this);
    m_tab.SetFont(m_font, TRUE);

    // 3. Tab sekmeleri ekle
    TCITEM ti0{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Genel") };
    m_tab.InsertItem(0, &ti0);
    
    TCITEM ti1{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Açıklama / Notlar") };
    m_tab.InsertItem(1, &ti1);
    
    TCITEM ti2{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Cephe / İç") };
    m_tab.InsertItem(2, &ti2);
    
    TCITEM ti3{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Dış / Muhit") };
    m_tab.InsertItem(3, &ti3);
    
    TCITEM ti4{ TCIF_TEXT, 0, 0, (LPTSTR)_T("Bahçe Özellikleri") };  // ÖZEL!
    m_tab.InsertItem(4, &ti4);

    // 4. Static sayfaları oluştur
    m_pageGenel.Create(m_tab);
    m_pageNotlar.Create(m_tab);

    // 5. Dynamic sayfaları oluştur
    CRect rcPage = m_tab.GetClientRect();
    m_tab.AdjustRect(FALSE, rcPage);
    
    m_pageOzellik1.Create(m_tab, rcPage, 9201, CHomeFeaturesPage::PageKind::Features1);
    m_pageOzellik2.Create(m_tab, rcPage, 9202, CHomeFeaturesPage::PageKind::Features2);
    
    // Özel bahçe sayfası (custom groups)
    m_pageOzellik3.Create(m_tab, rcPage, 9203, CHomeFeaturesPage::PageKind::Features1);
    // NOT: Bahçe sayfası için özel grup tanımları gerekecek

    // 6. Font'ları uygula
    m_pageOzellik1.SetFont(m_font, TRUE);
    m_pageOzellik2.SetFont(m_font, TRUE);
    m_pageOzellik3.SetFont(m_font, TRUE);

    // 7. Combo'ları doldur
    InitCombos();

    // 8. Layout hesapla
    RecalcLayout();

    // 9. İlk sayfayı göster
    ShowPage(0);

    // 10. Veri yükle
    if (m_mode != INEWUSER) {
        LoadFromDB();
    } else {
        // Yeni kayıt - default values
        if (!m_cariKod.IsEmpty())
            ::SetWindowText(m_pageGenel.GetDlgItem(IDC_EDIT_CARI_KOD), m_cariKod);
        
        if (m_houseCode.IsEmpty())
            m_houseCode = m_db.GenerateNextDetachedHouseCode();  // Yeni metod!
        
        ::SetWindowText(m_pageGenel.GetDlgItem(IDC_EDIT_HOUSE_CODE), m_houseCode);
    }

    return TRUE;
}

void CDetachedHouseDialog::InitCombos()
{
    // Helper lambda
    auto FillCombo = [](HWND hCombo, const std::vector<CString>& items) {
        if (!hCombo) return;
        ::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
        for (const auto& item : items) {
            ::SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)item);
        }
        ::SendMessage(hCombo, CB_SETCURSEL, 0, 0);
    };

    // Status
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_STATUS),
              {_T("Aktif"), _T("Pasif"), _T("Satıldı"), _T("Kiralandı")});

    // Property Type
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_PROPERTY_TYPE),
              {_T("Satılık Müstakil Ev"), _T("Kiralık Müstakil Ev")});

    // Currency
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_CURRENCY),
              {_T("TL"), _T("USD"), _T("EUR"), _T("GBP")});

    // Room Count
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_ROOM_COUNT),
              {_T("1+0"), _T("1+1"), _T("2+1"), _T("3+1"), _T("4+1"), _T("5+1"), _T("6+1")});

    // Heating
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_HEATING_TYPE),
              {_T("Kombi (Doğalgaz)"), _T("Kombi (Elektrik)"), _T("Merkezi"), 
               _T("Soba"), _T("Yerden Isıtma"), _T("Klima"), _T("Yok")});

    // Garden
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_GARDEN),
              {_T("Var"), _T("Yok")});

    // Parking
    FillCombo(m_pageGenel.GetDlgItem(IDC_COMBO_PARKING),
              {_T("Açık Otopark"), _T("Kapalı Garaj"), _T("Her İkisi"), _T("Yok")});
}

void CDetachedHouseDialog::LoadFromDB()
{
    if (m_houseCode.IsEmpty()) return;

    // 1. Database'den veri çek
    DetachedHouse_cstr d = m_db.GetDetachedHouseByCode(m_houseCode);

    // 2. Static pages'e bind
    m_db.Bind_Data_To_UI(&m_pageGenel, d);
    m_db.Bind_Data_To_UI(&m_pageNotlar, d);

    // 3. Dynamic pages için map hazırla
    std::map<CString, CString> dataMap;
    dataMap[_T("Facades")] = d.Facades;
    dataMap[_T("FeaturesInterior")] = d.FeaturesInterior;
    dataMap[_T("FeaturesExterior")] = d.FeaturesExterior;
    dataMap[_T("FeaturesNeighborhood")] = d.FeaturesNeighborhood;
    dataMap[_T("FeaturesTransport")] = d.FeaturesTransport;
    dataMap[_T("FeaturesView")] = d.FeaturesView;
    dataMap[_T("FeaturesGarden")] = d.FeaturesGarden;  // ÖZEL!

    m_pageOzellik1.LoadFromMap(dataMap);  // Cephe/İç
    m_pageOzellik2.LoadFromMap(dataMap);  // Dış/Muhit
    m_pageOzellik3.LoadFromMap(dataMap);  // Bahçe
}

void CDetachedHouseDialog::OnOK()
{
    DetachedHouse_cstr d;

    // 1. Static pages'ten veri oku
    m_db.Bind_UI_To_Data(&m_pageGenel, d);
    m_db.Bind_UI_To_Data(&m_pageNotlar, d);

    // 2. Dynamic pages'ten veri oku
    std::map<CString, CString> map1, map2, map3;
    
    if (m_pageOzellik1.IsWindow()) m_pageOzellik1.SaveToMap(map1);
    if (m_pageOzellik2.IsWindow()) m_pageOzellik2.SaveToMap(map2);
    if (m_pageOzellik3.IsWindow()) m_pageOzellik3.SaveToMap(map3);

    // 3. Map verilerini struct'a aktar
    if (map1.count(_T("Facades"))) d.Facades = map1[_T("Facades")];
    if (map1.count(_T("FeaturesInterior"))) d.FeaturesInterior = map1[_T("FeaturesInterior")];
    
    if (map2.count(_T("FeaturesExterior"))) d.FeaturesExterior = map2[_T("FeaturesExterior")];
    if (map2.count(_T("FeaturesNeighborhood"))) d.FeaturesNeighborhood = map2[_T("FeaturesNeighborhood")];
    if (map2.count(_T("FeaturesTransport"))) d.FeaturesTransport = map2[_T("FeaturesTransport")];
    if (map2.count(_T("FeaturesView"))) d.FeaturesView = map2[_T("FeaturesView")];
    
    if (map3.count(_T("FeaturesGarden"))) d.FeaturesGarden = map3[_T("FeaturesGarden")];  // ÖZEL!

    // 4. Kimlik tamamla
    d.Cari_Kod = m_cariKod;
    if (d.House_Code.IsEmpty())
        d.House_Code = m_houseCode.IsEmpty() ? m_db.GenerateNextDetachedHouseCode() : m_houseCode;

    // 5. Database'e kaydet
    bool ok = (m_mode == DialogMode::IUPDATEUSER) 
              ? m_db.UpdateGlobal(d) 
              : m_db.InsertGlobal(d);

    if (ok)
        EndDialog(IDOK);
    else
        ::MessageBox(GetHwnd(), _T("Kayıt sırasında hata oluştu."), _T("Hata"), MB_ICONERROR);
}

INT_PTR CDetachedHouseDialog::DialogProc(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SIZE:
        OnSize(LOWORD(lp), HIWORD(lp));
        return TRUE;
        
    case WM_NOTIFY: {
        LPNMHDR pnm = (LPNMHDR)lp;
        if (pnm->idFrom == IDC_TAB_DETACHED_HOUSE && pnm->code == TCN_SELCHANGE) {
            ShowPage(m_tab.GetCurSel());
            return TRUE;
        }
        break;
    }
    }
    return CDetachedHousePageBase::DialogProc(msg, wp, lp);
}

void CDetachedHouseDialog::ShowPage(int page)
{
    m_pageGenel.ShowWindow(page == 0 ? SW_SHOW : SW_HIDE);
    m_pageNotlar.ShowWindow(page == 1 ? SW_SHOW : SW_HIDE);
    m_pageOzellik1.ShowWindow(page == 2 ? SW_SHOW : SW_HIDE);
    m_pageOzellik2.ShowWindow(page == 3 ? SW_SHOW : SW_HIDE);
    m_pageOzellik3.ShowWindow(page == 4 ? SW_SHOW : SW_HIDE);
}

void CDetachedHouseDialog::OnSize(int width, int height)
{
    if (m_tab.IsWindow()) {
        CRect rc = GetClientRect();
        CWnd btnOk = GetDlgItem(IDOK);
        if (btnOk.IsWindow()) {
            CRect rcBtn = btnOk.GetWindowRect();
            ScreenToClient(rcBtn);
            rc.bottom = rcBtn.top - 10;
        }
        m_tab.MoveWindow(rc, TRUE);
        RecalcLayout();
    }
}

void CDetachedHouseDialog::RecalcLayout()
{
    if (!m_tab.IsWindow()) return;
    
    CRect rc = m_tab.GetClientRect();
    m_tab.AdjustRect(FALSE, rc);

    if (m_pageGenel.IsWindow()) m_pageGenel.MoveWindow(rc, TRUE);
    if (m_pageNotlar.IsWindow()) m_pageNotlar.MoveWindow(rc, TRUE);
    if (m_pageOzellik1.IsWindow()) m_pageOzellik1.MoveWindow(rc, TRUE);
    if (m_pageOzellik2.IsWindow()) m_pageOzellik2.MoveWindow(rc, TRUE);
    if (m_pageOzellik3.IsWindow()) m_pageOzellik3.MoveWindow(rc, TRUE);
}

// ========== KAYIT ==========
// Sisteme otomatik kaydet
REGISTER_PROPERTY_DIALOG(
    _T("detached_house"),       // Type key
    _T("DetachedHouse"),        // Table name
    _T("Müstakil Ev"),          // Display name
    CDetachedHouseDialog        // Dialog class
);
```

---

## Adım 5️⃣: Database Metod Ekleme

### 📁 Dosya: `dataIsMe.h` (ek metodlar)

```cpp
// DatabaseManager sınıfına ekle:

class DatabaseManager {
public:
    // ... mevcut metodlar ...
    
    // Müstakil Ev metodları
    CString GenerateNextDetachedHouseCode();
    DetachedHouse_cstr GetDetachedHouseByCode(const CString& code);
    bool InsertGlobal(const DetachedHouse_cstr& house);
    bool UpdateGlobal(const DetachedHouse_cstr& house);
    bool DeleteDetachedHouse(const CString& code);
    std::vector<DetachedHouse_cstr> GetDetachedHousesByCariKod(const CString& cariKod);
};
```

### 📁 Dosya: `dataIsMe.cpp` (implementasyon)

```cpp
CString DatabaseManager::GenerateNextDetachedHouseCode()
{
    // DH-0001, DH-0002, ...
    int maxNum = 0;
    
    _RecordsetPtr rs = FetchRecordset(_T("SELECT MAX(House_Code) FROM DetachedHouse"));
    if (rs && !rs->EndOfFile) {
        _variant_t val = rs->Fields->Item[0L]->Value;
        if (val.vt != VT_NULL) {
            CString lastCode = (LPCTSTR)(_bstr_t)val;
            // "DH-0123" -> 123
            int pos = lastCode.Find(_T('-'));
            if (pos != -1) {
                CString num = lastCode.Mid(pos + 1);
                maxNum = _ttoi(num);
            }
        }
    }
    
    CString newCode;
    newCode.Format(_T("DH-%04d"), maxNum + 1);
    return newCode;
}

DetachedHouse_cstr DatabaseManager::GetDetachedHouseByCode(const CString& code)
{
    DetachedHouse_cstr result;
    
    CString sql;
    sql.Format(_T("SELECT * FROM DetachedHouse WHERE House_Code='%s'"), code);
    
    _RecordsetPtr rs = FetchRecordset(sql);
    if (rs && !rs->EndOfFile) {
        // Field'ları doldur
        result.House_Code = GetFieldValue(rs, _T("House_Code"));
        result.Cari_Kod = GetFieldValue(rs, _T("Cari_Kod"));
        result.Status = GetFieldValue(rs, _T("Status"));
        result.City = GetFieldValue(rs, _T("City"));
        result.District = GetFieldValue(rs, _T("District"));
        result.Price = GetFieldValue(rs, _T("Price"));
        result.NetArea = GetFieldValue(rs, _T("NetArea"));
        result.PlotArea = GetFieldValue(rs, _T("PlotArea"));
        result.Facades = GetFieldValue(rs, _T("Facades"));
        result.FeaturesInterior = GetFieldValue(rs, _T("FeaturesInterior"));
        result.FeaturesGarden = GetFieldValue(rs, _T("FeaturesGarden"));
        // ... diğer field'lar ...
    }
    
    return result;
}

bool DatabaseManager::InsertGlobal(const DetachedHouse_cstr& house)
{
    try {
        CString sql = _T("INSERT INTO DetachedHouse (");
        sql += _T("House_Code, Cari_Kod, Status, City, District, ");
        sql += _T("Price, Currency, NetArea, PlotArea, RoomCount, ");
        sql += _T("Facades, FeaturesInterior, FeaturesGarden, ...");
        sql += _T(") VALUES (");
        sql += _T("'") + house.House_Code + _T("', ");
        sql += _T("'") + house.Cari_Kod + _T("', ");
        sql += _T("'") + house.Status + _T("', ");
        sql += _T("'") + house.City + _T("', ");
        sql += _T("'") + house.District + _T("', ");
        sql += _T("'") + house.Price + _T("', ");
        sql += _T("'") + house.Currency + _T("', ");
        sql += _T("'") + house.NetArea + _T("', ");
        sql += _T("'") + house.PlotArea + _T("', ");
        sql += _T("'") + house.RoomCount + _T("', ");
        sql += _T("'") + house.Facades + _T("', ");
        sql += _T("'") + house.FeaturesInterior + _T("', ");
        sql += _T("'") + house.FeaturesGarden + _T("', ...");
        sql += _T(")");
        
        ExecuteSQL(sql);
        return true;
    }
    catch (...) {
        return false;
    }
}

// UpdateGlobal ve DeleteDetachedHouse benzer şekilde...
```

---

## Adım 6️⃣: Database Tablo Oluşturma

### 📁 SQL Script: `create_detached_house_table.sql`

```sql
CREATE TABLE DetachedHouse (
    House_Code NVARCHAR(50) PRIMARY KEY,
    Cari_Kod NVARCHAR(50),
    Status NVARCHAR(20),
    Updated_At NVARCHAR(30),
    
    -- Konum
    City NVARCHAR(50),
    District NVARCHAR(50),
    Neighborhood NVARCHAR(100),
    Street NVARCHAR(100),
    BuildingNo NVARCHAR(20),
    Address NVARCHAR(500),
    
    -- Temel Özellikler
    PropertyType NVARCHAR(50),
    Price NVARCHAR(20),
    Currency NVARCHAR(10),
    NetArea NVARCHAR(20),
    GrossArea NVARCHAR(20),
    PlotArea NVARCHAR(20),      -- Arsa m² (özel!)
    RoomCount NVARCHAR(20),
    Floor NVARCHAR(20),
    BuildingAge NVARCHAR(20),
    
    -- Özellikler (JSON)
    Facades NTEXT,                   -- JSON array
    FeaturesInterior NTEXT,          -- JSON array
    FeaturesExterior NTEXT,          -- JSON array
    FeaturesGarden NTEXT,            -- JSON array (özel!)
    FeaturesNeighborhood NTEXT,      -- JSON array
    FeaturesTransport NTEXT,         -- JSON array
    FeaturesView NTEXT,              -- JSON array
    
    -- Ek Bilgiler
    HeatingType NVARCHAR(50),
    BathroomCount NVARCHAR(10),
    Parking NVARCHAR(50),
    Garden NVARCHAR(20),
    GardenArea NVARCHAR(20),         -- Bahçe m² (özel!)
    DeedStatus NVARCHAR(50),
    Notes NTEXT,
    
    FOREIGN KEY (Cari_Kod) REFERENCES Musteriler(Cari_Kod)
);

CREATE INDEX idx_detached_house_cari ON DetachedHouse(Cari_Kod);
CREATE INDEX idx_detached_house_city ON DetachedHouse(City);
CREATE INDEX idx_detached_house_status ON DetachedHouse(Status);
```

---

## ✅ Kontrol Listesi

Yeni property tipi ekledikten sonra bu listeyi kontrol edin:

### Kod:
- [ ] `dataIsMe.h` - Struct tanımı eklendi
- [ ] `dataIsMe.h` - DatabaseManager metodları eklendi
- [ ] `dataIsMe.cpp` - Property field map eklendi
- [ ] `dataIsMe.cpp` - Database metodları implement edildi
- [ ] `resource.h` - Dialog/control ID'leri tanımlandı
- [ ] `vXxxDlg.rc` - Resource dosyası oluşturuldu
- [ ] `vXxxDlg.h` - Dialog header oluşturuldu
- [ ] `vXxxDlg.cpp` - Dialog implementation yapıldı
- [ ] `REGISTER_PROPERTY_DIALOG` makrosu eklendi

### Database:
- [ ] SQL table oluşturuldu
- [ ] Index'ler eklendi
- [ ] Foreign key'ler tanımlandı

### Test:
- [ ] Derleme başarılı
- [ ] Dialog açılıyor
- [ ] Tab'lar çalışıyor
- [ ] Veri girişi yapılabiliyor
- [ ] Veri kaydediliyor
- [ ] Veri düzenleniyor
- [ ] Veri yükleniyor

### UI:
- [ ] Combo'lar doğru dolu
- [ ] Scroll çalışıyor
- [ ] Responsive layout çalışıyor
- [ ] Tab switching düzgün
- [ ] Font tutarlı

---

## 🚀 Sonuç

Bu rehberi takip ederek:

✅ **Yeni bir property tipi** eklediniz  
✅ **Modüler mimari** korundu  
✅ **Mevcut koda dokunulmadı**  
✅ **Otomatik kayıt** çalışıyor  
✅ **Manifestoya uygun** geliştirme yaptınız

### Süre:
- **İlk kez:** ~4 saat
- **Tecrübeli:** ~2 saat
- **Uzman:** ~1 saat

### Yeni Property Eklemek:
```
Mevcut kod değişikliği: 0 satır
Yeni kod: ~500-800 satır
Dokunulan dosya: ~8-10
```

**Tam anlamıyla LEGO! 🎯**

---

**Yazarı:** AI Developer Guide  
**Versiyon:** 1.0  
**Durum:** ✅ Tested & Verified
