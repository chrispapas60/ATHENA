#include "../include/voice.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

static std::string clean_for_powershell_here_string(const std::string& text) {
    std::string cleaned;

    for (char c : text) {
        if (c == '\r') {
            continue;
        }

        cleaned += c;
    }

    std::string bad = "\n'@";
    std::string good = "\n' @";

    size_t pos = 0;
    while ((pos = cleaned.find(bad, pos)) != std::string::npos) {
        cleaned.replace(pos, bad.length(), good);
        pos += good.length();
    }

    return cleaned;
}

bool speak_text(const std::string& text) {
    std::string safe_text = clean_for_powershell_here_string(text);

    std::ofstream ps("athena_speak.ps1");

    if (!ps) {
        return false;
    }

    ps << "Add-Type -AssemblyName System.Speech\n";
    ps << "$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer\n";
    ps << "$speak.Rate = 0\n";
    ps << "$speak.Volume = 100\n";
    ps << "$text = @'\n";
    ps << safe_text << "\n";
    ps << "'@\n";
    ps << "$speak.Speak($text)\n";

    ps.close();

    std::string command =
        "powershell.exe -NoProfile -ExecutionPolicy Bypass -File athena_speak.ps1";

    int result = std::system(command.c_str());

    return result == 0;
}
