# GitHub Copilot Instructions for Real Estate Management System

## Project Overview

This is a **professional real estate management system** built with C++ and Win32++. It's designed for:
- High-volume operations
- Modular and extensible architecture  
- Easy addition of new property types
- Platform-independent data layer

## Technology Stack

- **Language**: C++11 or later
- **Framework**: Win32++ (Windows GUI)
- **Database**: MS Access / SQL Server via ADO
- **Graphics**: GDI+
- **JSON**: nlohmann/json (header-only)
- **Platform**: Windows 7+
- **Compiler**: Visual C++ 2017+ or MinGW

## Code Style and Conventions

### Naming Conventions
- **Classes**: `CMyClass` (Hungarian notation with 'C' prefix)
- **Member Variables**: `m_variableName` (prefix with 'm_')
- **Functions**: `PascalCase()` 
- **Constants**: `kConstantName`
- **Structs**: `StructName_cstr` (suffix with '_cstr')
- **Dialog IDs**: `IDD_DIALOG_NAME` 
- **Control IDs**: `IDC_CONTROL_NAME`

### Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Allman style (braces on new line)
- **Max Line Length**: 120 characters
- **File Encoding**: UTF-8

### Comments
- Use **Turkish** for business logic and domain-specific comments
- Use **English** for technical implementation details
- Include header comments in every file with purpose and author info

## Architecture Patterns

### LoadFromMap/SaveToMap Pattern
This is the **core data exchange pattern** - always use it for dialogs:

```cpp
// Loading data into dialog
std::map<CString, CString> data = db.FetchRecordMap(...);
dialog.LoadFromMap(data);

// Saving data from dialog
std::map<CString, CString> data;
dialog.SaveToMap(data);
db.UpdateRecord(data);
```

**Why?** This pattern ensures:
- Platform-independent data flow
- Database independence (works with any backend)
- Easy testing with mock data
- Simple serialization (JSON/XML/...)

### Modular Property System

When adding a new property type, use the **LEGO approach**:

```cpp
// 1. Define data structure (dataIsMe.h)
struct NewPropertyType_cstr {
    CString Property_Code;
    CString Cari_Kod;
    // ... fields
};

// 2. Create dialog class (vNewPropertyDlg.h/cpp)
class CNewPropertyDialog : public CDialog {
    void LoadFromMap(const std::map<CString, CString>& data);
    void SaveToMap(std::map<CString, CString>& data) const;
};

// 3. Register with ONE line (PropertyModuleRegistry.h)
REGISTER_PROPERTY_DIALOG(
    _T("new_type"),           // Type key
    _T("NewPropertyTable"),   // Table name  
    _T("Display Name"),       // Turkish display name
    CNewPropertyDialog        // Dialog class
);
```

### Resource Management
- Always use RAII wrappers (Win32xx classes)
- Use `CFont`, `CBrush`, `CPen` instead of raw HFONT, HBRUSH, HPEN
- Never forget to release GDI/COM resources

## Best Practices

### DO:
- ✅ Use `LoadFromMap/SaveToMap` for all dialogs
- ✅ Use RAII for resource management
- ✅ Use `REGISTER_PROPERTY_DIALOG` macro for new property types
- ✅ Implement responsive layouts (dynamic column calculation)
- ✅ Use `FeaturesCodec` for encoding/decoding JSON arrays
- ✅ Test with real data from Sahibinden.com imports
- ✅ Keep dialogs database-agnostic
- ✅ Use Win32++ wrappers instead of raw Win32 API

### DON'T:
- ❌ Don't directly bind UI to database structs
- ❌ Don't hard-code window sizes (use responsive layouts)
- ❌ Don't use raw pointers for Win32 resources
- ❌ Don't mix business logic with UI code
- ❌ Don't forget to handle dialog resizing
- ❌ Don't use tabs in source files (use spaces)
- ❌ Don't break the LoadFromMap/SaveToMap contract

## Key Files and Their Purpose

- `PropertyModuleRegistry.h` - Central property type registry (LEGO system)
- `dataIsMe.h` - All data structures (`*_cstr` structs)
- `vHomeDlg.h/cpp` - Example dialog (Home/Apartment)
- `vVillaDlg.h/cpp` - Example dialog (Villa)
- `CHomeFeaturesPage.h/cpp` - Reusable dynamic checkbox page
- `FeaturesCodec.h/cpp` - JSON encoding/decoding for features
- `SahibindenParser.h/cpp` - Clipboard data parsing
- `AllGlobalHeader.h` - Global includes and definitions

## Common Tasks

### Adding a New Property Type
See `DEVELOPER_GUIDE_ADD_PROPERTY.md` for step-by-step instructions.

### Parsing Sahibinden.com Data
- Use `SahibindenParser` class
- Support heuristic field detection
- Handle multiple alias formats
- Sanitize and normalize data

### Working with Dynamic Features
- Use `CHomeFeaturesPage` component
- Features are stored as JSON arrays in database
- Use `FeaturesCodec` for serialization
- Support runtime checkbox generation

## Testing Guidelines

### Manual Testing
- Test all property type dialogs
- Test tab navigation and scrolling  
- Test Sahibinden clipboard import
- Test responsive layout at different window sizes
- Verify data persistence (save/load cycle)

### Performance Targets
- Dialog open: < 100ms
- Tab switch: < 50ms
- 1000 checkbox layout: < 200ms
- Scroll at 60 FPS: ~16ms/frame

## Documentation

When making changes:
- Update relevant `.md` files in the root directory
- Keep `README.md` synchronized with features
- Document new patterns in `ARCHITECTURE_GUIDE.md`
- Add examples to developer guides

## Additional Resources

- **Architecture**: `ARCHITECTURE_GUIDE.md`
- **Manifesto**: `MANIFESTO.md` (project vision)
- **Quick Start**: `QUICK_START.md`
- **Property System**: `MODULAR_PROPERTY_SYSTEM.md`
- **Refactoring Notes**: `REFACTORING_SUMMARY.md`

## Language Notes

- This is primarily a **Turkish-language** project for the Turkish real estate market
- UI strings and business terms should be in Turkish
- Technical documentation can be in English
- Code comments use both languages as appropriate

---

**Remember**: This system is designed to be modular, maintainable, and scalable. When in doubt, follow the existing patterns in `vHomeDlg.cpp` and `vVillaDlg.cpp`.
