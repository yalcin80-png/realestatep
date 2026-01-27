# Test ve Doğrulama Kılavuzu

## CTreeListView Durum Menüsü Test Senaryoları

Bu dokuman, yeni eklenen durum yönetimi özelliklerini test etmek için kullanılır.

## Test Ortamı Hazırlığı

### Gereksinimler
- Windows işletim sistemi
- Visual Studio veya uyumlu C++ derleyici
- Proje kaynak dosyaları
- Test veritabanı

### Derleme
1. Projeyi Visual Studio'da açın
2. Release veya Debug modunda derleyin
3. Hata olmadığından emin olun

## Test Senaryoları

### Test 1: Menü Görünürlüğü
**Amaç**: Yeni menü öğelerinin doğru görüntülendiğini doğrulamak

**Adımlar**:
1. Uygulamayı başlatın
2. TreeListView'da herhangi bir mülk satırına sağ tıklayın
3. "Durum Değiştir" alt menüsüne gidin

**Beklenen Sonuç**:
- Aşağıdaki menü öğeleri görünmeli:
  - ✓ Satıldı (Kırmızı)
  - ✓ Beklemede (Yeşil)
  - ✓ Fiyat Takipte (Sarı)
  - ✓ Durum: Sorunlu (Gri)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 2: "Satıldı" Durumu - Kırmızı Renk
**Amaç**: Satıldı durumunun doğru renkte görüntülendiğini doğrulamak

**Adımlar**:
1. Bir mülk satırına sağ tıklayın
2. Durum Değiştir > Satıldı (Kırmızı) seçeneğini tıklayın

**Beklenen Sonuç**:
- Satır arka planı koyu kırmızı (RGB(220, 50, 50)) olmalı
- Yazı rengi beyaz (RGB(255, 255, 255)) olmalı
- Veritabanında Status alanı "Satıldı" olarak güncellenmiş olmalı

**Kontrol Listesi**:
- [ ] Renk değişimi anında gerçekleşti
- [ ] Arka plan rengi doğru (koyu kırmızı)
- [ ] Yazı rengi doğru (beyaz)
- [ ] Veritabanı güncellendi
- [ ] Okunabilirlik iyi (WCAG AA uyumlu)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 3: "Beklemede" Durumu - Yeşil Renk
**Amaç**: Beklemede durumunun doğru renkte görüntülendiğini doğrulamak

**Adımlar**:
1. Bir mülk satırına sağ tıklayın
2. Durum Değiştir > Beklemede (Yeşil) seçeneğini tıklayın

**Beklenen Sonuç**:
- Satır arka planı koyu yeşil (RGB(0, 128, 0)) olmalı
- Yazı rengi beyaz (RGB(255, 255, 255)) olmalı
- Veritabanında Status alanı "Beklemede" olarak güncellenmiş olmalı

**Kontrol Listesi**:
- [ ] Renk değişimi anında gerçekleşti
- [ ] Arka plan rengi doğru (koyu yeşil)
- [ ] Yazı rengi doğru (beyaz)
- [ ] Veritabanı güncellendi
- [ ] Okunabilirlik iyi (WCAG AA uyumlu)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 4: "Fiyat Takipte" Durumu - Sarı Renk
**Amaç**: Fiyat Takipte durumunun doğru renkte görüntülendiğini doğrulamak

**Adımlar**:
1. Bir mülk satırına sağ tıklayın
2. Durum Değiştir > Fiyat Takipte (Sarı) seçeneğini tıklayın

**Beklenen Sonuç**:
- Satır arka planı koyu sarı/altın (RGB(184, 134, 11)) olmalı
- Yazı rengi siyah (RGB(0, 0, 0)) olmalı
- Veritabanında Status alanı "Fiyat Takipte" olarak güncellenmiş olmalı

**Kontrol Listesi**:
- [ ] Renk değişimi anında gerçekleşti
- [ ] Arka plan rengi doğru (koyu sarı/altın)
- [ ] Yazı rengi doğru (siyah)
- [ ] Veritabanı güncellendi
- [ ] Okunabilirlik iyi (WCAG AA uyumlu)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 5: "Durum: Sorunlu" - Gri Renk
**Amaç**: Sorunlu durumunun doğru renkte görüntülendiğini doğrulamak

**Adımlar**:
1. Bir mülk satırına sağ tıklayın
2. Durum Değiştir > Durum: Sorunlu (Gri) seçeneğini tıklayın

**Beklenen Sonuç**:
- Satır arka planı koyu gri (RGB(128, 128, 128)) olmalı
- Yazı rengi beyaz (RGB(255, 255, 255)) olmalı
- Veritabanında Status alanı "Durum: Sorunlu" olarak güncellenmiş olmalı

**Kontrol Listesi**:
- [ ] Renk değişimi anında gerçekleşti
- [ ] Arka plan rengi doğru (koyu gri)
- [ ] Yazı rengi doğru (beyaz)
- [ ] Veritabanı güncellendi
- [ ] Okunabilirlik iyi (WCAG AA uyumlu)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 6: Gradient Efektler
**Amaç**: Görsel iyileştirmelerin çalıştığını doğrulamak

**Adımlar**:
1. Bir mülk satırını seçin
2. Sağ tarafta görünen butonlar üzerine fareyi getirin (hover)

**Beklenen Sonuç**:
- Edit butonu (✎) üzerine gelindiğinde mavi gradient efekt görünmeli
- Print butonu (🖨) üzerine gelindiğinde yeşil gradient efekt görünmeli
- Butonlar smooth ve professional görünmeli

**Kontrol Listesi**:
- [ ] Edit butonu hover efekti çalışıyor
- [ ] Print butonu hover efekti çalışıyor
- [ ] Gradient geçişleri smooth
- [ ] Performans iyi (gecikme yok)

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 7: Çoklu Mülk Üzerinde Test
**Amaç**: Birden fazla mülkün durumunu değiştirip renklerin doğru göründüğünü test etmek

**Adımlar**:
1. İlk mülkü "Satıldı" yap
2. İkinci mülkü "Beklemede" yap
3. Üçüncü mülkü "Fiyat Takipte" yap
4. Dördüncü mülkü "Sorunlu" yap

**Beklenen Sonuç**:
- Her mülk kendi durumuna uygun renkte görünmeli
- Renkler karışmamalı
- Tüm mülkler okunabilir olmalı

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 8: Veritabanı Persistency
**Amaç**: Durum değişikliklerinin kalıcı olduğunu doğrulamak

**Adımlar**:
1. Bir mülkün durumunu değiştirin (örn: Beklemede)
2. Uygulamayı kapatın
3. Uygulamayı tekrar açın
4. Aynı mülkü bulun

**Beklenen Sonuç**:
- Mülk yine aynı durumda (Beklemede) olmalı
- Renk değişikliği korunmuş olmalı

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 9: Hata Durumu
**Amaç**: Veritabanı güncellenemediğinde hata mesajının gösterildiğini doğrulamak

**Adımlar**:
1. Veritabanı bağlantısını kesmek için gerekli koşulları oluşturun (veya simulation yapın)
2. Bir mülkün durumunu değiştirmeyi deneyin

**Beklenen Sonuç**:
- "Durum güncellenemedi" hata mesajı görünmeli
- Renk değişmemeli
- Uygulama crash olmamalı

**Durum**: [ ] PASS / [ ] FAIL

---

### Test 10: Accessibility (Erişilebilirlik)
**Amaç**: Renk kontrastlarının erişilebilirlik standartlarına uygun olduğunu doğrulamak

**Test Yöntemi**: 
- Online contrast checker kullanın (örn: WebAIM Contrast Checker)
- Veya görme engelli kullanıcılarla test yapın

**Kontrol Edilecek Renkler**:

1. **Satıldı**: Beyaz yazı (#FFFFFF) / Koyu Kırmızı arka plan (#DC3232)
   - Beklenen Kontrast Oranı: ≥ 4.5:1
   - [ ] PASS / [ ] FAIL

2. **Beklemede**: Beyaz yazı (#FFFFFF) / Koyu Yeşil arka plan (#008000)
   - Beklenen Kontrast Oranı: ≥ 4.5:1
   - [ ] PASS / [ ] FAIL

3. **Fiyat Takipte**: Siyah yazı (#000000) / Koyu Sarı arka plan (#B8860B)
   - Beklenen Kontrast Oranı: ≥ 4.5:1
   - [ ] PASS / [ ] FAIL

4. **Sorunlu**: Beyaz yazı (#FFFFFF) / Koyu Gri arka plan (#808080)
   - Beklenen Kontrast Oranı: ≥ 4.5:1
   - [ ] PASS / [ ] FAIL

**Durum**: [ ] PASS / [ ] FAIL

---

## Performans Testi

### Test 11: Büyük Veri Seti
**Amaç**: Çok sayıda kayıt olduğunda performansı test etmek

**Adımlar**:
1. En az 1000 kayıtlı bir veritabanı kullanın
2. Farklı kayıtların durumlarını değiştirin
3. Scroll yapın ve renk değişikliklerini gözlemleyin

**Beklenen Sonuç**:
- Renk değişiklikleri hızlı olmalı (< 1 saniye)
- Scroll performansı düşmemeli
- Memory leak olmamalı

**Durum**: [ ] PASS / [ ] FAIL

---

## Regresyon Testi

### Test 12: Mevcut Fonksiyonalite
**Amaç**: Yeni eklenen özelliklerin mevcut fonksiyonları bozmadığını doğrulamak

**Test Edilecek Özellikler**:
- [ ] Mülk ekleme çalışıyor
- [ ] Mülk silme çalışıyor
- [ ] Mülk düzenleme çalışıyor
- [ ] Arama fonksiyonu çalışıyor
- [ ] Filtreleme çalışıyor
- [ ] Sıralama çalışıyor
- [ ] Diğer sağ tık menü öğeleri çalışıyor

**Durum**: [ ] PASS / [ ] FAIL

---

## Test Sonuç Özeti

| Test No | Test Adı | Durum | Notlar |
|---------|----------|-------|--------|
| Test 1  | Menü Görünürlüğü | ☐ | |
| Test 2  | Satıldı Durumu | ☐ | |
| Test 3  | Beklemede Durumu | ☐ | |
| Test 4  | Fiyat Takipte Durumu | ☐ | |
| Test 5  | Sorunlu Durumu | ☐ | |
| Test 6  | Gradient Efektler | ☐ | |
| Test 7  | Çoklu Mülk | ☐ | |
| Test 8  | DB Persistency | ☐ | |
| Test 9  | Hata Durumu | ☐ | |
| Test 10 | Accessibility | ☐ | |
| Test 11 | Performans | ☐ | |
| Test 12 | Regresyon | ☐ | |

**Toplam Pass**: ___ / 12
**Toplam Fail**: ___ / 12

---

## Bilinen Sorunlar
(Test sırasında bulunan sorunları buraya not edin)

1. 
2. 
3. 

---

## Test Ekibi
- **Tester**: _______________
- **Tarih**: _______________
- **Build/Version**: _______________
- **Test Ortamı**: _______________

---

## Onay
- [ ] Tüm testler başarıyla tamamlandı
- [ ] Critical bug yok
- [ ] Production'a deploy edilebilir

**İmza**: _______________
**Tarih**: _______________
