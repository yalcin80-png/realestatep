#pragma once
#ifndef DOCUMENTDATA_MANAGER_H
#define DOCUMENTDATA_MANAGER_H

#include "stdafx.h"
#include "PreviewItem.h" // PreviewItem struct burada tanımlı olmalı
#include "dataIsMe.h"    // Veritabanı yapıları

class DocumentDataManager
{
public:
    // Ana fonksiyon: İstenilen belge tipi, tablo ve kayıt kodu ile veriyi hazırlar.
    static PreviewItem PrepareDocument(PreviewDocType docType, const CString& tableName, const CString& recordCode);

    // --- Formatlayıcılar ---
    // Not: Bu fonksiyonlar doküman hazırlama sırasında hem sınıf içinden hem de
    // .cpp içindeki yardımcı (anon namespace) fonksiyonlardan çağrılabiliyor.
    // Bu yüzden public tutuluyor.
    static CString FormatMoney(const CString& rawPrice);
    static CString FormatBool(const CString& val); // "1" -> "Var", "0" -> "Yok"
    static CString GetDateNow();                   // Bugünün tarihi (DD.MM.YYYY)
    static CString CleanText(const CString& text); // Boşsa "-" döner

private:
    // --- Yardımcı Yükleyiciler ---
    static void LoadAgentInfo(PreviewItem& item);

    // Mülk bilgilerini yükler ve Mülk Sahibinin Cari Kodunu döner
    static CString LoadPropertyInfo(PreviewItem& item, const CString& table, const CString& code);

    // Mülk sahibini yükler
    static void LoadOwnerInfo(PreviewItem& item, const CString& cariKod);
};

#endif