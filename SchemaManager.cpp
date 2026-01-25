

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






