Add-Type -AssemblyName System.Speech
$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer
$speak.Rate = 0
$speak.Volume = 100
$text = @'
Committing changes with the message "Add basic PC tools to ATHENA".
'@
$speak.Speak($text)
