#ifndef DOC_PDFDOCUMENTWRITER_H
#define DOC_PDFDOCUMENTWRITER_H

#pragma once

// Yeni çizim motorumuz
#include "PodofoDrawContext.h"
#include "PreviewItem.h" // Enum (PreviewDocType) tanýmý
#include <vector>
#include <utility>

class PdfDocumentWriter
{
public:
    PdfDocumentWriter();
    ~PdfDocumentWriter();

    // PDF Dosyasýný oluþturur
    bool Create(const CString& filePath);

    // Sayfa/Sayfalar ekler (Otomatik Tasarým Seçimi ile)
    bool AddPage(PreviewDocType docType,
        const CString& tableName,
        const CString& title,
        const std::vector<std::pair<CString, CString>>& fields);

    // Dosyayý diske yazar ve kapatýr
    bool Save();

private:
    CString           m_filePath;
    PodofoDrawContext m_ctx; // Artýk PoDoFo kullanýyoruz
    bool              m_isOpen = false;
};

#endif