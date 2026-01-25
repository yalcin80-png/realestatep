#pragma once
#ifndef GDI_LOCATION_FORM_LAYOUT_H
#define GDI_LOCATION_FORM_LAYOUT_H

#include "IDocumentLayout.h"
#include "ThemeConfig.h"
#include <map>
#include <vector>

// İsim Değişikliği: GdiShowingLayout -> GdiLocationFormLayout
class GdiLocationFormLayout : public IDocumentLayout
{
public:
    GdiLocationFormLayout() {}
    virtual ~GdiLocationFormLayout() {}

    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual int  GetTotalPages() const override { return 1; }
    virtual void Render(IDrawContext& ctx, int pageNo) override;

private:
    std::map<CString, CString> m_data;

    // Yardımcı: Veriyi güvenli çek
    CString GetVal(const CString& key) {
        auto it = m_data.find(key);
        if (it != m_data.end()) return it->second;
        return _T("");
    }

    // Yardımcı: Font Oluşturucu
    FontDesc MakeFont(float size, bool bold = false, bool italic = false) {
        return FontDesc(L"Arial", size, bold, italic);
    }
};

#endif