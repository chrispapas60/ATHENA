Add-Type -AssemblyName System.Speech
$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer
$speak.Rate = 0
$speak.Volume = 100
$text = @'
Opening https://www.youtube.com
'@
$speak.Speak($text)
