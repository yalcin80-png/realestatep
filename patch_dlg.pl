use strict;use warnings;local $/;
my $file='/mnt/data/fix3/SahibindenImportDlg.cpp';
open my $fh,'<:raw',$file or die $!; my $d=<$fh>; close $fh;

# 1) Fix OnButtonNavigate URL source (edit control)
$d =~ s/strUrl = GetWindowText\(\);\r\n/strUrl = m_editUrl.GetWindowText();\r\n/;

# 2) Replace OnButtonImport implementation to always grab full HTML
my $new_onimport = <<'CPP';
void CSahibindenImportDlg::OnButtonImport()
{
    LogLine(_T("Veri çekiliyor (Full HTML Mode)..."));
    m_btnImport.EnableWindow(FALSE);

    // En güvenilir yöntem: WebView2'nin render ettiği tam HTML'i alıp importer'a ver.
    // Not: Telefon çoğu zaman kullanıcı 'Telefonu Göster' tıklamadan DOM'da görünmez.
    m_browser.GetSourceCode([this](std::wstring htmlContent) {
        this->ProcessHtmlContent(htmlContent);
    });
}
CPP

# Replace the whole existing OnButtonImport function
$d =~ s/void CSahibindenImportDlg::OnButtonImport\(\)\r\n\{[\s\S]*?\}\r\n\r\n\/\/ JSON Verisini İşle/\Q$new_onimport\E\r\n\r\n\/\/ JSON Verisini İşle/;

# 3) Fix currentUrl usage in ProcessJsonContent (if ever used)
$d =~ s/currentUrl = GetWindowText\(\);/currentUrl = m_editUrl.GetWindowText();/g;

open my $out,'>:raw',$file or die $!; print $out $d; close $out;
print "patched Sah...Dlg.cpp\n";
