#pragma once
#ifndef DATAMAPPER_H
#define DATAMAPPER_H

#include "stdafx.h"
#include <map>
#include <vector>
#include <algorithm>

// ============================================================================
// DATAMAPPER: Central Data Mapping and Normalization Utility
// ============================================================================
// This class handles mapping between database field names and 
// the expected field names in JSON templates and GDI layouts.
// It also provides normalization and validation of field values.
// ============================================================================

class DataMapper
{
public:
    // Map database fields to standard/expected field names
    // Handles case-insensitive lookup and aliases
    static std::map<CString, CString> MapFields(
        const std::vector<std::pair<CString, CString>>& dbFields);
    
    // Normalize field value (trim whitespace, handle encoding)
    static CString NormalizeValue(const CString& value);
    
    // Get value with fallback (returns empty string if not found)
    static CString GetValue(
        const std::map<CString, CString>& data,
        const CString& key,
        const CString& defaultValue = _T(""));
    
    // Check if a boolean field is true
    static bool GetBoolValue(
        const std::map<CString, CString>& data,
        const CString& key);
    
    // Common field name aliases (for case-insensitive and variant matching)
    static CString NormalizeFieldName(const CString& fieldName);

private:
    // Field name mapping table (DB name -> Standard name)
    static std::map<CString, CString> GetFieldAliases();
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline std::map<CString, CString> DataMapper::GetFieldAliases()
{
    static std::map<CString, CString> aliases;
    if (aliases.empty())
    {
        // Common field mappings (case variations, aliases)
        // These help bridge differences between DB schema and template expectations
        
        // Owner/Customer fields
        aliases[_T("ownername")] = _T("OwnerName");
        aliases[_T("owner_name")] = _T("OwnerName");
        aliases[_T("customername")] = _T("OwnerName");
        
        aliases[_T("ownertc")] = _T("OwnerTC");
        aliases[_T("owner_tc")] = _T("OwnerTC");
        aliases[_T("tcno")] = _T("OwnerTC");
        
        aliases[_T("ownerphone")] = _T("OwnerPhone");
        aliases[_T("owner_phone")] = _T("OwnerPhone");
        aliases[_T("phone")] = _T("OwnerPhone");
        aliases[_T("telephone")] = _T("OwnerPhone");
        
        aliases[_T("owneraddress")] = _T("OwnerAddress");
        aliases[_T("owner_address")] = _T("OwnerAddress");
        aliases[_T("customeraddress")] = _T("OwnerAddress");
        
        // Property fields
        aliases[_T("fulladdress")] = _T("FullAddress");
        aliases[_T("full_address")] = _T("FullAddress");
        aliases[_T("address")] = _T("FullAddress");
        aliases[_T("propertyaddress")] = _T("FullAddress");
        
        aliases[_T("city")] = _T("City");
        aliases[_T("il")] = _T("City");
        
        aliases[_T("district")] = _T("District");
        aliases[_T("ilce")] = _T("District");
        
        aliases[_T("neighborhood")] = _T("Neighborhood");
        aliases[_T("mahalle")] = _T("Neighborhood");
        
        aliases[_T("street")] = _T("Street");
        aliases[_T("sokak")] = _T("Street");
        
        aliases[_T("price")] = _T("Price");
        aliases[_T("fiyat")] = _T("Price");
        
        aliases[_T("currency")] = _T("Currency");
        aliases[_T("parabirimi")] = _T("Currency");
        
        // Date fields
        aliases[_T("date")] = _T("Date");
        aliases[_T("tarih")] = _T("Date");
        aliases[_T("createdate")] = _T("Date");
    }
    return aliases;
}

inline CString DataMapper::NormalizeFieldName(const CString& fieldName)
{
    CString normalized = fieldName;
    normalized.MakeLower();
    normalized.Trim();
    
    auto aliases = GetFieldAliases();
    auto it = aliases.find(normalized);
    if (it != aliases.end())
        return it->second;
    
    return fieldName; // Return original if no alias found
}

inline std::map<CString, CString> DataMapper::MapFields(
    const std::vector<std::pair<CString, CString>>& dbFields)
{
    std::map<CString, CString> result;
    
    for (const auto& field : dbFields)
    {
        // Store both original key and normalized key
        result[field.first] = NormalizeValue(field.second);
        
        // Also add with normalized field name
        CString normalizedKey = NormalizeFieldName(field.first);
        if (normalizedKey != field.first)
            result[normalizedKey] = NormalizeValue(field.second);
    }
    
    return result;
}

inline CString DataMapper::NormalizeValue(const CString& value)
{
    CString normalized = value;
    normalized.Trim(); // Remove leading/trailing whitespace
    
    // Handle encoding issues - ensure proper Unicode representation
    // This is especially important for Turkish characters (ı, ğ, ü, ş, ö, ç)
    
    return normalized;
}

inline CString DataMapper::GetValue(
    const std::map<CString, CString>& data,
    const CString& key,
    const CString& defaultValue)
{
    // Try direct lookup
    auto it = data.find(key);
    if (it != data.end() && !it->second.IsEmpty())
        return it->second;
    
    // Try normalized lookup
    CString normalizedKey = NormalizeFieldName(key);
    it = data.find(normalizedKey);
    if (it != data.end() && !it->second.IsEmpty())
        return it->second;
    
    return defaultValue;
}

inline bool DataMapper::GetBoolValue(
    const std::map<CString, CString>& data,
    const CString& key)
{
    CString value = GetValue(data, key);
    value.MakeLower();
    value.Trim();
    
    // Supported boolean true values: "1", "true", "yes", "evet" (Turkish), "x"
    // This list can be extended as needed
    return (value == _T("1") || 
            value == _T("true") || 
            value == _T("yes") || 
            value == _T("evet") ||  // Turkish: "yes"
            value == _T("x"));      // Checkbox marker
}

#endif // DATAMAPPER_H
