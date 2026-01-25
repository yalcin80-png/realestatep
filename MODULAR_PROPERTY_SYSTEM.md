# Modüler (Lego) Mülk Sistemi

Bu paketle birlikte, yeni bir mülk türünü sisteme eklemek için **core**
dosyalarına dokunmadan yalnızca...

## Yeni mülk türü ekleme (3 parça + 1 satır)

1. `vXxxDlg.h/.cpp` : Dialog sınıfı
   - Ctor imzası: `C...Dlg(DatabaseManager&, DialogMode, const CString& cariKod, const CString& recordCode)`
   - Açılış: `DoModal(parent)`

2. `vXxxDlg.rc` : Resource (isteğe bağlı)

3. `dataIsMe.h/.cpp` : Struct + PropertyMap listesi

4. Tek satır kayıt (Dialog .cpp içinde):

```cpp
#include "PropertyModuleRegistry.h"
REGISTER_PROPERTY_DIALOG(_T("mytype"), _T("MyTable"), _T("Görünen Ad"), CMyDlg);
```

Hepsi bu. Yeni tür, müşteri kartındaki **+ (Hızlı Ekle)** menüsünde otomatik görünür.

## Dinamik komut aralığı

Komutlar çalışma anında `34000..34999` aralığından atanır. `resource.h` değiştirmek gerekmez.
