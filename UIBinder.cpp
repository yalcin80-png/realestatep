#include "stdafx.h"
#include "UIBinder.h"
#include <commctrl.h> // DatePicker için

// =============================================================
//  A) EDIT BOX (Metin Kutusu) İŞLEYİCİSİ
// =============================================================
class EditBinder : public IControlBinder
{
public:
    bool CanHandle(HWND hWnd) const override {
        CString cls = GetClassName(hWnd);
        return (cls.CompareNoCase(_T("Edit")) == 0 || cls.CompareNoCase(_T("RichEdit20W")) == 0);
    }

    void Set(HWND hWnd, const CString& value) override {
        ::SetWindowText(hWnd, value);
    }

    CString Get(HWND hWnd) override {
        int len = ::GetWindowTextLength(hWnd);
        CString str;
        ::GetWindowText(hWnd, str.GetBuffer(len + 1), len + 1);
        str.ReleaseBuffer();
        return str;
    }
};

// =============================================================
//  B) COMBO BOX (Açılır Kutu) İŞLEYİCİSİ
// =============================================================
class ComboBoxBinder : public IControlBinder
{
public:
    bool CanHandle(HWND hWnd) const override {
        return (GetClassName(hWnd).CompareNoCase(_T("ComboBox")) == 0);
    }

    void Set(HWND hWnd, const CString& value) override {
        // Önce listede var mı diye bak ve seç
        LRESULT idx = ::SendMessage(hWnd, CB_SELECTSTRING, -1, (LPARAM)(LPCTSTR)value);
        
        // Listede yoksa ve DropDown (yazılabilir) ise metni direkt yaz
        if (idx == CB_ERR) {
            ::SetWindowText(hWnd, value);
        }
    }

    CString Get(HWND hWnd) override {
        // Seçili index var mı?
        int sel = (int)::SendMessage(hWnd, CB_GETCURSEL, 0, 0);
        if (sel != CB_ERR) {
            // Listeden al
            int len = (int)::SendMessage(hWnd, CB_GETLBTEXTLEN, sel, 0);
            CString str;
            ::SendMessage(hWnd, CB_GETLBTEXT, sel, (LPARAM)str.GetBuffer(len + 1));
            str.ReleaseBuffer();
            return str;
        }
        // Yoksa edit alanından al (Editable ise)
        int len = ::GetWindowTextLength(hWnd);
        CString str;
        ::GetWindowText(hWnd, str.GetBuffer(len + 1), len + 1);
        str.ReleaseBuffer();
        return str;
    }
};

// =============================================================
//  C) CHECK BOX / RADIO BUTTON İŞLEYİCİSİ
// =============================================================
class ButtonBinder : public IControlBinder
{
public:
    bool CanHandle(HWND hWnd) const override {
        return (GetClassName(hWnd).CompareNoCase(_T("Button")) == 0);
    }

    void Set(HWND hWnd, const CString& value) override {
        // "1", "True", "Evet" -> Checked
        bool bCheck = (value == _T("1") || value.CompareNoCase(_T("True")) == 0 || value.CompareNoCase(_T("Evet")) == 0);
        ::SendMessage(hWnd, BM_SETCHECK, bCheck ? BST_CHECKED : BST_UNCHECKED, 0);
    }

    CString Get(HWND hWnd) override {
        return (::SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) ? _T("1") : _T("0");
    }
};

// =============================================================
//  D) DATETIME PICKER (Tarih Seçici) İŞLEYİCİSİ
// =============================================================
class DateTimeBinder : public IControlBinder
{
public:
    bool CanHandle(HWND hWnd) const override {
        return (GetClassName(hWnd).CompareNoCase(_T("SysDateTimePick32")) == 0);
    }

    void Set(HWND hWnd, const CString& value) override {
        if (value.IsEmpty()) {
            // Boşsa check'i kaldır (No Date)
            // DateTime_SetSystemTime(hWnd, GDT_NONE, NULL); // Win32 Macro
            ::SendMessage(hWnd, DTM_SETSYSTEMTIME, GDT_NONE, 0);
            return;
        }

        // String (YYYY-MM-DD) -> SYSTEMTIME çevrimi
        SYSTEMTIME st = {0};
        int y=0, m=0, d=0;
        if (_stscanf_s(value, _T("%d-%d-%d"), &y, &m, &d) == 3) {
            st.wYear = y; st.wMonth = m; st.wDay = d;
            ::SendMessage(hWnd, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
        }
    }

    CString Get(HWND hWnd) override {
        SYSTEMTIME st = {0};
        if (::SendMessage(hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&st) == GDT_VALID) {
            CString s; s.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
            return s;
        }
        return _T("");
    }
};

// =============================================================
//  E) LIST BOX İŞLEYİCİSİ (Gelecek İçin Hazırlık)
// =============================================================
class ListBoxBinder : public IControlBinder
{
public:
    bool CanHandle(HWND hWnd) const override {
        return (GetClassName(hWnd).CompareNoCase(_T("ListBox")) == 0);
    }

    void Set(HWND hWnd, const CString& value) override {
        // Gelen değer listede varsa seç
        ::SendMessage(hWnd, LB_SELECTSTRING, -1, (LPARAM)(LPCTSTR)value);
    }

    CString Get(HWND hWnd) override {
        int sel = (int)::SendMessage(hWnd, LB_GETCURSEL, 0, 0);
        if (sel != LB_ERR) {
            int len = (int)::SendMessage(hWnd, LB_GETTEXTLEN, sel, 0);
            CString str;
            ::SendMessage(hWnd, LB_GETTEXT, sel, (LPARAM)str.GetBuffer(len + 1));
            str.ReleaseBuffer();
            return str;
        }
        return _T("");
    }
};

// =============================================================
//  FACTORY IMPLEMENTATION
// =============================================================

UIFactory::UIFactory() {
    // İşleyicileri öncelik sırasına göre ekle
    RegisterBinder(std::make_shared<DateTimeBinder>());
    RegisterBinder(std::make_shared<ComboBoxBinder>());
    RegisterBinder(std::make_shared<ButtonBinder>()); // Checkbox
    RegisterBinder(std::make_shared<ListBoxBinder>());
    RegisterBinder(std::make_shared<EditBinder>()); // En genel olan en sonda
}

void UIFactory::SetValueToControl(HWND hWnd, const CString& value) {
    if (!hWnd || !::IsWindow(hWnd)) return;
    for (auto& binder : m_binders) {
        if (binder->CanHandle(hWnd)) {
            binder->Set(hWnd, value);
            return;
        }
    }
    // Hiçbiri tanımazsa varsayılan olarak metni bas
    ::SetWindowText(hWnd, value);
}

CString UIFactory::GetValueFromControl(HWND hWnd) {
    if (!hWnd || !::IsWindow(hWnd)) return _T("");
    for (auto& binder : m_binders) {
        if (binder->CanHandle(hWnd)) {
            return binder->Get(hWnd);
        }
    }
    // Hiçbiri tanımazsa metni oku
    int len = ::GetWindowTextLength(hWnd);
    CString str;
    ::GetWindowText(hWnd, str.GetBuffer(len+1), len+1);
    str.ReleaseBuffer();
    return str;
}