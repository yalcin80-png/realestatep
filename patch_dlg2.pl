use strict;use warnings;local $/;
my $file='/mnt/data/fix3/SahibindenImportDlg.cpp';
open my $fh,'<:raw',$file or die $!; my $d=<$fh>; close $fh;

my $newfunc = "void CSahibindenImportDlg::OnButtonImport()\n".
"{\n".
"    CString currentUrl = m_editUrl.GetWindowText();\n".
"    currentUrl.Trim();\n".
"    if (currentUrl.IsEmpty()) { LogLine(_T(\"URL boş.\")); m_btnImport.EnableWindow(TRUE); return; }\n\n".
"    LogLine(_T(\"Sayfa kaynağı alınıyor (WebView2)...\"));\n".
"    m_btnImport.EnableWindow(FALSE);\n\n".
"    // Not: JSON-only wrapper ile parse etmek satıcı adı/telefon gibi alanları kaçırır.\n".
"    // Bu yüzden tam DOM HTML'i alıp import ediyoruz (içinde gaPageViewTrackingJson da var).\n".
"    m_browser.GetSourceCode([this](std::wstring htmlContent) {\n".
"        this->ProcessHtmlContent(htmlContent);\n".
"    });\n".
"}\n\n";

my $re = qr/void CSahibindenImportDlg::OnButtonImport\(\)\n\{[\s\S]*?\n\}\n\n\/\/ JSON Verisini İşle/;

if ($d =~ $re) {
    $d =~ s/$re/$newfunc."\/\/ JSON Verisini İşle"/e;
} else {
    die "could not find OnButtonImport block";
}

open my $oh,'>:raw',$file or die $!; print $oh $d; close $oh;
print "patched OnButtonImport\n";
