#pragma once
#include <string>
#include <vector>
#include <windows.h>

// Windows OCR API'lerini kullanmak için gereken forward declaration'lar
// Bu header'ı MFC projelerinde direkt include etmek bazen çakışma yaratır,
// o yüzden karmaşık WinRT tiplerini cpp dosyasına saklıyoruz.

class OcrHelper
{
public:
    OcrHelper();
    ~OcrHelper();

    // Verilen pencerenin (HWND) görüntüsünü alıp içindeki metinleri satır satır döner
    std::vector<std::wstring> RecognizeTextFromWindow(HWND hWnd);

    // Tüm ekranın veya belirli bir rect'in OCR'ını yapar
    std::vector<std::wstring> RecognizeTextFromScreen(RECT rect);

private:
    // İç yardımcı fonksiyonlar
    HBITMAP CaptureWindow(HWND hWnd);
    HBITMAP CaptureScreen(RECT rect);
};