

#include "stdafx.h"
#include "dataIsMe.h"
#include "SchemaManager.h"
#include "resource.h"

// Singleton örneğini oluştur
//SchemaManager& SchemaManager::Get() {
//    static SchemaManager instance;
//    return instance;
//}




const std::vector<FieldDef>& SchemaManager::GetSchema(const CString& tableName) {
    if (!m_isInit) {
        InitSchemas();
        m_isInit = true;
    }
    return m_schemas[tableName];
}

// 🛠️ YARDIMCI FONKSİYON (FABRİKA)
// Veri kaynağı (GetProperties) ile Şema (FieldDef) arasındaki köprü.
// Bu fonksiyon, dataIsMe.cpp içindeki merkezi listeyi okuyup şemayı doldurur.
template<typename T>
void LoadSchemaFromFactory(std::map<CString, std::vector<FieldDef>>& schemas, const CString& tableName)
{
    const auto& props = GetProperties<T>(); // Merkezi Fabrikadan Çek
    std::vector<FieldDef> fields;

    for (const auto& p : props) {
        // FieldDef oluştur (dbName, resID, label, type, width, visible)
        fields.emplace_back(p.dbName, p.resID, p.label, p.type, p.width, p.visible);

        // isAttribute özelliğini ayrıca set et (Constructor'da yoksa)
        fields.back().isAttribute = p.isAttribute;
        fields.back().aliases = p.aliases;
    }
    schemas[tableName] = fields;
}

void SchemaManager::InitSchemas() {
    m_schemas.clear();

    LoadSchemaFromFactory<Customer_cstr>(m_schemas, TABLE_NAME_CUSTOMER);
    LoadSchemaFromFactory<Home_cstr>(m_schemas, TABLE_NAME_HOME);
    LoadSchemaFromFactory<Land_cstr>(m_schemas, TABLE_NAME_LAND);
    LoadSchemaFromFactory<Field_cstr>(m_schemas, TABLE_NAME_FIELD);
    LoadSchemaFromFactory<Vineyard_cstr>(m_schemas, TABLE_NAME_VINEYARD);
    LoadSchemaFromFactory<Villa_cstr>(m_schemas, TABLE_NAME_VILLA);
    LoadSchemaFromFactory<Commercial_cstr>(m_schemas, TABLE_NAME_COMMERCIAL);
    LoadSchemaFromFactory<Car_cstr>(m_schemas, TABLE_NAME_CAR);
    LoadSchemaFromFactory<Offer_cstr>(m_schemas, TABLE_NAME_OFFERS);
    LoadSchemaFromFactory<Appointment_cstr>(m_schemas, TABLE_NAME_APPOINTMENT);
    LoadSchemaFromFactory<CompanyInfo_cstr>(m_schemas, TABLE_NAME_SETTINGS);


}

// ============================================================================
// SCHEMA VALIDATION - Runtime validation of database fields
// ============================================================================

bool SchemaManager::ValidateFields(
    const CString& tableName,
    const std::vector<std::pair<CString, CString>>& fields,
    std::vector<CString>& errors)
{
    errors.clear();
    
    if (!m_isInit) {
        InitSchemas();
        m_isInit = true;
    }
    
    auto schemaIt = m_schemas.find(tableName);
    if (schemaIt == m_schemas.end())
    {
        errors.push_back(_T("Schema not found for table: ") + tableName);
        return false;
    }
    
    const auto& schema = schemaIt->second;
    bool isValid = true;
    
    // Build a map of field names for quick lookup
    std::map<CString, CString> fieldMap;
    for (const auto& field : fields)
    {
        fieldMap[field.first] = field.second;
    }
    
    // Check for required fields with empty values
    for (const auto& fieldDef : schema)
    {
        // Skip if not visible or not an attribute
        if (!fieldDef.visible || !fieldDef.isAttribute)
            continue;
        
        auto it = fieldMap.find(fieldDef.dbName);
        if (it == fieldMap.end() || it->second.IsEmpty())
        {
            // Check aliases
            bool foundInAlias = false;
            for (const auto& alias : fieldDef.aliases)
            {
                auto aliasIt = fieldMap.find(alias);
                if (aliasIt != fieldMap.end() && !aliasIt->second.IsEmpty())
                {
                    foundInAlias = true;
                    break;
                }
            }
            
            if (!foundInAlias)
            {
                // Field is missing or empty - this is a validation issue
                CString msg;
                msg.Format(_T("Field '%s' (%s) is empty or missing"), 
                    fieldDef.dbName, fieldDef.label);
                errors.push_back(msg);
                isValid = false; // Mark as invalid
            }
        }
    }
    
    return isValid;
}

CString SchemaManager::GetFieldLabel(const CString& tableName, const CString& fieldName)
{
    if (!m_isInit) {
        InitSchemas();
        m_isInit = true;
    }
    
    auto schemaIt = m_schemas.find(tableName);
    if (schemaIt == m_schemas.end())
        return fieldName;
    
    const auto& schema = schemaIt->second;
    for (const auto& fieldDef : schema)
    {
        if (fieldDef.dbName == fieldName)
            return fieldDef.label;
        
        // Check aliases
        for (const auto& alias : fieldDef.aliases)
        {
            if (alias == fieldName)
                return fieldDef.label;
        }
    }
    
    return fieldName;
}






