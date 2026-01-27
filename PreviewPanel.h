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
    void CreateStatusBar();
    void LayoutChildren();
    void UpdatePreview(); // Ekrana çizim tetikler
    void UpdatePageInfo(); // Sayfa bilgisini günceller
    void UpdateZoomInfo(); // Zoom bilgisini günceller

    // İşlem Fonksiyonları
    void OnBtnPrevPage();
    void OnBtnNextPage();
    void OnSaveAsPDF();
    void OnSaveAsPNG();
    void OnPrint();
    void OnPrintAdvanced(); // Gelişmiş yazdırma seçenekleri

    // Yardımcılar
    HBITMAP GeneratePreviewBitmap(); // Geçerli sayfayı bellekte çizer
    bool    SaveBitmapToPNG(HBITMAP hBmp, const CString& filePath);
    CString FormatZoomPercentage() const; // Zoom yüzdesini formatlar
private:
    CToolBar  m_ToolBar;
    CStatusBar m_StatusBar;
    CImageList m_ImgList;

    // Veriler
    PreviewItem m_data;
    int         m_currentPage;
    int         m_totalPages;
    double      m_zoomPercentage; // Zoom yüzdesi (100.0 = %100)
};

#endif // PREVIEW_PANEL_H