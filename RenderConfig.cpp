// RenderConfig.cpp
#include "stdafx.h"
#include "RenderConfig.h"

// Varsayılanı istersen GDI_ONLY yap, istersen JSON_PREFERRED.
// Şimdilik güvenli olsun diye GDI_ONLY yapıyorum:
ContractRenderMode g_ContractRenderMode = ContractRenderMode::GDI_ONLY;
