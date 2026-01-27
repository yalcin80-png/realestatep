# SahibindenImporter: Car and Villa Support Testing Guide

## Overview
This document describes the changes made to support Car (Vasıta) and Villa property types in the SahibindenImporter, and provides testing scenarios.

## Changes Summary

### 1. Extended SahibindenListingPayload Structure
Added 21 new fields to support Car and Villa properties:

#### Car Fields (17 fields)
- `CAR_Brand` - Marka (e.g., "Volkswagen")
- `CAR_Series` - Seri (e.g., "Bora")
- `CAR_Model` - Model (e.g., "1.6 Comfortline")
- `CAR_Year` - Yıl (e.g., "2002")
- `CAR_Km` - KM (e.g., "280.000")
- `CAR_FuelType` - Yakıt Tipi (e.g., "Benzin & LPG")
- `CAR_Transmission` - Vites (e.g., "Manuel")
- `CAR_BodyType` - Kasa Tipi (e.g., "Sedan")
- `CAR_EnginePower` - Motor Gücü (e.g., "105 hp")
- `CAR_EngineVolume` - Motor Hacmi (e.g., "1598 cc")
- `CAR_Drive` - Çekiş (e.g., "Önden Çekiş")
- `CAR_Color` - Renk (e.g., "Gümüş Gri")
- `CAR_Warranty` - Garanti (e.g., "Evet/Hayır")
- `CAR_DamageRecord` - Ağır Hasar Kayıtlı (e.g., "Evet/Hayır")
- `CAR_Plate` - Plaka/Uyruk (e.g., "Türkiye (TR) Plakalı")
- `CAR_FromWho` - Kimden (e.g., "Galeriden")
- `CAR_Cat2` - Category detection helper

#### Villa Fields (4 fields)
- `VILLA_OpenArea` - Açık Alan m² (e.g., "350")
- `VILLA_TotalFloors` - Kat Sayısı (e.g., "2")
- `VILLA_Balcony` - Balkon (e.g., "Var")
- `VILLA_Elevator` - Asansör (e.g., "Var")

### 2. JSON Parsing Enhancement
Enhanced both `ImportFromJsonAndHtmlString()` and `ParseTrackingJson()` to extract:
- Car fields from customVars/dmpData with Turkish and English key variants
- Villa-specific fields with multiple key variations
- Proper category detection using cat2 field

**Key Mappings Added:**
```cpp
// Car Examples
CAR_Brand = PickFirstW(customVars, {L"Marka", L"Brand"}, dmpData, {L"marka", L"brand"});
CAR_FuelType = PickFirstW(customVars, {L"Yakıt Tipi", L"Yakıt"}, dmpData, {L"yakit_tipi"});

// Villa Examples  
VILLA_OpenArea = PickFirstW(customVars, {L"Açık Alan m²", L"Acik Alan m2"}, dmpData, {L"acik_alan"});
```

### 3. HTML Fallback Support
Enhanced `ParseHtmlDirectly()` to parse Car and Villa fields from HTML when JSON is unavailable:

**Car Patterns Added:**
- Marka, Seri, Model, Yıl/Yil, KM/Km
- Yakıt, Vites, Kasa Tipi, Motor Gücü/Gucu, Motor Hacmi
- Çekiş/Cekis, Renk, Garanti, Ağır Hasar/Agir Hasar, Plaka

**Villa Patterns Added:**
- Açık Alan/Acik Alan, Asansör/Asansor

### 4. Car Type Detection and Database Insertion
Implemented complete Car support in `SaveToDatabase()`:

**Detection Logic:**
```cpp
auto isCar = [&]() -> bool {
    CString cat2 = p.CAR_Cat2; cat2.MakeLower();
    return isType(_T("vasıta")) || isType(_T("vasita")) || 
           isType(_T("otomobil")) || 
           (cat2.Find(_T("otomobil")) != -1) || 
           (urlLower.Find(_T("vasita")) != -1);
};
```

**Database Mapping:**
- Creates `Car_cstr` structure
- Maps all 17 car-specific fields
- Generates car title from Brand + Series + Model
- Includes location (City, District, Neighborhood)
- Stores seller info, features, and metadata

### 5. Enhanced Villa Database Insertion
Improved Villa support with complete field mapping:

**New Villa Mappings:**
- All villa-specific fields now properly populated
- Added: AcikAlanM2, KatSayisi, BathroomCount, HeatingType
- Added: KitchenType, Parking, Furnished, UsageStatus
- Added: InSite, SiteName, Dues, CreditEligible
- Added: DeedStatus, SellerType, Swap, BuildingAge
- Fallback logic for Balcony and Elevator (Villa-specific → generic)

## Testing Scenarios

### Test 1: Car Import (Vasıta/Otomobil)
**Test URL Pattern:** `https://www.sahibinden.com/ilan/vasita-otomobil-*/detay-*`

**Expected Data:**
```
Marka: Volkswagen
Seri: Bora
Model: 1.6 Comfortline
Yıl: 2002
KM: 280.000
Yakıt Tipi: Benzin & LPG
Vites: Manuel
Kasa Tipi: Sedan
Motor Gücü: 105 hp
Motor Hacmi: 1598 cc
Çekiş: Önden Çekiş
Renk: Gümüş Gri
```

**Verification Steps:**
1. Import a car listing using the importer
2. Verify `isCar()` returns true
3. Check database for new Car_cstr record
4. Verify all car-specific fields are populated
5. Confirm Title is generated as "Volkswagen Bora 1.6 Comfortline"

### Test 2: Villa Import (Satılık Villa)
**Test URL Pattern:** `https://www.sahibinden.com/ilan/emlak-konut-satilik-villa/detay-*`

**Expected Data:**
```
Açık Alan: 350 m²
Kat Sayısı: 2
Banyo Sayısı: 5
Isıtma: Kombi (Doğalgaz)
Mutfak: Kapalı
Balkon: Var
Asansör: Yok
Otopark: Açık Otopark
Eşyalı: Hayır
Kullanım Durumu: Boş
Site İçerisinde: Evet
Aidat (TL): [Amount]
Krediye Uygun: Evet
```

**Verification Steps:**
1. Import a villa listing using the importer
2. Verify `isType(_T("villa"))` returns true
3. Check database for new Villa_cstr record
4. Verify villa-specific fields are populated:
   - AcikAlanM2 = "350"
   - KatSayisi = "2"
   - BathroomCount = "5"
   - All other fields mapped correctly

### Test 3: Home/Daire Import (Should Still Work)
**Test URL Pattern:** `https://www.sahibinden.com/ilan/emlak-konut-satilik-daire/detay-*`

**Expected Data:**
```
m² (Brüt): 180
Oda: 3.5+1
Kat: 1
```

**Verification Steps:**
1. Import a home/apartment listing
2. Verify existing functionality still works
3. Check database for Home_cstr record
4. Confirm no regression in existing features

### Test 4: JSON Parsing
**JSON Source:** gaPageViewTrackingJson in HTML

**Verification Steps:**
1. Extract JSON data from test HTML
2. Verify `customVars` array parsing
3. Verify `dmpData` array parsing
4. Check that Turkish key variants are matched:
   - "Marka" → CAR_Brand
   - "Açık Alan m²" → VILLA_OpenArea

### Test 5: HTML Fallback
**Scenario:** JSON extraction fails

**Verification Steps:**
1. Test with page that lacks gaPageViewTrackingJson
2. Verify `ParseHtmlDirectly()` is called
3. Check that regex patterns match HTML structure
4. Confirm fields are extracted from `<li><strong>Label</strong>&nbsp;<span>Value</span></li>` pattern

## Database Schema Verification

### Car Table (Car_cstr)
Verify these fields are populated:
- `Car_Code` (auto-generated)
- `Cari_Kod` (customer reference)
- `Brand`, `Series`, `Model`, `Year`, `Km`
- `FuelType`, `Transmission`, `BodyType`
- `EnginePower`, `EngineVolume`, `Drive`, `Color`
- `DamageRecord`, `Warranty`, `Plate`
- `Price`, `Currency`
- `City`, `District`, `Neighborhood`
- `ListingNo`, `ListingDate`, `ListingURL`

### Villa Table (Villa_cstr)  
Verify these NEW fields are populated:
- `AcikAlanM2` (Open Area)
- `KatSayisi` (Total Floors)
- `BathroomCount`, `HeatingType`, `KitchenType`
- `Parking`, `Furnished`, `UsageStatus`
- `InSite`, `SiteName`, `Dues`
- `CreditEligible`, `DeedStatus`, `SellerType`
- `BuildingAge`, `Swap`
- Attributes: `balcony`, `elevator`

## Edge Cases to Test

1. **Missing Fields:** Verify graceful handling when fields are absent
2. **Turkish Character Encoding:** Test with "İ", "ı", "Ş", "ş", "Ğ", "ğ", "Ü", "ü", "Ö", "ö", "Ç", "ç"
3. **Mixed Category Detection:** URL says "villa" but cat2 says something else
4. **Car with Turkish Characters:** "Yakıt" vs "Yakit", "Çekiş" vs "Cekis"
5. **Duplicate Import Prevention:** Verify same listing isn't imported twice

## Regression Testing

Ensure existing functionality still works:
1. Home/Daire imports
2. Land/Arsa imports  
3. Field/Tarla imports
4. Commercial/Ticari imports
5. Contact extraction
6. Feature extraction
7. Price normalization
8. Customer matching/creation

## Code Review Points

1. ✅ All Car fields properly extracted in 3 locations (JSON, JSON+HTML, HTML fallback)
2. ✅ Villa fields properly extracted in 3 locations
3. ✅ Car detection logic covers: "vasıta", "vasita", "otomobil", cat2, URL
4. ✅ Database insertion creates proper Car_cstr with all fields
5. ✅ Villa insertion enhanced with all missing fields
6. ✅ Fallback logic for Balcony and Elevator in Villa
7. ✅ Title generation for Cars from Brand + Series + Model
8. ✅ Turkish character variants handled (İ/I, ı/i, Ş/S, ş/s, etc.)

## Success Criteria

- [ ] Car listings import successfully to Car_cstr table
- [ ] All 17 car-specific fields are populated from JSON/HTML
- [ ] Villa listings import with all new fields
- [ ] No regression in existing property types (Home, Land, Field, Commercial)
- [ ] Turkish character encoding works correctly
- [ ] HTML fallback works when JSON is unavailable
- [ ] Duplicate detection prevents re-import
- [ ] Customer matching/creation works for all types
