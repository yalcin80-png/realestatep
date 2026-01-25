#include "stdafx.h"
#include "PdfDocumentWriter.h"
#include "LayoutFactory.h" // Tasarým Fabrikasý
#include "ThemeConfig.h"   // Renk ve Boyutlar
#include <memory>          // std::unique_ptr

PdfDocumentWriter::PdfDocumentWriter()
{
}

PdfDocumentWriter::~PdfDocumentWriter()
{
    // Eðer dosya hala açýksa (Save çaðrýlmadýysa) kapatmaya çalýþ
    if (m_isOpen)
    {
        m_ctx.EndDocument();
        m_isOpen = false;
    }
}

bool PdfDocumentWriter::Create(const CString& filePath)
{
    m_filePath = filePath;

    // Belgeyi baþlat (PoDoFo)
    if (!m_ctx.BeginDocument(filePath))
        return false;

    m_isOpen = true;
    return true;
}

bool PdfDocumentWriter::AddPage(PreviewDocType docType,
    const CString& tableName,
    const CString& title,
    const std::vector<std::pair<CString, CString>>& fields)
{
    if (!m_isOpen)
        return false;

    // 1. FABRÝKADAN UYGUN RESSAMI (LAYOUT) ÝSTE
    std::unique_ptr<IDocumentLayout> layout(LayoutFactory::CreateLayout(docType));

    if (!layout) return false;

    // 2. VERÝLERÝ HAZIRLA
    // Liste tasarýmý için TableName ve Title bilgisini verilerin arasýna ekliyoruz
    // (Layout sýnýfý bunlarý ayýklamayý biliyor)
    std::vector<std::pair<CString, CString>> localFields = fields;
    localFields.push_back({ _T("TableName"), tableName });
    localFields.push_back({ _T("DocTitle"), title });

    layout->SetData(localFields);

    // 3. SAYFA SAYISINI ÖÐREN
    int totalPages = layout->GetTotalPages();
    if (totalPages < 1) totalPages = 1;

    // 4. ÇOKLU SAYFA DÖNGÜSÜ
    for (int i = 1; i <= totalPages; ++i)
    {
        // NOT: Render fonksiyonu içinde "SetLogicalPageSize" çaðrýlýr.
        // Modern DrawContext yapýmýzda bu çaðrý, gerekirse yeni bir sayfa açar
        // ve boyutlarý A4 olarak ayarlar.

        layout->Render(m_ctx, i);
    }

    return true;
}

bool PdfDocumentWriter::Save()
{
    if (!m_isOpen)
        return false;

    // Belgeyi sonlandýr ve diske yaz
    bool ok = m_ctx.EndDocument();

    m_isOpen = false;
    return ok;
}