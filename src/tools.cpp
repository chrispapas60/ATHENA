#include "../include/tools.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

std::string to_lower_copy(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return text;
}

bool starts_with(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

std::string trim_copy(const std::string& text) {
    size_t start = text.find_first_not_of(" \t\n\r");
    size_t end = text.find_last_not_of(" \t\n\r");

    if (start == std::string::npos) {
        return "";
    }

    return text.substr(start, end - start + 1);
}

std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::uppercase << static_cast<int>(c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string clean_for_powershell_here_string(const std::string& text) {
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

std::string open_website(const std::string& url) {
    std::string command = "cmd.exe /C start \"\" \"" + url + "\"";
    int result = std::system(command.c_str());

    if (result != 0) {
        return "I tried to open the website, but Windows failed.";
    }

    return "Opening it now.";
}

std::string open_application(const std::string& app_command, const std::string& display_name) {
    std::string command = "cmd.exe /C start \"\" " + app_command;
    int result = std::system(command.c_str());

    if (result != 0) {
        return "I tried to open " + display_name + ", but Windows failed.";
    }

    return "Opening " + display_name + ".";
}

std::string copy_to_clipboard_and_paste(const std::string& text) {
    std::string safe_text = clean_for_powershell_here_string(text);

    std::ofstream ps("athena_type.ps1");

    if (!ps) {
        return "I could not create the typing script.";
    }

    ps << "Add-Type -AssemblyName System.Windows.Forms\n";
    ps << "$text = @'\n";
    ps << safe_text << "\n";
    ps << "'@\n";
    ps << "Set-Clipboard -Value $text\n";
    ps << "Start-Sleep -Milliseconds 300\n";
    ps << "[System.Windows.Forms.SendKeys]::SendWait('^v')\n";

    ps.close();

    int result = std::system(
        "powershell.exe -NoProfile -ExecutionPolicy Bypass -File athena_type.ps1"
    );

    if (result != 0) {
        return "I copied the text, but pasting failed.";
    }

    return "Done. I typed it.";
}

std::string handle_pc_command(const std::string& user_input, bool& handled) {
    handled = true;

    std::string raw = trim_copy(user_input);
    std::string input = to_lower_copy(raw);

    if (input == "open youtube" || input == "youtube") {
        return open_website("https://www.youtube.com");
    }

    if (input == "open google" || input == "google") {
        return open_website("https://www.google.com");
    }

    if (input == "open github" || input == "github") {
        return open_website("https://github.com");
    }

    if (input == "open chatgpt" || input == "chatgpt") {
        return open_website("https://chatgpt.com");
    }

    if (input == "open word" || input == "open microsoft word" || input == "word") {
        return open_application("winword", "Word");
    }

    if (input == "open notepad" || input == "notepad") {
        return open_application("notepad", "Notepad");
    }

    if (input == "open calculator" || input == "calculator") {
        return open_application("calc", "Calculator");
    }

    if (input == "open steam" || input == "steam") {
        return open_website("steam://open/main");
    }

    if (
        input == "open vscode" ||
        input == "open vs code" ||
        input == "open visual studio code" ||
        input == "vscode" ||
        input == "vs code"
    ) {
        return open_application("code", "VS Code");
    }

    if (starts_with(input, "open website ")) {
        std::string url = trim_copy(raw.substr(std::string("open website ").length()));

        if (
            !starts_with(to_lower_copy(url), "http://") &&
            !starts_with(to_lower_copy(url), "https://")
        ) {
            url = "https://" + url;
        }

        return open_website(url);
    }

    if (starts_with(input, "go to ")) {
        std::string url = trim_copy(raw.substr(std::string("go to ").length()));

        if (
            !starts_with(to_lower_copy(url), "http://") &&
            !starts_with(to_lower_copy(url), "https://")
        ) {
            url = "https://" + url;
        }

        return open_website(url);
    }

    if (starts_with(input, "search ")) {
        std::string query = trim_copy(raw.substr(std::string("search ").length()));
        std::string url = "https://www.google.com/search?q=" + url_encode(query);
        return open_website(url);
    }

    if (starts_with(input, "write ")) {
        std::string text = trim_copy(raw.substr(std::string("write ").length()));
        return copy_to_clipboard_and_paste(text);
    }

    if (starts_with(input, "type ")) {
        std::string text = trim_copy(raw.substr(std::string("type ").length()));
        return copy_to_clipboard_and_paste(text);
    }

    if (input == "lock pc" || input == "lock computer") {
        int result = std::system("rundll32.exe user32.dll,LockWorkStation");

        if (result != 0) {
            return "I tried to lock the PC, but Windows failed.";
        }

        return "Locked.";
    }

    handled = false;
    return "";
}
