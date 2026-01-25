#pragma once
#include "stdafx.h"
#include "resource.h" // ID'leri tanıması için şart
#include <vector>
#include <map>

// Veri Tipleri
enum FieldType { 
    FT_TEXT,    // Metin (EditBox, ComboBox)
    FT_INT,     // Tam Sayı
    FT_DOUBLE,  // Ondalıklı Sayı (Fiyat, m2)
    FT_DATE,    // Tarih
    FT_BOOL,    // Checkbox (Evet/Hayır)
    FT_MEMO     // Uzun Metin
};


// Senin beğendiğin yapı
struct FieldDef {
    CString dbName;    // Veritabanı Kolon Adı ("Ada")
    UINT resID;        // Formdaki ID (IDC_EDIT_ADA) - Yoksa 0
    CString label;     // Liste Başlığı ("Ada No")
    FieldType type;    // Tipi
    int width;         // Liste genişliği
    bool visible;      // Listede görünsün mü?
    bool isAttribute;  // TRUE ise JSON'a gider, FALSE ise Kolondur.
    std::vector<CString> aliases; // Opsiyonel: alternatif anahtar adları
    FieldDef(CString db, UINT id, CString lbl, FieldType t, int w = 100, bool v = true)
        : dbName(db), resID(id), label(lbl), type(t), width(w), visible(v), isAttribute(false), aliases() {}
};





template<typename T>
    void LoadSchemaFromFactory(std::map<CString, std::vector<FieldDef>>& schemas, const CString& tableName);



class SchemaManager {
public:
    static SchemaManager& Get() {
        static SchemaManager instance;
        return instance;
    }




    // Tablonun alan listesini getirir
    const std::vector<FieldDef>& GetSchema(const CString& tableName);
    // Bu fonksiyonu her struct için özelleştireceğiz (Fabrika Deposu)
private:
    SchemaManager() {} // Private Constructor
    void InitSchemas(); // Tanımları dolduracak fonksiyon
    std::map<CString, std::vector<FieldDef>> m_schemas;
    bool m_isInit = false;
}; 


