Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
}
"@

$text = @'
Athena is typing this character by character
'@

$p = Start-Process notepad.exe -PassThru
Start-Sleep -Seconds 2
$p.Refresh()
[Win32]::ShowWindowAsync($p.MainWindowHandle, 3) | Out-Null
[Win32]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 800

function Send-OneChar($ch) {
    switch ($ch) {
        "`n" { [System.Windows.Forms.SendKeys]::SendWait('{ENTER}'); return }
        "`t" { [System.Windows.Forms.SendKeys]::SendWait('{TAB}'); return }
        " "  { [System.Windows.Forms.SendKeys]::SendWait(' '); return }
        "+"  { [System.Windows.Forms.SendKeys]::SendWait('{+}'); return }
        "^"  { [System.Windows.Forms.SendKeys]::SendWait('{^}'); return }
        "%"  { [System.Windows.Forms.SendKeys]::SendWait('{%}'); return }
        "~"  { [System.Windows.Forms.SendKeys]::SendWait('{~}'); return }
        "("  { [System.Windows.Forms.SendKeys]::SendWait('{(}'); return }
        ")"  { [System.Windows.Forms.SendKeys]::SendWait('{)}'); return }
        "["  { [System.Windows.Forms.SendKeys]::SendWait('{[}'); return }
        "]"  { [System.Windows.Forms.SendKeys]::SendWait('{]}'); return }
        "{"  { [System.Windows.Forms.SendKeys]::SendWait('{{}'); return }
        "}"  { [System.Windows.Forms.SendKeys]::SendWait('{}}'); return }
        default { [System.Windows.Forms.SendKeys]::SendWait([string]$ch); return }
    }
}

foreach ($ch in $text.ToCharArray()) {
    Send-OneChar $ch
    Start-Sleep -Milliseconds 55
}
