#pragma once
#include "stdafx.h"
#include "resource.h"
#include "dataIsMe.h"
#include "SchemaManager.h"
#include "SahibindenParser.h"
#include <map>

// vFieldDlg.h

class CFieldDialog : public CDialog
{
public:
    // Constructor
    CFieldDialog(DatabaseManager& db, DialogMode mode,
        const CString& cariKod, const CString& fieldCodeToEdit = _T(""))
        : CDialog(IDD_FLD_DIALOG),
        m_db(db),
        m_mode(mode),
        m_cariKod(cariKod),
        m_fieldCodeToEdit(fieldCodeToEdit) {
    }

    // Public Setters/Getters
    void SetCariKod(const CString& code) { m_cariKod = code; }

    // Dışarıdan tetiklenebilen olaylar
    void OnLoadFromClipboard();
    void OnLoadFromKml();
    void OnOpenInEarth();
    void OnMergeWithKml();

protected:
    BOOL OnInitDialog() override;
    void OnOK() override;
    INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    // Arayüz Elemanlarını Kurma
    void OnSetCtrl();
    void FillCombo(int id, const std::vector<CString>& items);
private:
    // --- Üye Değişkenler ---
    DatabaseManager& m_db = DatabaseManager::GetInstance();
    DialogMode       m_mode;
    CString          m_cariKod;
    CString          m_fieldCodeToEdit;

    // Geçici veri tutucular
    Field_cstr       m_currentRecord; // Veritabanı kaydı için esas struct
    CString          m_rawKmlData;    // KML verisini saklar

private:
    // --- Yardımcı Fonksiyonlar ---

    // 1. Veri <-> Arayüz Köprüsü (SchemaManager kullanır)
    void UpdateUIFromMap(const std::map<CString, CString>& dataMap);
    std::map<CString, CString> GetMapFromUI();

    // 2. Struct <-> Map Dönüşümü (Field_cstr yapısını dinamik yapıya çevirir)
    std::map<CString, CString> StructToMap(const Field_cstr& d);
    void MapToStruct(const std::map<CString, CString>& map, Field_cstr& d);

    // 3. UI Yardımcıları
    void LoadCombos(); // ComboBox içeriklerini doldurur
    void SetControlText(UINT id, const CString& text);
    CString GetControlText(UINT id);
    void SetComboSelection(UINT id, const CString& value);

    // 4. Dosya İşlemleri
    CString ReadFileContent(const CString& path); // UTF-8 aware file reader
    std::map<CString, CString> ParseKmlInternal(const CString& kmlContent);
    void SanitizeDataMap(std::map<CString, CString>& dataMap);
    // KML ve Sahibinden datalarını zekice birleştirir
    std::map<CString, CString> SmartMerge(
        const std::map<CString, CString>& primary,
        const std::map<CString, CString>& secondary);
};