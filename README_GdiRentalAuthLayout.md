# Kiralık Tek Yetki Sözleşmesi - GDI Rendering Implementation

## Quick Start

This implementation provides GDI-based rendering for the Kiralık Tek Yetki Sözleşmesi (Rental Authorization Agreement) form.

## Files Overview

### Core Implementation
- **GdiRentalAuthLayout.h** - Class declaration (3 pages)
- **GdiRentalAuthLayout.cpp** - Complete rendering implementation

### Documentation
- **GdiRentalAuthLayout_FieldMapping.md** - Complete field reference (40+ fields)
- **FORM_STRUCTURE_VISUALIZATION.md** - ASCII visualization of all 3 pages
- **IMPLEMENTATION_SUMMARY.md** - Detailed implementation notes
- **README_GdiRentalAuthLayout.md** - This file

## Usage

### From Code

```cpp
#include "GdiRentalAuthLayout.h"

// 1. Create layout instance
GdiRentalAuthLayout layout;

// 2. Prepare data
std::vector<std::pair<CString, CString>> fields;
fields.push_back({_T("OwnerName"), _T("Mehmet Yılmaz")});
fields.push_back({_T("OwnerTC"), _T("12345678901")});
fields.push_back({_T("OwnerPhone"), _T("0532 123 45 67")});
// ... add more fields

// 3. Set data
layout.SetData(fields);

// 4. Render pages
IDrawContext& ctx = /* your draw context */;
for (int page = 1; page <= 3; page++) {
    layout.Render(ctx, page);
}
```

### From UI

1. Select a property in the tree view
2. Click "Kiralık Yetki" menu item
3. System calls `OnCreateRentalAuthContract()`
4. Form preview opens with all 3 pages

## Form Structure

### Page 1: Bilgiler (Information)
- İş Sahibi Bilgileri (Owner Information) - 5 fields
- Emlak İşletmesi Bilgileri (Real Estate Agency) - 5 fields  
- Gayrimenkul Tapu Kaydi (Property Deed) - 9 fields
- Gayrimenkul Adresi (Property Address) - 5 fields including checkboxes
- İmar Durumu (Zoning Status) - 2 fields

### Page 2: Özellikler (Features)
- Yapıya Ait Bilgiler (Building Info) - 7 fields
- İletişim Olanakları (Access) - 4 fields including parking options
- Yapı Bölüm Alanları (Building Areas) - 5 fields
- İç Özellikler (Interior Features) - 12 checkboxes in grid

### Page 3: Hizmet ve İmza (Service & Signatures)
- Açıklamalar (Descriptions) - 2 fields
- Kiralama Bedeli (Rental Amount) - 2 fields
- Kiralama Hizmet Bedeli (Service Fee) - Legal text paragraph
- Tarafların Yükümlülükleri (Obligations) - Bullet points
- İmza Alanları (Signatures) - 3 signature boxes

## Key Features

✅ **Win32++ Compatible** - No MFC/ATL dependencies
✅ **Turkish Characters** - Full UTF-8/Unicode support (ç, ğ, ı, ö, ş, ü)
✅ **Professional Layout** - Follows real estate industry standards
✅ **3 Pages** - Properly paginated A4 format
✅ **40+ Fields** - Comprehensive data coverage
✅ **Color Coded** - Red headers, blue data, gray backgrounds
✅ **Checkboxes** - For options and feature selection
✅ **Signature Boxes** - 3 parties (Agency, Company, Owner)

## Color Scheme

- **Headers**: RGB(200, 0, 0) - Red
- **Data Values**: RGB(0, 0, 150) - Blue
- **Labels**: RGB(0, 0, 0) - Black
- **Section Backgrounds**: RGB(220, 220, 220) - Light Gray
- **Page Background**: RGB(255, 255, 255) - White

## Font Specifications

- **Title**: Arial 20pt Bold (Page 1), 16pt Bold (Pages 2-3)
- **Section Headers**: Arial 12pt Bold
- **Labels**: Arial 10pt Bold
- **Data**: Arial 10pt Regular
- **Body Text**: Arial 9pt Regular
- **Footer**: Arial 8pt Regular

## Data Field Categories

### Owner (İş Sahibi)
- OwnerName, OwnerTC, OwnerPhone, OwnerEmail, OwnerAddress

### Agent (Emlak İşletmesi)
- AgentName, AgentTitle, AgentPhone, AgentEmail, AgentAddress

### Property Location
- City, District, Neighborhood, FullAddress, Owners

### Deed Registration
- Pafta, Ada, Parsel, Floor, BuildingNo, LandShare

### Zoning
- ZoningArea, ZoningStatus

### Building Info
- LandArea, BuildingAge, ActualUse, ConstructionStatus
- GrossArea, NetArea, NaturalGasArea

### Transportation
- PublicTransport, RailSystems, SeaTransport

### Areas
- LivingRoom, Kitchen, Bathroom, WC, BedroomCount

### Rental Terms
- PropertyType, OccupancyStatus
- RentAmountNumeric, RentAmountText

## Technical Details

### Dimensions
- Page Size: 2100 x 2970 logical units (A4)
- Margins: 50 units
- Content Area: 1900 x 2870 units
- Line Height: 45 units
- Checkbox: 18 x 18 units
- Signature Box: 550 x 200 units

### Rendering
- Uses IDrawContext interface
- Platform-independent drawing
- Supports GDI, GDI+, Cairo, Haru PDF
- UTF-8 text encoding

### Data Binding
- SetData() accepts vector of CString pairs
- GetVal() retrieves values with fallback
- Missing fields show "....................."

## Integration Points

This layout integrates with:
- **LayoutFactory**: Creates instances based on document type
- **PreviewPanel**: Displays rendered pages
- **CTreeListVDlg**: Triggers form creation from property selection
- **DatabaseManager**: Fetches property and owner data

## Build Requirements

- Windows SDK
- C++11 or later
- Win32++ library (header-only)
- GDI API (included in Windows)

## Testing

Manual testing recommended:
1. Select property with complete data
2. Generate form preview
3. Verify all 3 pages render correctly
4. Check Turkish character display
5. Test PDF export
6. Print to verify layout

## Security

✅ CodeQL scan completed - No security issues found

## Support

For field mapping details, see: `GdiRentalAuthLayout_FieldMapping.md`
For visual layout, see: `FORM_STRUCTURE_VISUALIZATION.md`
For implementation details, see: `IMPLEMENTATION_SUMMARY.md`

## Version History

- v1.0 (2026-01-25) - Initial complete implementation
  - 3-page form rendering
  - 40+ data fields
  - Turkish character support
  - Professional layout with colors
  - Signature sections
  - Code review feedback addressed
  - Security scan passed

## License

Part of the realestatep project.
