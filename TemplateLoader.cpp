#include "stdafx.h"
#include "TemplateLoader.h"
#include "StringHelpers.h" 
#include "ThemeConfig.h"
#include "json.hpp"
#include <vector>
#include <string>
#include <fstream>

using json = nlohmann::json;
using namespace FormEngine;

// =========================================================
// YARDIMCI: BYTE KONTROLÜ (UTF-8 Mİ?)
// =========================================================
static bool IsValidUtf8(const std::string& str) {
    const unsigned char* bytes = (const unsigned char*)str.c_str();
    unsigned int codepoint;
    while (*bytes) {
        if ((bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D || (0x20 <= bytes[0] && bytes[0] <= 0x7E))) {
            bytes += 1;
            continue;
        }
        if ((bytes[0] & 0x80) == 0) { // ASCII
            bytes += 1;
            continue;
        }
        if ((bytes[0] & 0xE0) == 0xC0) { // 2-byte
            if ((bytes[1] & 0xC0) != 0x80) return false;
            bytes += 2;
            continue;
        }
        if ((bytes[0] & 0xF0) == 0xE0) { // 3-byte
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return false;
            bytes += 3;
            continue;
        }
        if ((bytes[0] & 0xF8) == 0xF0) { // 4-byte
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return false;
            bytes += 4;
            continue;
        }
        return false; // Geçersiz UTF-8 byte'ı
    }
    return true;
}

// =========================================================
// YARDIMCI: ANSI (CP1254 - Türkçe) -> UTF-8 ÇEVİRİCİ
// =========================================================
static std::string ConvertAnsiToUtf8(const std::string& ansiContent)
{
    int wLen = ::MultiByteToWideChar(1254, 0, ansiContent.c_str(), -1, NULL, 0);
    if (wLen <= 0) return "";
    std::vector<wchar_t> wBuf(wLen);
    ::MultiByteToWideChar(1254, 0, ansiContent.c_str(), -1, wBuf.data(), wLen);

    int uLen = ::WideCharToMultiByte(CP_UTF8, 0, wBuf.data(), -1, NULL, 0, NULL, NULL);
    if (uLen <= 0) return "";
    std::vector<char> uBuf(uLen);
    ::WideCharToMultiByte(CP_UTF8, 0, wBuf.data(), -1, uBuf.data(), uLen, NULL, NULL);

    return std::string(uBuf.data());
}

// =========================================================
// GÜVENLİ DOSYA OKUMA
// =========================================================
static std::string ReadFileSmart(const CString& path) {
    HANDLE hFile = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return "";

    DWORD fileSize = ::GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) { ::CloseHandle(hFile); return ""; }

    std::vector<char> buffer(fileSize + 1);
    DWORD bytesRead = 0;
    if (!::ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL)) { ::CloseHandle(hFile); return ""; }
    buffer[bytesRead] = '\0';
    ::CloseHandle(hFile);

    std::string content = buffer.data();

    // 1. BOM Temizliği (Dosya başındaki gizli karakterler)
    if (content.size() >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
        content = content.substr(3);
    }

    // 2. Encoding Kontrolü
    // Eğer dosya geçerli UTF-8 değilse (yani Türkçe karakterler ANSI ise), çevir.
    if (!IsValidUtf8(content)) {
        return ConvertAnsiToUtf8(content);
    }

    return content;
}

// =========================================================
// PARSER (Değişmedi, sadece string tipini CString'e çeviren yardımcılar var)
// =========================================================
static int GetInt(const json& j, const char* key, int defVal = 0) {
    if (!j.contains(key)) return defVal;
    const auto& val = j[key];
    if (val.is_number()) return val.get<int>();
    if (val.is_string()) { try { return std::stoi(val.get<std::string>()); } catch (...) {} }
    return defVal;
}

static std::string GetString(const json& j, const char* key) {
    if (!j.contains(key)) return "";
    const auto& val = j[key];
    if (val.is_string()) return val.get<std::string>();
    return "";
}

static JElement ParseJsonElement(const json& j) {
    JElement el;
    std::string type = GetString(j, "type");

    if (type == "text") el.type = JElementType::Text;
    else if (type == "field") el.type = JElementType::Field;
    else if (type == "group") el.type = JElementType::Group;
    else if (type == "rect") el.type = JElementType::Rect;
    else if (type == "line") el.type = JElementType::Line;
    else if (type == "image") el.type = JElementType::Image;
    else if (type == "checkbox") el.type = JElementType::Checkbox;
    else if (type == "section") el.type = JElementType::Section;
    else if (type == "row") el.type = JElementType::Row;
    else el.type = JElementType::Text; // Varsayılan

    el.x = GetInt(j, "x", 0);
    el.y = GetInt(j, "y", 0);
    el.w = GetInt(j, "w", 0);
    el.h = GetInt(j, "h", 0);

    el.text = ToCString(GetString(j, "text"));
    el.bindKey = ToCString(GetString(j, "bind"));
    el.imagePath = ToCString(GetString(j, "path"));

    std::string font = GetString(j, "font");
    el.fontId = ToCString(font.empty() ? "Default" : font);

    std::string c = GetString(j, "color");
    el.color = ToCString(c.empty() ? "Black" : c);
    std::string tc = GetString(j, "textColor");
    el.textColor = ToCString(tc);

    std::string a = GetString(j, "align");
    if (a == "center") el.align = JAlign::Center;
    else if (a == "right") el.align = JAlign::Right;
    else el.align = JAlign::Left;

    el.border = j.value("border", false);
    el.filled = j.value("filled", false);
    el.multiline = j.value("multiline", false);
    el.gap = GetInt(j, "gap", 0);

    std::string layout = GetString(j, "layout");
    el.layout = (layout == "vertical") ? JLayoutMode::Vertical : JLayoutMode::Absolute;

    if (j.contains("children") && j["children"].is_array()) {
        for (const auto& child : j["children"]) el.children.push_back(ParseJsonElement(child));
    }
    return el;
}

// ------------------------------
// TEMPLATE LOADER
// ------------------------------
TemplateLoader::TemplateLoader() {}
TemplateLoader::~TemplateLoader() {}

JTemplate TemplateLoader::Load(const CString& filePath)
{
    JTemplate tmpl;
    CString fullPath = FormEngine::GetExePath() + _T("\\") + filePath;

    // 1. Dosyayı Akıllı Oku (UTF-8'e çevirir)
    std::string jsonContent = ReadFileSmart(fullPath);

    if (jsonContent.empty()) {
        CString msg; msg.Format(_T("Şablon dosyası boş veya bulunamadı:\n%s"), fullPath);
        ::MessageBox(NULL, msg, _T("Dosya Hatası"), MB_ICONERROR);
        return tmpl;
    }

    json j;
    try {
        // 2. Parse Etmeyi Dene
        j = json::parse(jsonContent);
    }
    catch (json::parse_error& e) {
        // Hata varsa tam yerini göster (Satır/Sütun)
        CString err;
        err.Format(_T("JSON Hatası (byte %d):\n%S\n\nLütfen JSON dosyasındaki virgülleri ve parantezleri kontrol edin."), e.byte, e.what());
        ::MessageBox(NULL, err, _T("Şablon Hatası"), MB_ICONERROR);
        return tmpl;
    }

    // --- FONTS ---
    if (j.contains("fonts")) {
        for (auto& [key, val] : j["fonts"].items()) {
            std::wstring name = ToWString(key);
            std::wstring family = ToWString(GetString(val, "family"));
            float size = (float)GetInt(val, "size", 10);
            bool bold = val.value("bold", false);
            bool italic = val.value("italic", false);
            tmpl.fonts[name] = FontDesc(family, size, bold, italic);
        }
    }

    // --- PAGES ---
    if (j.contains("pages")) {
        for (auto& p : j["pages"]) {
            JPageDef page;
            page.width = GetInt(p, "width", 2100);
            page.height = GetInt(p, "height", 2970);
            if (p.contains("elements")) {
                for (auto& e : p["elements"]) page.elements.push_back(ParseJsonElement(e));
            }
            tmpl.pages.push_back(page);
        }
    }

    return tmpl;
}