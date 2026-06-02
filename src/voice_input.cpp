#include "../include/voice_input.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string run_command_capture_input(const std::string& command) {
    std::string result;
    char buffer[256];

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}

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

std::string listen_and_transcribe() {
    std::string wav_linux = "athena_input.wav";
    std::string wav_windows = run_command_capture_input("wslpath -w athena_input.wav");

    if (wav_windows.empty()) {
        return "[voice error: could not convert path]";
    }

    // Built-in laptop microphone:
    std::string microphone_name = "Microphone Array (Realtek(R) Audio)";

    std::cout << "[Recording 5 seconds... speak now]\n";

    std::string record_command =
        "powershell.exe -NoProfile -Command "
        "\"ffmpeg -hide_banner -loglevel error -y "
        "-f dshow "
        "-i audio='" + microphone_name + "' "
        "-t 5 -ac 1 -ar 16000 "
        "'" + wav_windows + "'\"";

    int record_result = std::system(record_command.c_str());

    if (record_result != 0) {
        return "[voice error: microphone recording failed]";
    }

    std::string whisper_command =
        "./external/whisper.cpp/build/bin/whisper-cli "
        "-m ./external/whisper.cpp/models/ggml-base.en.bin "
        "-f athena_input.wav "
        "-otxt "
        "-of athena_transcript "
        "-nt "
        ">/dev/null 2>&1";

    int whisper_result = std::system(whisper_command.c_str());

    if (whisper_result != 0) {
        return "[voice error: whisper transcription failed]";
    }

    std::string transcript = read_text_file_input("athena_transcript.txt");

    if (transcript.empty()) {
        return "[voice error: no speech detected]";
    }

    return transcript;
}