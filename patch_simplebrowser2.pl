use strict;use warnings;local $/;
my $file='/mnt/data/fix3/SimpleBrowser.cpp';
open my $fh,'<:raw',$file or die $!; my $d=<$fh>; close $fh;

# Patch GetSourceCode callback body
$d =~ s/if \(resultObjectAsJson\) \{\r\n\s*\/\/ Basit JSON decode[\s\S]*?callback\(html\);\r\n\s*\}\r\n\s*else callback\(L""\);/if (resultObjectAsJson) {\r\n                    std::wstring html = WebView2JsonStringToWString(resultObjectAsJson);\r\n                    callback(html);\r\n                }\r\n                else callback(L"");/;

# Patch GetListingJSON callback body
$d =~ s/if \(resultObjectAsJson\) \{\r\n\s*std::wstring jsonStr[\s\S]*?callback\(jsonStr\);\r\n\s*\}\r\n\s*else \{\r\n\s*callback\(L""\);\r\n\s*\}/if (resultObjectAsJson) {\r\n                    std::wstring jsonStr = WebView2JsonStringToWString(resultObjectAsJson);\r\n                    callback(jsonStr);\r\n                }\r\n                else {\r\n                    callback(L"");\r\n                }/;

open my $of,'>:raw',$file or die $!; print $of $d; close $of;
print "patched SimpleBrowser.cpp (decode)\n";
