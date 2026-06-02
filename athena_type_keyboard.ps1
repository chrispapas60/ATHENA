$wshell = New-Object -ComObject WScript.Shell
$text = @'
Athena is typing like a keyboard
'@

function Send-OneChar($ch) {
    switch ($ch) {
        "`n" { $wshell.SendKeys('{ENTER}'); return }
        "`t" { $wshell.SendKeys('{TAB}'); return }
        " "  { $wshell.SendKeys(' '); return }
        "+"  { $wshell.SendKeys('{+}'); return }
        "^"  { $wshell.SendKeys('{^}'); return }
        "%"  { $wshell.SendKeys('{%}'); return }
        "~"  { $wshell.SendKeys('{~}'); return }
        "("  { $wshell.SendKeys('{(}'); return }
        ")"  { $wshell.SendKeys('{)}'); return }
        "["  { $wshell.SendKeys('{[}'); return }
        "]"  { $wshell.SendKeys('{]}'); return }
        "{"  { $wshell.SendKeys('{{}'); return }
        "}"  { $wshell.SendKeys('{}}'); return }
        default { $wshell.SendKeys([string]$ch); return }
    }
}

Start-Sleep -Milliseconds 600
foreach ($ch in $text.ToCharArray()) {
    Send-OneChar $ch
    Start-Sleep -Milliseconds 45
}
