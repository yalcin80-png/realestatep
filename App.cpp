/////////////////////////////
// App.cpp
//

#include "stdafx.h"

// App.cpp bu projeye dahil edilirse, InitInstance implementasyonu header'daki
// inline versiyonla çakışmasın diye inline'ı kapatıyoruz.
#define REALTYAPP_NO_INLINE_INITINSTANCE
#include "App.h"

// InitInstance implementasyonu App.h içinde inline veriliyor.
