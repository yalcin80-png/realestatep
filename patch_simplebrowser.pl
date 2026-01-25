use strict;
use warnings;
local $/;
my $file = '/mnt/data/fix3/SimpleBrowser.cpp';
open my $fh, '<:raw', $file or die $!;
my $data = <$fh>;
close $fh;

my $insert = "static std::wstring WebView2JsonStringToWString(LPCWSTR resultObjectAsJson)\r\n".
"{\r\n".
"    if (!resultObjectAsJson) return L\"\";\r\n".
"    try {\r\n".
"        std::wstring w = resultObjectAsJson;\r\n".
"        // JSON decode icin UTF-8'e cevir\r\n".
"        int u8len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);\r\n".
"        std::string u8; u8.resize(u8len);\r\n".
"        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &u8[0], u8len, nullptr, nullptr);\r\n".
"        // nlohmann::json ile decode: \"...\" -> ... ve \\u003C -> < gibi\r\n".
"        json j = json::parse(u8, nullptr, false);\r\n".
"        if (j.is_discarded()) return L\"\";\r\n".
"        if (!j.is_string()) return L\"\";\r\n".
"        std::string s = j.get<std::string>();\r\n".
"        int wlen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);\r\n".
"        std::wstring out; out.resize(wlen);\r\n".
"        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], wlen);\r\n".
"        return out;\r\n".
"    } catch (...) {\r\n".
"        return L\"\";\r\n".
"    }\r\n".
"}\r\n\r\n";

# Insert helper after using namespace line
if ($data =~ /using namespace Microsoft::WRL;\r\n\r\n/s) {
    $data =~ s/using namespace Microsoft::WRL;\r\n\r\n/using namespace Microsoft::WRL;\r\n\r\n$insert/s;
}

# Replace GetSourceCode decode block
$data =~ s/std::wstring html = resultObjectAsJson;\r\n\s*if \(html\.size\(\) > 2\) html = html\.substr\(1, html\.size\(\) - 2\);\r\n\s*\/\/ Ka\ç\ış karakterlerini temizle \(\\\"\)\r\n\s*size_t pos = 0;\r\n\s*while \(\(pos = html\.find\(L"\\\\\"", pos\)\) != std::wstring::npos\) \{ html\.replace\(pos, 2, L"\""\); pos\+\+; \}\r\n\s*while \(\(pos = html\.find\(L"\\\\\\\\", pos\)\) != std::wstring::npos\) \{ html\.replace\(pos, 2, L"\\\\"\); pos\+\+; \}\r\n\r\n\s*callback\(html\);/std::wstring html = WebView2JsonStringToWString(resultObjectAsJson);\r\n\s*callback(html);/s;

# Replace GetListingJSON decode block
$data =~ s/std::wstring jsonStr = resultObjectAsJson;\r\n\r\n\s*\/\/ Gelen veri yine JSON string i\çindedir \(\"\.\.\.\"\), d\ış t\ırnaklar\ı temizle\r\n\s*if \(jsonStr\.size\(\) >= 2 && jsonStr\.front\(\) == '"'\) \{\r\n\s*jsonStr = jsonStr\.substr\(1, jsonStr\.size\(\) - 2\);\r\n\s*\}\r\n\r\n\s*\/\/ İ\çerideki escape karakterlerini temizle \(\\\"\)\r\n\s*size_t pos = 0;\r\n\s*while \(\(pos = jsonStr\.find\(L"\\\\\"", pos\)\) != std::wstring::npos\) \{ jsonStr\.replace\(pos, 2, L"\""\); pos\+\+; \}\r\n\r\n\s*callback\(jsonStr\);/std::wstring jsonStr = WebView2JsonStringToWString(resultObjectAsJson);\r\n\s*callback(jsonStr);/s;

open my $oh, '>:raw', $file or die $!;
print $oh $data;
close $oh;
print "patched SimpleBrowser.cpp\n";
