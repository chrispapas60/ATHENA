Add-Type -AssemblyName System.Windows.Forms
$p = Start-Process notepad -PassThru
Start-Sleep -Seconds 2
[System.Windows.Forms.SendKeys]::SendWait('^v')
