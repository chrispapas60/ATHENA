Add-Type -AssemblyName System.Windows.Forms
$text = @'
Athena
'@
[System.Windows.Forms.Clipboard]::SetText($text)
