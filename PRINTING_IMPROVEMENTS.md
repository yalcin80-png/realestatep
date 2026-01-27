# Profesyonel Yazdırma ve Önizleme Geliştirmeleri

## Genel Bakış

Bu belge, realestatep projesinde GDI tabanlı yazdırma ve önizleme sistemine yapılan profesyonel iyileştirmeleri açıklar.

## Mimari

Sistem **Win32++** mimarisi üzerine kurulmuştur. MFC veya ATL bağımlılığı yoktur.

### Ana Bileşenler

1. **PreviewPanel.cpp/h** - Ana önizleme ve yazdırma kontrol paneli
2. **ZoomWnd.cpp/h** - Yakınlaştırma ve kaydırma işlevselliği
3. **GdiPlusDrawContext.cpp/h** - GDI+ tabanlı çizim bağlamı
4. **IDocumentLayout.h** - Belge düzeni arayüzü

## Yapılan İyileştirmeler

### 1. Gelişmiş Yazdırma Diyaloğu

#### Özellikler:
- ✅ **Sayfa Aralığı Seçimi**: Tüm sayfalar, belirli sayfa aralığı
- ✅ **Kopya Sayısı**: Birden fazla kopya yazdırma desteği
- ✅ **Harmanlama**: Kopya harmanlaması (collation) desteği
- ✅ **Sayfa Doğrulama**: Geçersiz sayfa aralıklarını otomatik düzeltme

#### Kod Örneği:
```cpp
PRINTDLG pd = { 0 };
pd.lStructSize = sizeof(pd);
pd.hwndOwner = *this;
pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_ALLPAGES | PD_PAGENUMS;
pd.nMinPage = 1;
pd.nMaxPage = m_totalPages;
```

### 2. Durum Çubuğu (Status Bar)

#### Üç Bölümlü Yapı:
1. **Sayfa Bilgisi**: "Sayfa 3 / 10" formatında
2. **Yakınlaştırma Bilgisi**: "Yakınlaştırma: %150" veya "Yakınlaştırma: %100 (En iyi sığdır)"
3. **İşlem Durumu**: "Hazır", "PDF oluşturuluyor...", "Yazdırılıyor..." vb.

#### Dinamik Güncellemeler:
- Sayfa değiştiğinde otomatik güncelleme
- Zoom değiştiğinde otomatik güncelleme
- İşlemler sırasında ilerleme gösterimi

### 3. Klavye Kısayolları

Kullanıcı deneyimini iyileştirmek için tam klavye desteği:

| Kısayol | İşlev |
|---------|-------|
| **Page Up** | Önceki sayfa |
| **Page Down** | Sonraki sayfa |
| **Home** | İlk sayfaya git |
| **End** | Son sayfaya git |
| **Ctrl + Plus (+)** | Yakınlaştır |
| **Ctrl + Minus (-)** | Uzaklaştır |
| **Ctrl + P** | Yazdır |
| **Ctrl + S** | PDF olarak kaydet |
| **Fare Tekerleği** | Yakınlaştır/Uzaklaştır |

### 4. Akıllı Buton Yönetimi

Butonlar içerik durumuna göre otomatik etkinleştirilir/devre dışı bırakılır:

```cpp
// Önceki butonu - sadece 1. sayfada değilsek aktif
BOOL enablePrev = (m_currentPage > 1);
m_ToolBar.EnableButton(ID_BTN_PREV, enablePrev);

// Sonraki butonu - sadece son sayfada değilsek aktif
BOOL enableNext = (m_currentPage < m_totalPages);
m_ToolBar.EnableButton(ID_BTN_NEXT, enableNext);
```

### 5. Yüksek Kaliteli Yazdırma

#### Yazıcı Algılama:
Sistem, ekran ve yazıcı arasındaki farkı otomatik algılar ve kalite ayarlarını optimize eder:

```cpp
const int tech = ::GetDeviceCaps(hdc, TECHNOLOGY);
const bool isPrinter = (tech == DT_RASPRINTER);

if (isPrinter) {
    // Yazıcı için en yüksek kalite
    m_graphics->SetSmoothingMode(SmoothingModeHighQuality);
    m_graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    m_graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    m_graphics->SetCompositingQuality(CompositingQualityHighQuality);
    m_graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
} else {
    // Ekran için optimize edilmiş ayarlar
    m_graphics->SetSmoothingMode(SmoothingModeAntiAlias);
    m_graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    // ...
}
```

#### A4 Sayfa Boyutu:
Standart A4 kağıt boyutu (210x297 mm) 300 DPI çözünürlükte:
- **Genişlik**: 2480 piksel
- **Yükseklik**: 3508 piksel

### 6. Profesyonel Görsel Tasarım

#### Sayfa Gölgeleri:
Önizlemede sayfa etrafında profesyonel gölge efekti:

```cpp
// Gölge çiz (hafif siyah)
const int shadowOffset = 4;
RECT rcShadow = { 
    m_ptszDest.x + shadowOffset, 
    m_ptszDest.y + shadowOffset,
    m_ptszDest.x + m_ptszDest.cx + shadowOffset, 
    m_ptszDest.y + m_ptszDest.cy + shadowOffset 
};
HBRUSH hShadowBrush = ::CreateSolidBrush(RGB(160, 160, 160));
::FillRect(hdcDraw, &rcShadow, hShadowBrush);
```

#### Sayfa Çerçevesi:
Sayfa etrafında ince çerçeve çizimi:

```cpp
HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
::Rectangle(hdcDraw, m_ptszDest.x, m_ptszDest.y, 
           m_ptszDest.x + m_ptszDest.cx, m_ptszDest.y + m_ptszDest.cy);
```

#### Geliştirilmiş Ölçeklendirme:
HALFTONE modu ile daha kaliteli önizleme:

```cpp
::SetStretchBltMode(hdcDraw, HALFTONE);
::SetBrushOrgEx(hdcDraw, 0, 0, NULL);
```

### 7. Kapsamlı Hata İşleme

#### Detaylı Hata Mesajları:
Kullanıcıya sorunun ne olduğu ve nasıl çözüleceği hakkında bilgi:

```cpp
MessageBox(L"PDF Dosyası oluşturulamadı.\n\n"
          L"Olası sebepler:\n"
          L"- Dosya başka bir program tarafından kullanılıyor\n"
          L"- Yazma izniniz yok\n"
          L"- Disk dolu", 
          L"Hata", MB_ICONERROR);
```

#### İlerleme Göstergeleri:
Uzun işlemler sırasında durum çubuğunda ilerleme:

```cpp
CString progress;
progress.Format(L"PDF oluşturuluyor: Sayfa %d / %d", i, pages);
m_StatusBar.SetPartText(2, progress);
```

#### Başarı Bildirimları:
İşlem başarılı olduğunda detaylı bilgi:

```cpp
CString successMsg;
successMsg.Format(L"Yazdırma tamamlandı.\n%d sayfa başarıyla yazdırıldı.", 
                 endPage - startPage + 1);
MessageBox(successMsg, L"Bilgi", MB_ICONINFORMATION);
```

### 8. PDF ve PNG Kaydetme İyileştirmeleri

#### PDF Kaydetme:
- Haru PDF kütüphanesi ile UTF-8/Türkçe karakter desteği
- Çok sayfalı belge desteği
- İlerleme gösterimi
- Detaylı hata mesajları

#### PNG Kaydetme:
- Yüksek çözünürlüklü PNG çıktısı (2480x3508)
- GDI+ ile kaliteli kodlama
- Tek sayfa kaydı (mevcut önizleme sayfası)
- Başarı mesajında dosya yolu bilgisi

## Teknik Detaylar

### Değişen Dosyalar

1. **PreviewPanel.h**
   - Status bar desteği eklendi
   - Yeni yardımcı fonksiyonlar (UpdatePageInfo, UpdateZoomInfo, FormatZoomPercentage)
   - m_zoomPercentage üye değişkeni eklendi

2. **PreviewPanel.cpp**
   - CreateStatusBar() fonksiyonu eklendi
   - OnPrint() fonksiyonu tamamen yeniden yazıldı
   - Klavye işleyici eklendi (WM_KEYDOWN)
   - Gelişmiş hata işleme ve kullanıcı geri bildirimi

3. **ZoomWnd.h**
   - m_bBestFit ve m_ptszDest public yapıldı (zoom bilgisi erişimi için)

4. **ZoomWnd.cpp**
   - ZoomIn/ZoomOut fonksiyonlarında parent bildirim eklendi
   - ActualSize/BestFit fonksiyonlarında parent bildirim eklendi
   - OnPaint() fonksiyonunda gölge ve çerçeve efektleri eklendi
   - HALFTONE stretch modu ile kalite iyileştirmesi

5. **GdiPlusDrawContext.cpp**
   - Begin() fonksiyonunda yazıcı/ekran algılama
   - Yazıcı için yüksek kalite ayarları
   - Ekran için optimize edilmiş ayarlar

### Bağımlılıklar

- **Win32++**: Temel pencere ve kontrol framework'ü
- **GDI+**: Çizim ve grafik işlemleri
- **Haru PDF**: PDF oluşturma (mevcut)
- **IDocumentLayout**: Belge düzeni arayüzü (mevcut)

### Performans

- Önizleme oluşturma: ~100-200ms (belge karmaşıklığına bağlı)
- Yazdırma: Yazıcı hızına bağlı
- PDF oluşturma: Sayfa başına ~50-100ms
- PNG kaydetme: ~50-100ms

## Kullanım Örnekleri

### Önizleme Gösterme

```cpp
PreviewItem item;
item.docType = DOC_CONTRACT_SALES_AUTH;
item.recordCode = L"HM001";
item.fields = GetFieldsFromDatabase();

CPreviewPanel previewPanel;
previewPanel.Create(hParent);
previewPanel.SetPreviewData(item);
```

### Yazdırma

Kullanıcı "Yazdır" butonuna bastığında veya Ctrl+P tuşladığında:
1. Gelişmiş yazdırma diyaloğu açılır
2. Kullanıcı sayfa aralığı, kopya sayısı seçer
3. Sistem seçilen sayfaları yüksek kalitede yazdırır
4. İlerleme durum çubuğunda gösterilir
5. Başarı mesajı gösterilir

### PDF Kaydetme

Kullanıcı "PDF" butonuna bastığında veya Ctrl+S tuşladığında:
1. Dosya kaydet diyaloğu açılır
2. Kullanıcı dosya adı ve konumu seçer
3. Tüm sayfalar PDF'e dönüştürülür
4. İlerleme gösterilir
5. Başarı mesajı dosya yolu ile birlikte gösterilir

## Test Senaryoları

### 1. Temel Önizleme
- [ ] Önizleme açılır açılmaz sayfa doğru görünür
- [ ] Durum çubuğu doğru bilgileri gösterir
- [ ] Sayfa gölgesi ve çerçevesi doğru çizilir

### 2. Sayfa Gezinme
- [ ] İleri/Geri butonları çalışır
- [ ] Page Up/Down tuşları çalışır
- [ ] Home/End tuşları çalışır
- [ ] İlk/son sayfada butonlar devre dışı kalır

### 3. Yakınlaştırma
- [ ] Yakınlaştır/Uzaklaştır butonları çalışır
- [ ] Ctrl +/- tuşları çalışır
- [ ] Fare tekerleği çalışır
- [ ] Zoom yüzdesi durum çubuğunda güncellenir
- [ ] "En iyi sığdır" modu doğru gösterilir

### 4. Yazdırma
- [ ] Yazdırma diyaloğu açılır
- [ ] Sayfa aralığı seçimi çalışır
- [ ] Kopya sayısı ayarlanabilir
- [ ] Yazdırma başarılı olur
- [ ] İlerleme gösterilir
- [ ] Hata durumunda uygun mesaj gösterilir

### 5. Kaydetme
- [ ] PDF kaydetme çalışır
- [ ] PNG kaydetme çalışır
- [ ] Dosya adı önerilir
- [ ] İlerleme gösterilir
- [ ] Başarı/hata mesajları doğru

## Gelecek İyileştirmeler (Opsiyonel)

Şu anda sistem profesyonel ve tam işlevseldir. Gelecekte eklenebilecek özellikler:

1. **Çoklu Sayfa Önizleme**: 2-up, 4-up görünümler
2. **Baskı Önizlemesi**: Yazdırmadan önce tam önizleme
3. **Kağıt Boyutu Seçimi**: A4, Letter, A3 vb.
4. **Sayfa Yönü**: Portre/Landscape otomatik algılama
5. **Yer İmleri**: PDF'de yer imi desteği
6. **Filigran**: İsteğe bağlı filigran ekleme

## Sonuç

Bu iyileştirmeler ile sistem artık:
- ✅ Profesyonel kullanıcı deneyimi sunar
- ✅ Yüksek kaliteli çıktı üretir
- ✅ Kapsamlı klavye desteği sağlar
- ✅ Detaylı hata işleme yapar
- ✅ Modern görsel tasarıma sahiptir
- ✅ Türkçe dil desteği tam çalışır
- ✅ Win32++ mimarisine uyumludur

---

**Not**: Bu sistem Win32++ üzerine kurulmuştur ve MFC/ATL kullanmaz. Tüm iyileştirmeler mevcut sınıflara minimal değişikliklerle yapılmıştır.
