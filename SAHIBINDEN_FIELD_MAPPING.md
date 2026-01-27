# Sahibinden Field Mapping Documentation

This document describes the field mapping between Sahibinden.com JSON/HTML data and the application's data structures.

## Overview

The `SahibindenImporter` class extracts listing data from Sahibinden.com pages in multiple ways:
1. **JSON parsing** from `gaPageViewTrackingJson` data-json attribute (primary method)
2. **HTML scraping** from page structure (fallback method)
3. **Combined approach** using JSON for technical details and HTML for seller/contact information

## Field Extraction Strategy

### Data Sources
- **customVars**: Array in JSON containing Turkish field names
- **dmpData**: Array in JSON containing English/snake_case field names
- **HTML elements**: Backup source when JSON is not available

### Extraction Functions
- `ImportFromJsonAndHtmlString()`: Main entry point combining JSON and HTML
- `ParseTrackingJson()`: Extracts data from gaPageViewTrackingJson
- `ParseHtmlDirectly()`: Fallback HTML scraping
- `ExtractContactFromHtml()`: Extracts seller name and phone
- `ExtractFeaturesFromHtml()`: Extracts property features

## Field Mappings by Category

### 1. Houses (Home_cstr)

All house fields are properly extracted and mapped:

| Field Name | Turkish Keys | English Keys | Notes |
|------------|--------------|--------------|-------|
| ListingNo | İlan No | classifiedId, id | Listing identifier |
| ListingDate | İlan Tarihi | ilan_tarihi | Date format varies |
| PropertyType | Emlak Tipi | cat0 | Property category |
| RoomCount | Oda Sayısı | oda_sayisi | Room count |
| NetArea | m² (Net) | m2_net | Net square meters |
| GrossArea | m² (Brüt) | m2_brut | Gross square meters |
| BuildingAge | Bina Yaşı | bina_yasi | Building age |
| Floor | Bulunduğu Kat | bulundugu_kat | Current floor |
| TotalFloor | Kat Sayısı | kat_sayisi | Total floors |
| HeatingType | Isıtma | isitma | Heating system |
| BathroomCount | Banyo Sayısı | banyo_sayisi | Number of bathrooms |
| KitchenType | Mutfak | mutfak | Kitchen type |
| Balcony | Balkon | balkon | Balcony info |
| Elevator | Asansör | asansor | Elevator availability |
| Parking | Otopark | otopark | Parking availability |
| Furnished | Eşyalı | esyali | Furnished status |
| UsageStatus | Kullanım Durumu | kullanim_durumu | Usage status |
| InSite | Site İçerisinde | site_icerisinde | In complex |
| SiteName | Site Adı | site_adi | Complex name |
| Dues | Aidat (TL) | aidat_tl | Monthly dues |
| CreditEligible | Krediye Uygun | krediye_uygun | Loan eligible |
| DeedStatus | Tapu Durumu | tapu_durumu | Title deed status |
| SellerType | Kimden | kimden | From whom |
| Swap | Takas | - | Exchange/trade |

### 2. Villas (Villa_cstr)

**FIXED**: All villa fields now properly extracted (previously only 4 fields were populated).

| Field Name | Turkish Keys | English Keys | Notes |
|------------|--------------|--------------|-------|
| **AcikAlanM2** | **Açık Alan m²**, Açık Alan, Acik Alan m2 | **acik_alan_m2**, outdoor_area | **CRITICAL: Outdoor area in m²** |
| OdaSayisi | Oda Sayısı | oda_sayisi | Room count |
| NetMetrekare | m² (Net) | m2_net | Net square meters |
| BrutMetrekare | m² (Brüt) | m2_brut | Gross square meters |
| BuildingAge | Bina Yaşı | bina_yasi | Building age |
| HeatingType | Isıtma | isitma | Heating system |
| BathroomCount | Banyo Sayısı | banyo_sayisi | Number of bathrooms |
| KitchenType | Mutfak | mutfak | Kitchen type |
| Parking | Otopark | otopark | Parking availability |
| Furnished | Eşyalı | esyali | Furnished status |
| UsageStatus | Kullanım Durumu | kullanim_durumu | Usage status |
| InSite | Site İçerisinde | site_icerisinde | In complex |
| SiteName | Site Adı | site_adi | Complex name |
| Dues | Aidat (TL) | aidat_tl | Monthly dues |
| CreditEligible | Krediye Uygun | krediye_uygun | Loan eligible |
| DeedStatus | Tapu Durumu | tapu_durumu | Title deed status |
| SellerType | Kimden | kimden | From whom |
| Swap | Takas | - | Exchange/trade |

### 3. Land Plots (Land_cstr)

All land fields were already properly extracted:

| Field Name | Turkish Keys | English Keys | Notes |
|------------|--------------|--------------|-------|
| LAND_ImarDurumu | İmar Durumu, Imar Durumu, ZoningStatus | imar_durumu, zoning_status | Zoning status |
| LAND_M2 | m², m2, Metrekare, Tapu Alanı (m²) | m2, m2_brut, m2_net, area | Land area |
| LAND_M2Fiyati | m² Fiyatı, m2 Fiyatı, PricePerM2 | m2_fiyati, price_per_m2 | Price per m² |
| LAND_AdaNo | Ada No, Ada, Ada Numarası | ada_no | Island number |
| LAND_ParselNo | Parsel No, Parsel, Parsel Numarası | parsel_no | Parcel number |
| LAND_PaftaNo | Pafta No, Pafta | pafta_no | Map sheet number |
| LAND_KaksEmsal | Kaks (Emsal), KAKS, Emsal, Kaks | kaks_emsal, kaks, emsal | Floor area ratio |
| LAND_Gabari | Gabari | gabari | Building height limit |
| LAND_KrediUygunluk | Krediye Uygunluk, Krediye Uygun | krediye_uygunluk, krediye_uygun | Loan eligible |
| LAND_TapuDurumu | Tapu Durumu | tapu_durumu | Title deed status |
| LAND_Kimden | Kimden | kimden | From whom |
| LAND_Takas | Takas | takas | Exchange/trade |

### 4. Cars/Vehicles (Car_cstr)

**NEW**: Complete vehicle/car data extraction support added.

| Field Name | Turkish Keys | English Keys | Notes |
|------------|--------------|--------------|-------|
| **Brand** | **Marka** | **marka**, brand | **Vehicle brand** |
| **Model** | **Model** | **model** | **Vehicle model** |
| **Km** | **KM**, Kilometre | **km**, kilometre | **Mileage** |
| **FuelType** | **Yakıt Tipi**, Yakıt | **yakit_tipi**, yakit, fuel_type | **Fuel type** |
| **Transmission** | **Vites Tipi**, Vites | **vites_tipi**, vites, transmission | **Transmission type** |
| **Drive** | **Çekiş** | **cekis**, drive | **Drive type (FWD/RWD/AWD)** |
| **Color** | **Renk** | **renk**, color | **Vehicle color** |
| Series | Seri | seri, series | Vehicle series |
| Year | Yıl, Model Yılı | yil, year, model_yili | Model year |
| EngineVolume | Motor Hacmi | motor_hacmi, engine_volume | Engine displacement |
| EnginePower | Motor Gücü | motor_gucu, engine_power | Engine horsepower |
| BodyType | Kasa Tipi | kasa_tipi, body_type | Body type |
| DamageRecord | Hasar Kaydı, Hasarlı | hasar_kaydi, damage_record | Accident history |
| Warranty | Garanti, Garanti Durumu | garanti, warranty | Warranty status |
| Plate | Plaka | plaka, plate | License plate |
| VehicleCondition | Araç Durumu, Vasıta Durumu | arac_durumu, vehicle_condition, vasita_durumu | Vehicle condition |

### 5. Common Fields (All Types)

| Field Name | Turkish Keys | English Keys | Notes |
|------------|--------------|--------------|-------|
| priceText | ilan_fiyat, Fiyat | fiyat | Price with currency |
| city | loc2 | loc2 | City/province |
| district | loc3 | loc3 | District |
| neighborhood | loc5 | loc5 | Neighborhood |
| contactName | - | - | Seller name (from HTML) |
| contactPhone | - | - | Seller phone (from HTML) |
| featuresText | - | - | Selected features (from HTML) |

## Vehicle Detection Logic

The importer detects vehicle listings using multiple criteria:

```cpp
bool isCar = 
    url.contains("/vasita/") ||      // Vehicle category in URL
    url.contains("/otomobil/") ||    // Car category in URL
    url.contains("/araba/") ||       // Car keyword in URL
    (!Brand.empty() && !Model.empty()) || // Has brand and model
    (!Km.empty());                   // Has mileage data
```

## Robust Field Extraction

The `PickFirstW()` helper function provides fallback mechanisms:
- Tries multiple Turkish key variations (e.g., "m² (Brüt)", "m2 (Brüt)")
- Falls back to English/snake_case keys (e.g., "m2_brut")
- Returns first non-empty value found
- Handles Turkish character encoding properly via wide strings

## HTML Fallback Parsing

When JSON is unavailable, the importer can extract fields from HTML:
- Regex-based extraction from `<li><strong>Label</strong> <span>Value</span>` patterns
- Location parsing from breadcrumb links
- Price extraction from `classifiedInfo` sections
- Feature extraction from `class="selected"` list items

## Testing Recommendations

Since no automated test infrastructure exists, manual testing should cover:

1. **House listings**: Verify all 20+ fields are extracted
2. **Villa listings**: Verify AcikAlanM2 and all villa-specific fields
3. **Land listings**: Verify all cadastral fields (Ada, Parsel, Pafta, etc.)
4. **Car listings**: Verify Marka, Model, KM, Yakıt, Vites, Çekiş, Renk
5. **Mixed categories**: Test detection logic correctly routes to right table
6. **JSON-only mode**: Test with direct JSON input
7. **HTML fallback**: Test when JSON is missing or malformed
8. **Turkish characters**: Verify proper encoding/decoding

## Future Improvements

1. Add automated unit tests for field extraction
2. Add integration tests with sample HTML/JSON fixtures
3. Add validation for extracted values (e.g., numeric ranges)
4. Add logging for missing/unexpected fields
5. Add field mapping configuration file for easier maintenance
6. Consider supporting other real estate platforms (Hepsiemlak, etc.)

## References

- `SahibindenImporter.h` - Payload structure definitions
- `SahibindenImporter.cpp` - Extraction and mapping logic
- `dataIsMe.h` - Database structure definitions
- `SahibindenBulkWindow.cpp` - Bulk import automation
