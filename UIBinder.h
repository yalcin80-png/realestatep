#pragma once
#include "stdafx.h"
#include <vector>
#include <memory>

// =============================================================
//  1. SOYUT ARAYÜZ (STRATEJİ)
//  Her kontrol tipi (Edit, Combo, Date) bu sınıfı miras alacak.
// =============================================================
class IControlBinder
{
public:
    virtual ~IControlBinder() = default;

    // Bu binder bu pencere tipini (örn: "Edit") yönetebilir mi?
    virtual bool CanHandle(HWND hWnd) const = 0;

    // Veriyi Kontrole Yaz (DB -> UI)
    virtual void Set(HWND hWnd, const CString& value) = 0;

    // Veriyi Kontrolden Oku (UI -> DB)
    virtual CString Get(HWND hWnd) = 0;

protected:
    // Yardımcı: Pencere sınıf adını öğren
    CString GetClassName(HWND hWnd) const {
        TCHAR szClass[64];
        ::GetClassName(hWnd, szClass, 64);
        return CString(szClass);
    }
};

// =============================================================
//  2. MERKEZİ FABRİKA (FACTORY)
//  Hangi kontrol için hangi binder'ın çalışacağını seçer.
// =============================================================
class UIFactory
{
public:
    static UIFactory& Get() {
        static UIFactory instance;
        return instance;
    }

    // Sisteme yeni kontrol tipleri buradan eklenir
    void RegisterBinder(std::shared_ptr<IControlBinder> binder) {
        m_binders.push_back(binder);
    }

    // Veriyi otomatik olarak doğru kontrole yazar
    void SetValueToControl(HWND hWnd, const CString& value);

    // Veriyi otomatik olarak doğru kontrolden okur
    CString GetValueFromControl(HWND hWnd);

private:
    UIFactory(); // Private Constructor (Singleton)
    std::vector<std::shared_ptr<IControlBinder>> m_binders;
};