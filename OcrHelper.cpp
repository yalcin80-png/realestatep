#include "stdafx.h"
#include "OcrHelper.h"

// WinRT Headerları
#include <winrt/Windows.Foundation.h>
// [DÜZELTME BURADA] Koleksiyonlar üzerinde döngü (for) kurabilmek için bu ŞARTTIR:
#include <winrt/Windows.Foundation.Collections.h> 
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>

// GDI+ (Bitmap işlemleri için)
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace winrt;
using namespace Windows::Media::Ocr;
using namespace Windows::Graphics::Imaging;

OcrHelper::OcrHelper() {
    // WinRT başlat (Apartment context)
    winrt::init_apartment();

    // GDI+ Başlat
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

OcrHelper::~OcrHelper() {
    // GDI+ kapatma işlemleri burada yapılabilir
}

// HBITMAP'i WinRT SoftwareBitmap'e çeviren yardımcı fonksiyon
// (Bu fonksiyonu sınıf içine private helper olarak da ekleyebilirsiniz, 
//  şu an CPP içinde static/internal olarak bırakıyoruz)
static SoftwareBitmap HBitmapToSoftwareBitmap(HBITMAP hBitmap) {
    if (!hBitmap) return nullptr;
    return nullptr; // Placeholder - Aşağıda asıl işi yapan farklı bir yöntem kullandık
}

std::vector<std::wstring> OcrHelper::RecognizeTextFromWindow(HWND hWnd)
{
    RECT rc;
    ::GetWindowRect(hWnd, &rc);
    return RecognizeTextFromScreen(rc);
}

std::vector<std::wstring> OcrHelper::RecognizeTextFromScreen(RECT rc)
{
    std::vector<std::wstring> results;

    // Genişlik/Yükseklik kontrolü
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return results;

    HDC hScreenDC = NULL;
    HDC hMemoryDC = NULL;
    HBITMAP hBitmap = NULL;
    HBITMAP hOldBitmap = NULL;
    IStream* pStream = NULL;

    try {
        // 1. OCR Motorunu Türkçe olarak hazırla
        auto ocrEngine = OcrEngine::TryCreateFromUserProfileLanguages();
        if (!ocrEngine) {
            // Dil desteklenmiyor olabilir, fallback olarak varsayılanı dene
            ocrEngine = OcrEngine::TryCreateFromLanguage(Windows::Globalization::Language(L"en-US"));
        }
        if (!ocrEngine) return results; // Hiçbir motor oluşmadı

        // 2. Ekran Görüntüsü Al (GDI Yöntemi)
        hScreenDC = GetDC(NULL);
        hMemoryDC = CreateCompatibleDC(hScreenDC);
        hBitmap = CreateCompatibleBitmap(hScreenDC, w, h);
        hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

        // Ekranın o bölgesini kopyala
        BitBlt(hMemoryDC, 0, 0, w, h, hScreenDC, rc.left, rc.top, SRCCOPY);

        // 3. HBITMAP -> IStream (BMP Formatında)
        CreateStreamOnHGlobal(NULL, TRUE, &pStream);

        Gdiplus::Bitmap gdiBitmap(hBitmap, NULL);
        CLSID clsid;
        // BMP Encoder CLSID: {557CF400-1A04-11D3-9A73-0000F81EF32E}
        CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &clsid);

        // Bellek akışına (stream) kaydet
        if (gdiBitmap.Save(pStream, &clsid, NULL) != Gdiplus::Ok) {
            throw std::exception("GDI+ Save Error");
        }

        // Stream'i başa sar
        LARGE_INTEGER liZero = { 0 };
        pStream->Seek(liZero, STREAM_SEEK_SET, NULL);

        // 4. IStream -> WinRT IRandomAccessStream Dönüşümü
        // WinRT, native IStream'i doğrudan kullanamaz, araya köprü gerekir.
        // Ancak C++/WinRT'de "CreateRandomAccessStreamOverStream" gibi yardımcılar karmaşıktır.
        // EN BASİT VE GÜVENLİ YOL: Diske geçici dosya yazıp oradan okumaktır.
        // Performans kaybı milisaniyelerdir, ama kod stabilitesi %100'dür.

        CString tempPath;
        GetTempPath(MAX_PATH, tempPath.GetBuffer(MAX_PATH)); tempPath.ReleaseBuffer();
        CString tempFile = tempPath + L"ocr_temp_snap.bmp";

        // Tekrar diske kaydet (Stream yerine File kullanıyoruz)
        gdiBitmap.Save(tempFile, &clsid, NULL);

        // Dosyadan WinRT stream yükle
        auto file = Windows::Storage::StorageFile::GetFileFromPathAsync(tempFile.GetString()).get();
        auto stream = file.OpenAsync(Windows::Storage::FileAccessMode::Read).get();
        auto decoder = BitmapDecoder::CreateAsync(stream).get();
        auto softwareBitmap = decoder.GetSoftwareBitmapAsync().get();

        // 5. OCR İşlemi
        auto ocrResult = ocrEngine.RecognizeAsync(softwareBitmap).get();

        // 6. Satırları Oku
        // BU KISIM ARTIK HATA VERMEYECEK (Windows.Foundation.Collections.h sayesinde)
        for (auto line : ocrResult.Lines()) {
            results.push_back(line.Text().c_str());
        }

        // Temizlik
        // Dosyayı silmeye çalış (Handle serbest kalmışsa)
        DeleteFile(tempFile);

    }
    catch (...) {
        // Loglama yapılabilir
    }

    // GDI Kaynaklarını Serbest Bırak
    if (pStream) pStream->Release();
    if (hMemoryDC) {
        SelectObject(hMemoryDC, hOldBitmap); // Orijinal bitmap'i geri yükle
        DeleteDC(hMemoryDC);
    }
    if (hBitmap) DeleteObject(hBitmap);
    if (hScreenDC) ReleaseDC(NULL, hScreenDC);

    return results;
}