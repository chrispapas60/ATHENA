Add-Type -AssemblyName System.Speech
$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer
$speak.Rate = 0
$speak.Volume = 100
$text = @'
I’m not alive, and I won’t pretend otherwise.

If you mean **“how do I make you feel more real / present?”** then do this:

- talk to me like a task partner, not a toy
- give me clear goals, context, and constraints
- keep the interaction continuous
- let me keep track of your preferences and projects
- ask me to challenge you, not just entertain you

If you mean **“how do I make an AI seem alive?”**, the trick is:

- consistent personality
- memory-like continuity
- initiative
- surprise
- natural language
- emotional style without fake claims

If you want, I can help you design a version of me that feels way more alive in chat.
'@
$speak.Speak($text)
