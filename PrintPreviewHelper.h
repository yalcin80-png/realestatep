#pragma once
#ifndef PRINTPREVIEWHELPER_H
#define PRINTPREVIEWHELPER_H

#include "stdafx.h"
#include "GdiPlusDrawContext.h"
#include <memory>

// ============================================================================
// PRINTPREVIEWHELPER: Print Preview and Printing Utilities
// ============================================================================
// This class provides helper functions for print preview and printing,
// including printer DC handling, page setup, and error recovery.
// ============================================================================

class PrintPreviewHelper
{
public:
    // Get printer DC capabilities
    struct PrinterInfo
    {
        int dpiX;
        int dpiY;
        int pageWidth;      // Physical page width in pixels
        int pageHeight;     // Physical page height in pixels
        int printableWidth; // Printable area width
        int printableHeight;// Printable area height
        int offsetX;        // Left margin
        int offsetY;        // Top margin
        bool isValid;
        
        PrinterInfo() : dpiX(0), dpiY(0), pageWidth(0), pageHeight(0),
                       printableWidth(0), printableHeight(0),
                       offsetX(0), offsetY(0), isValid(false) {}
    };
    
    // Get printer device capabilities
    static PrinterInfo GetPrinterInfo(HDC hPrinterDC);
    
    // Calculate scale factor to fit logical page on physical page
    static void CalculateScaleFactor(
        double logicalWidth, double logicalHeight,
        int physicalWidth, int physicalHeight,
        double& outScaleX, double& outScaleY);
    
    // Setup GdiPlusDrawContext for printing
    static bool SetupPrintContext(
        GdiPlusDrawContext& ctx,
        HDC hPrinterDC,
        double logicalPageWidth,
        double logicalPageHeight);
    
    // Error message formatting
    static CString FormatPrintError(DWORD errorCode);
    
    // Validate printer DC before use
    static bool ValidatePrinterDC(HDC hPrinterDC);
};

// ============================================================================
// IMPLEMENTATION
// ============================================================================

inline PrintPreviewHelper::PrinterInfo PrintPreviewHelper::GetPrinterInfo(HDC hPrinterDC)
{
    PrinterInfo info;
    
    if (!hPrinterDC)
        return info;
    
    // Get device capabilities
    info.dpiX = ::GetDeviceCaps(hPrinterDC, LOGPIXELSX);
    info.dpiY = ::GetDeviceCaps(hPrinterDC, LOGPIXELSY);
    
    // Physical page size
    info.pageWidth = ::GetDeviceCaps(hPrinterDC, PHYSICALWIDTH);
    info.pageHeight = ::GetDeviceCaps(hPrinterDC, PHYSICALHEIGHT);
    
    // Printable area
    info.printableWidth = ::GetDeviceCaps(hPrinterDC, HORZRES);
    info.printableHeight = ::GetDeviceCaps(hPrinterDC, VERTRES);
    
    // Physical offsets (margins)
    info.offsetX = ::GetDeviceCaps(hPrinterDC, PHYSICALOFFSETX);
    info.offsetY = ::GetDeviceCaps(hPrinterDC, PHYSICALOFFSETY);
    
    info.isValid = (info.pageWidth > 0 && info.pageHeight > 0);
    
    return info;
}

inline void PrintPreviewHelper::CalculateScaleFactor(
    double logicalWidth, double logicalHeight,
    int physicalWidth, int physicalHeight,
    double& outScaleX, double& outScaleY)
{
    if (logicalWidth <= 0 || logicalHeight <= 0 ||
        physicalWidth <= 0 || physicalHeight <= 0)
    {
        outScaleX = outScaleY = 1.0;
        return;
    }
    
    double scaleX = (double)physicalWidth / logicalWidth;
    double scaleY = (double)physicalHeight / logicalHeight;
    
    // Use uniform scaling (maintain aspect ratio)
    double scale = (scaleX < scaleY) ? scaleX : scaleY;
    
    outScaleX = outScaleY = scale;
}

inline bool PrintPreviewHelper::SetupPrintContext(
    GdiPlusDrawContext& ctx,
    HDC hPrinterDC,
    double logicalPageWidth,
    double logicalPageHeight)
{
    if (!ValidatePrinterDC(hPrinterDC))
        return false;
    
    // Begin drawing context
    if (!ctx.Begin(hPrinterDC))
        return false;
    
    // Set logical page size (this handles scaling internally)
    ctx.SetLogicalPageSize(logicalPageWidth, logicalPageHeight);
    
    return true;
}

inline CString PrintPreviewHelper::FormatPrintError(DWORD errorCode)
{
    CString errorMsg;
    
    if (errorCode == 0)
    {
        errorMsg = _T("Bilinmeyen yazdırma hatası.");
        return errorMsg;
    }
    
    // Get system error message
    LPVOID lpMsgBuf = nullptr;
    ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);
    
    if (lpMsgBuf)
    {
        errorMsg.Format(_T("Yazdırma hatası (Kod: %d): %s"), errorCode, (LPCTSTR)lpMsgBuf);
        ::LocalFree(lpMsgBuf);
    }
    else
    {
        errorMsg.Format(_T("Yazdırma hatası (Kod: %d)"), errorCode);
    }
    
    return errorMsg;
}

inline bool PrintPreviewHelper::ValidatePrinterDC(HDC hPrinterDC)
{
    if (!hPrinterDC)
        return false;
    
    // Check if it's a valid DC
    int tech = ::GetDeviceCaps(hPrinterDC, TECHNOLOGY);
    
    // Validate it's a printer device
    if (tech != DT_RASPRINTER)
    {
        // Not a raster printer - could be other type, but typically we expect raster
        // Still allow it to proceed
    }
    
    // Check basic capabilities
    int width = ::GetDeviceCaps(hPrinterDC, HORZRES);
    int height = ::GetDeviceCaps(hPrinterDC, VERTRES);
    
    return (width > 0 && height > 0);
}

#endif // PRINTPREVIEWHELPER_H
