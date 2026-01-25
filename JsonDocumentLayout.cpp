#include "stdafx.h"
#include "JsonDocumentLayout.h"
#include "StringHelpers.h"

using namespace FormEngine;

JsonDocumentLayout::JsonDocumentLayout(const CString& jsonRelativePath)
{
    // Tam yolu oluşturma işini TemplateLoader yapıyor ama burada da tutabiliriz.
    // Biz sadece göreceli yolu saklayalım, renderer halletsin.
    m_jsonPath = jsonRelativePath;
}

JsonDocumentLayout::~JsonDocumentLayout()
{
}

void JsonDocumentLayout::SetData(const std::vector<std::pair<CString, CString>>& fields)
{
    m_data.clear();
    // Verileri Map'e aktar
    for (const auto& kv : fields) {
        m_data[kv.first] = kv.second;
    }
}

int JsonDocumentLayout::GetTotalPages() const
{
    return 1; // Şimdilik 1 sayfa
}

void JsonDocumentLayout::Render(IDrawContext& ctx, int pageNo)
{
    // Yolu burada veriyoruz, TemplateLoader exe yolunu kendi bulup ekliyor
    m_renderer.Render(ctx, m_jsonPath, m_data);
}