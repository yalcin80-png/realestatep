#pragma once
#ifndef PREVIEW_PANEL_H
#define PREVIEW_PANEL_H

#include "stdafx.h"
#include "Resource.h"
#include "PreviewItem.h" // Veri yapısı (PreviewItem)
#include "ZoomWnd.h"     // Yakınlaştırma penceresi

class CPreviewPanel : public CDialog
{
public:
    //CPreviewPanel() {};
    CPreviewPanel() : CDialog(IDD_PREVIEW_DLG) {}
    virtual ~CPreviewPanel();

    // Veriyi dışarıdan yüklemek için
    void SetPreviewData(const PreviewItem& data);
        CZoomWnd &GetZoomWnd() { return m_zoomWnd; };
    CZoomWnd  m_zoomWnd;

protected:
    virtual BOOL OnInitDialog() override;
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void OnOK() override {}
    virtual void OnCancel() override {}

private:
    // Arayüz Yönetimi
    void CreateToolbar();
    void LayoutChildren();
    void UpdatePreview(); // Ekrana çizim tetikler

    // İşlem Fonksiyonları
    void OnBtnPrevPage();
    void OnBtnNextPage();
    void OnSaveAsPDF();
    void OnSaveAsPNG();
    void OnPrint();

    // Yardımcılar
    HBITMAP GeneratePreviewBitmap(); // Geçerli sayfayı bellekte çizer
    bool    SaveBitmapToPNG(HBITMAP hBmp, const CString& filePath);
private:
    CToolBar  m_ToolBar;
    CImageList m_ImgList;

    // Veriler
    PreviewItem m_data;
    int         m_currentPage;
    int         m_totalPages;
};

#endif // PREVIEW_PANEL_H