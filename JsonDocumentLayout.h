#pragma once
#ifndef JSON_DOCUMENT_LAYOUT_H
#define JSON_DOCUMENT_LAYOUT_H

#include "IDocumentLayout.h"
#include "JsonFormRenderer.h"
#include <map>
#include <vector>

class JsonDocumentLayout : public IDocumentLayout
{
public:
    // Kurucu sadece dosya yolunu alır
    JsonDocumentLayout(const CString& jsonRelativePath);
    virtual ~JsonDocumentLayout();

    virtual void SetData(const std::vector<std::pair<CString, CString>>& fields) override;
    virtual int  GetTotalPages() const override;
    virtual void Render(IDrawContext& ctx, int pageNo) override;

private:
    JsonFormRenderer m_renderer;
    std::map<CString, CString> m_data;
    CString m_jsonPath;
};

#endif