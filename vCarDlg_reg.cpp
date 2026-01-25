// vCarDlg_reg.cpp
// Car dialog implementasyonu header içerisinde olduğundan, modüler kayıt
// işlemini tek bir translation unit'te yapıyoruz.

#include "stdafx.h"
#include "vCarDlg.h"
#include "PropertyModuleRegistry.h"

// 🧩 MODÜLER KAYIT
REGISTER_PROPERTY_DIALOG(_T("car"), TABLE_NAME_CAR, _T("Araba"), CCarDialog);
