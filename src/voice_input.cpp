#include "../include/voice_input.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string read_text_file_input(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();

    std::string text = buffer.str();

    while (
        !text.empty() &&
        (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')
    ) {
        text.pop_back();
    }

    return text;
}

bool file_exists_input(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

void create_record_script() {
    std::ofstream ps("athena_record.ps1");

    // Built-in laptop microphone:
    ps << "$mic = 'Microphone Array (Realtek(R) Audio)'\n";

    // If you want the Logitech headset instead, replace the line above with:
    // ps << "$mic = 'Microphone (Logitech PRO X Gaming Headset)'\n";

    ps << "$out = 'C:\\ATHENA\\athena_input.wav'\n";

    // Light filter. Heavy noise reduction can destroy words.
    ps << "$filter = 'highpass=f=70,lowpass=f=7600,volume=2.0'\n\n";

    ps << "$args = '-hide_banner -loglevel error -y -f dshow -i \"audio=' + $mic + '\" -ac 1 -ar 16000 -af \"' + $filter + '\" \"' + $out + '\"'\n\n";

    ps << "$psi = New-Object System.Diagnostics.ProcessStartInfo\n";
    ps << "$psi.FileName = 'ffmpeg.exe'\n";
    ps << "$psi.Arguments = $args\n";
    ps << "$psi.UseShellExecute = $false\n";
    ps << "$psi.RedirectStandardInput = $true\n";
    ps << "$psi.RedirectStandardOutput = $false\n";
    ps << "$psi.RedirectStandardError = $false\n\n";

    ps << "$p = New-Object System.Diagnostics.Process\n";
    ps << "$p.StartInfo = $psi\n";
    ps << "[void]$p.Start()\n\n";

    ps << "Write-Host '[Recording... press Enter to stop]'\n";
    ps << "[void][System.Console]::ReadLine()\n\n";

    ps << "try {\n";
    ps << "    $p.StandardInput.WriteLine('q')\n";
    ps << "    if (-not $p.WaitForExit(3000)) {\n";
    ps << "        $p.Kill()\n";
    ps << "    }\n";
    ps << "} catch {\n";
    ps << "    if ($p -and -not $p.HasExited) {\n";
    ps << "        $p.Kill()\n";
    ps << "    }\n";
    ps << "}\n";

    ps.close();
}

std::string choose_model_file_windows() {
    if (file_exists_input("external/whisper.cpp/models/ggml-medium.en.bin")) {
        return "C:\\ATHENA\\external\\whisper.cpp\\models\\ggml-medium.en.bin";
    }

    if (file_exists_input("external/whisper.cpp/models/ggml-small.en.bin")) {
        return "C:\\ATHENA\\external\\whisper.cpp\\models\\ggml-small.en.bin";
    }

    if (file_exists_input("external/whisper.cpp/models/ggml-base.en.bin")) {
        return "C:\\ATHENA\\external\\whisper.cpp\\models\\ggml-base.en.bin";
    }

    return "";
}

std::string listen_and_transcribe() {
    std::string wav_file = "athena_input.wav";
    std::string transcript_file = "athena_transcript.txt";

    create_record_script();

    std::cout << "[Press Enter again when you want to stop recording]\n";

    int record_result = std::system(
        "powershell.exe -NoProfile -ExecutionPolicy Bypass -File athena_record.ps1"
    );

    if (record_result != 0 || !file_exists_input(wav_file)) {
        return "[voice error: microphone recording failed]";
    }

    std::string whisper_exe = "C:\\ATHENA\\external\\whisper.cpp\\build\\bin\\whisper-cli.exe";

    if (!file_exists_input("external/whisper.cpp/build/bin/whisper-cli.exe")) {
        return "[voice error: whisper executable not found]";
    }

    std::string model_file = choose_model_file_windows();

    if (model_file.empty()) {
        return "[voice error: no whisper model found]";
    }

    std::string whisper_command =
        "cmd.exe /C \"\""
        + whisper_exe +
        "\" -m \""
        + model_file +
        "\" -f \"C:\\ATHENA\\athena_input.wav\" "
        "-l en "
        "-otxt "
        "-of \"C:\\ATHENA\\athena_transcript\" "
        "--prompt \"This is a voice command for ATHENA, a personal computer assistant. The user may say commands like open YouTube, open CERN, open Google Docs, open Word, search quantum mechanics notes, write this text, create a file, explain code, or talk casually.\""
        "\"";

    std::cout << "[Running Whisper]\n";

    int whisper_result = std::system(whisper_command.c_str());

    if (whisper_result != 0 || !file_exists_input(transcript_file)) {
        return "[voice error: whisper transcription failed]";
    }

    std::string transcript = read_text_file_input(transcript_file);

    if (transcript.empty()) {
        return "[voice error: no speech detected]";
    }

    std::cout << "\n[Transcript detected]\n";
    std::cout << transcript << "\n";

    return transcript;
}
