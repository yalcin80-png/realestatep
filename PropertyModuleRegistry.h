#pragma once

// ============================================================================
//  PropertyModuleRegistry.h
//  ULTRA MODÜLER "LEGO" MÜLK EKLEME ALTYAPISI (Win32++)
//  --------------------------------------------------------------------------
//  Hedef:
//   - Yeni bir mülk türü eklemek için core tarafı değiştirmeden,
//     sadece 3 parça eklenerek sistemin otomatik tanıması:
//        1) XxxDlg.h/.cpp  (Dialog sınıfı)
//        2) XxxDlg.rc      (Resource)
//        3) Xxx_cstr + PropertyMap listesi (dataIsMe.h/.cpp içinde)
//     + 1 satır REGISTER_PROPERTY_DIALOG(...) macro.
// ============================================================================

#include <functional>
#include <vector>
#include <unordered_map>

#include "stdafx.h"
#include "dataIsMe.h"   // DialogMode, DatabaseManager forward

class DatabaseManager;

// ---------------------------------------------------------------------------
// Dinamik komut aralığı (resource.h'a dokunmadan kullanılabilir)
// ---------------------------------------------------------------------------
static constexpr UINT IDM_PROPERTY_ADD_BASE = 34000;
static constexpr UINT IDM_PROPERTY_ADD_MAX  = 34999; // [BASE..MAX]

struct PropertyModuleDesc
{
    CString typeId;       // "home", "land", "field"...
    CString tableName;    // DB tablo adı (ayrı tablolar)
    CString displayName;  // Menüde görünen ad

    UINT cmdAdd = 0;      // runtime atanır

    // Dialog açıcı (yeni / güncelleme)
    //  - mode: INEWUSER / IUPDATEUSER / IVIEWER
    //  - cariKod: müşteri
    //  - recordCode: ilgili tablodaki anahtar (Home_Code, Land_Code...) - update için
    std::function<void(DatabaseManager& db, HWND parent, DialogMode mode,
                       const CString& cariKod, const CString& recordCode)> openDialog;
};

class PropertyModuleRegistry
{
public:
    static PropertyModuleRegistry& Instance()
    {
        static PropertyModuleRegistry g;
        return g;
    }

    // Modül kaydı (AutoReg macro bunu çağırır)
    void Register(PropertyModuleDesc d)
    {
        // CmdId runtime ver
        if (m_nextCmd <= IDM_PROPERTY_ADD_MAX)
            d.cmdAdd = m_nextCmd++;

        m_modules.emplace_back(d);
        m_byType[(LPCTSTR)d.typeId] = (int)m_modules.size() - 1;
        if (d.cmdAdd)
            m_byCmd[d.cmdAdd] = (int)m_modules.size() - 1;
    }

    const PropertyModuleDesc* FindByType(const CString& typeId) const
    {
        auto it = m_byType.find((LPCTSTR)typeId);
        if (it == m_byType.end()) return nullptr;
        return &m_modules[(size_t)it->second];
    }

    const PropertyModuleDesc* FindByCmd(UINT cmdId) const
    {
        auto it = m_byCmd.find(cmdId);
        if (it == m_byCmd.end()) return nullptr;
        return &m_modules[(size_t)it->second];
    }

    static bool IsAddCommand(UINT cmdId)
    {
        return (cmdId >= IDM_PROPERTY_ADD_BASE && cmdId <= IDM_PROPERTY_ADD_MAX);
    }

    const std::vector<PropertyModuleDesc>& Modules() const { return m_modules; }

    // Helper: cmdId -> dialog aç
    bool OpenAddByCmd(UINT cmdId, DatabaseManager& db, HWND parent, const CString& cariKod) const
    {
        const auto* m = FindByCmd(cmdId);
        if (!m || !m->openDialog) return false;
        m->openDialog(db, parent, INEWUSER, cariKod, CString());
        return true;
    }

private:
    PropertyModuleRegistry() = default;

private:
    UINT m_nextCmd = IDM_PROPERTY_ADD_BASE;
    std::vector<PropertyModuleDesc> m_modules;
    std::unordered_map<std::wstring, int> m_byType;
    std::unordered_map<UINT, int> m_byCmd;
};

// ---------------------------------------------------------------------------
// Auto-register Macro
//   DLG_CLASS ctor: (DatabaseManager&, DialogMode, const CString&, const CString&)
//   DLG_CLASS::DoModal(HWND)
// ---------------------------------------------------------------------------
// Not: Bu macro'yı .cpp dosyasına koyun (header'a koymayın) ki ODR/duplicate
//      static init riskleri azalır.
// ---------------------------------------------------------------------------

#define REGISTER_PROPERTY_DIALOG(TYPE_ID, TABLE_NAME, DISPLAY_NAME, DLG_CLASS) \
namespace { \
    struct AutoReg_##DLG_CLASS { \
        AutoReg_##DLG_CLASS() { \
            PropertyModuleDesc d; \
            d.typeId = (TYPE_ID); \
            d.tableName = (TABLE_NAME); \
            d.displayName = (DISPLAY_NAME); \
            d.openDialog = [](DatabaseManager& db, HWND parent, DialogMode mode, \
                              const CString& cariKod, const CString& recordCode) { \
                DLG_CLASS dlg(db, mode, cariKod, recordCode); \
                dlg.DoModal(parent); \
            }; \
            PropertyModuleRegistry::Instance().Register(d); \
        } \
    }; \
    static AutoReg_##DLG_CLASS g_AutoReg_##DLG_CLASS; \
}
