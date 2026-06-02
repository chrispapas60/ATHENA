#include "../include/tools.hpp"

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static std::string to_lower_tool(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return text;
}

static std::string trim_tool(const std::string& text) {
    size_t start = text.find_first_not_of(" \t\n\r");
    size_t end = text.find_last_not_of(" \t\n\r");

    if (start == std::string::npos) return "";
    return text.substr(start, end - start + 1);
}

static bool starts_with_tool(const std::string& text, const std::string& prefix) {
    return text.rfind(prefix, 0) == 0;
}

static bool contains_tool(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

static std::wstring utf8_to_wide(const std::string& text) {
    if (text.empty()) return L"";

    int size_needed = MultiByteToWideChar(
        CP_UTF8,
        0,
        text.c_str(),
        -1,
        nullptr,
        0
    );

    if (size_needed <= 0) return L"";

    std::wstring wide(size_needed, 0);

    MultiByteToWideChar(
        CP_UTF8,
        0,
        text.c_str(),
        -1,
        &wide[0],
        size_needed
    );

    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }

    return wide;
}

static std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else if (c == ' ') {
            escaped << '+';
        }
        else {
            escaped << '%' << std::uppercase << std::setw(2) << int(c) << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string open_website(const std::string& url) {
    std::wstring wide_url = utf8_to_wide(url);

    HINSTANCE result = ShellExecuteW(
        nullptr,
        L"open",
        wide_url.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );

    if ((INT_PTR)result <= 32) {
        return "I tried to open the website, but Windows failed.";
    }

    return "Opening " + url;
}

std::string open_application(const std::string& app) {
    std::wstring wide_app = utf8_to_wide(app);

    HINSTANCE result = ShellExecuteW(
        nullptr,
        L"open",
        wide_app.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );

    if ((INT_PTR)result <= 32) {
        return "I tried to open " + app + ", but Windows failed.";
    }

    return "Opening " + app;
}

struct WindowSearchData {
    DWORD process_id;
    HWND hwnd;
};

static BOOL CALLBACK enum_windows_callback(HWND hwnd, LPARAM lparam) {
    WindowSearchData* data = reinterpret_cast<WindowSearchData*>(lparam);

    DWORD window_pid = 0;
    GetWindowThreadProcessId(hwnd, &window_pid);

    if (window_pid == data->process_id && IsWindowVisible(hwnd)) {
        data->hwnd = hwnd;
        return FALSE;
    }

    return TRUE;
}

static HWND find_window_for_process(DWORD process_id) {
    WindowSearchData data;
    data.process_id = process_id;
    data.hwnd = nullptr;

    for (int i = 0; i < 30; ++i) {
        EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&data));

        if (data.hwnd != nullptr) {
            return data.hwnd;
        }

        Sleep(200);
    }

    return nullptr;
}

static bool focus_window(HWND hwnd) {
    if (!hwnd) return false;

    ShowWindow(hwnd, SW_RESTORE);
    Sleep(100);

    SetForegroundWindow(hwnd);
    Sleep(300);

    return true;
}

static void send_virtual_key(WORD vk, bool key_down) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;

    if (!key_down) {
        input.ki.dwFlags = KEYEVENTF_KEYUP;
    }

    SendInput(1, &input, sizeof(INPUT));
}

static void press_virtual_key(WORD vk) {
    send_virtual_key(vk, true);
    Sleep(20);
    send_virtual_key(vk, false);
    Sleep(30);
}

static void send_unicode_char(wchar_t ch) {
    INPUT input_down = {};
    input_down.type = INPUT_KEYBOARD;
    input_down.ki.wScan = ch;
    input_down.ki.dwFlags = KEYEVENTF_UNICODE;

    INPUT input_up = input_down;
    input_up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    SendInput(1, &input_down, sizeof(INPUT));
    Sleep(8);
    SendInput(1, &input_up, sizeof(INPUT));
    Sleep(25);
}

static bool type_like_keyboard_native(const std::string& text) {
    std::wstring wide_text = utf8_to_wide(text);

    if (wide_text.empty()) return false;

    Sleep(400);

    for (wchar_t ch : wide_text) {
        if (ch == L'\n') {
            press_virtual_key(VK_RETURN);
        }
        else if (ch == L'\t') {
            press_virtual_key(VK_TAB);
        }
        else {
            send_unicode_char(ch);
        }
    }

    return true;
}

static std::string type_text_active_window(const std::string& text) {
    std::string clean_text = trim_tool(text);

    if (clean_text.empty()) {
        return "Tell me what to type.";
    }

    bool ok = type_like_keyboard_native(clean_text);

    if (!ok) {
        return "Typing failed.";
    }

    return "Typed it like a real keyboard.";
}

static std::string press_key_command(const std::string& key_name) {
    std::string key = to_lower_tool(trim_tool(key_name));

    if (key == "enter") {
        press_virtual_key(VK_RETURN);
        return "Pressed Enter.";
    }

    if (key == "tab") {
        press_virtual_key(VK_TAB);
        return "Pressed Tab.";
    }

    if (key == "escape" || key == "esc") {
        press_virtual_key(VK_ESCAPE);
        return "Pressed Escape.";
    }

    if (key == "backspace") {
        press_virtual_key(VK_BACK);
        return "Pressed Backspace.";
    }

    if (key == "delete") {
        press_virtual_key(VK_DELETE);
        return "Pressed Delete.";
    }

    if (key == "space") {
        press_virtual_key(VK_SPACE);
        return "Pressed Space.";
    }

    if (key == "ctrl l" || key == "control l") {
        send_virtual_key(VK_CONTROL, true);
        press_virtual_key('L');
        send_virtual_key(VK_CONTROL, false);
        return "Selected address bar.";
    }

    if (key == "ctrl a" || key == "control a") {
        send_virtual_key(VK_CONTROL, true);
        press_virtual_key('A');
        send_virtual_key(VK_CONTROL, false);
        return "Selected all.";
    }

    if (key == "ctrl s" || key == "control s") {
        send_virtual_key(VK_CONTROL, true);
        press_virtual_key('S');
        send_virtual_key(VK_CONTROL, false);
        return "Saved.";
    }

    if (key == "alt tab") {
        send_virtual_key(VK_MENU, true);
        press_virtual_key(VK_TAB);
        send_virtual_key(VK_MENU, false);
        return "Switched window.";
    }

    return "I do not know that key yet: " + key_name;
}

static std::string open_notepad_and_type(const std::string& text) {
    std::string clean_text = trim_tool(text);

    if (clean_text.empty()) {
        return "Open Notepad and type what?";
    }

    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};

    si.cb = sizeof(si);

    wchar_t command_line[] = L"notepad.exe";

    BOOL created = CreateProcessW(
        nullptr,
        command_line,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!created) {
        return "Could not open Notepad.";
    }

    WaitForInputIdle(pi.hProcess, 5000);
    Sleep(800);

    HWND hwnd = find_window_for_process(pi.dwProcessId);

    if (!hwnd) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return "Notepad opened, but I could not find its window.";
    }

    focus_window(hwnd);
    Sleep(500);

    bool typed = type_like_keyboard_native(clean_text);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (!typed) {
        return "Notepad opened, but typing failed.";
    }

    return "Opened Notepad and typed the text like a keyboard.";
}

static void click_center_of_screen() {
    int screen_x = GetSystemMetrics(SM_CXSCREEN);
    int screen_y = GetSystemMetrics(SM_CYSCREEN);

    SetCursorPos(screen_x / 2, static_cast<int>(screen_y * 0.45));
    Sleep(200);

    INPUT inputs[2] = {};

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, inputs, sizeof(INPUT));
    Sleep(500);
}

static std::string open_google_docs_and_type(const std::string& text) {
    std::string clean_text = trim_tool(text);

    if (clean_text.empty()) {
        return "Open Google Docs and type what?";
    }

    open_website("https://docs.new");

    Sleep(12000);

    click_center_of_screen();

    bool typed = type_like_keyboard_native(clean_text);

    if (!typed) {
        return "Opened Google Docs, but typing failed.";
    }

    return "Opened Google Docs and typed the text like a keyboard.";
}

static std::string remove_common_words(std::string input) {
    input = trim_tool(input);

    std::vector<std::string> phrases = {
        "can you ",
        "please ",
        "for me",
        "athena ",
        "open a new tab for ",
        "open new tab for ",
        "open a tab for ",
        "open tab for ",
        "open a tab about ",
        "open tab about ",
        "open website ",
        "visit website ",
        "go to website ",
        "go to ",
        "search for ",
        "look up ",
        "find me ",
        "find ",
        "google ",
        "search ",
        "open ",
        "opne "
    };

    bool changed = true;

    while (changed) {
        changed = false;

        for (const auto& phrase : phrases) {
            std::string lower = to_lower_tool(input);
            size_t pos = lower.find(phrase);

            if (pos != std::string::npos) {
                input.erase(pos, phrase.size());
                input = trim_tool(input);
                changed = true;
                break;
            }
        }
    }

    return trim_tool(input);
}

static std::string open_tab_or_search(const std::string& request) {
    std::string cleaned = remove_common_words(request);
    std::string lower = to_lower_tool(cleaned);

    if (cleaned.empty()) {
        return "Tell me what tab to open.";
    }

    std::cout << "[Tab request cleaned] " << cleaned << "\n";

    std::unordered_map<std::string, std::string> known_sites = {
        {"youtube", "https://www.youtube.com"},
        {"you tube", "https://www.youtube.com"},
        {"google", "https://www.google.com"},
        {"gmail", "https://mail.google.com"},
        {"google docs", "https://docs.new"},
        {"docs", "https://docs.new"},
        {"google drive", "https://drive.google.com"},
        {"drive", "https://drive.google.com"},
        {"github", "https://github.com"},
        {"chatgpt", "https://chatgpt.com"},
        {"groq", "https://console.groq.com"},
        {"cern", "https://home.cern"},
        {"cern summer student", "https://careers.cern/students"},
        {"steam", "steam://open/main"}
    };

    for (const auto& site : known_sites) {
        if (lower == site.first || contains_tool(lower, site.first)) {
            return open_website(site.second);
        }
    }

    if (starts_with_tool(lower, "http://") || starts_with_tool(lower, "https://")) {
        return open_website(cleaned);
    }

    if (
        contains_tool(lower, ".com") ||
        contains_tool(lower, ".org") ||
        contains_tool(lower, ".net") ||
        contains_tool(lower, ".nl") ||
        contains_tool(lower, ".gr") ||
        contains_tool(lower, ".edu")
    ) {
        return open_website("https://" + cleaned);
    }

    return open_website("https://www.google.com/search?q=" + url_encode(cleaned));
}

std::string handle_basic_tool_command(const std::string& user_input, bool& handled) {
    handled = true;

    std::string input_original = trim_tool(user_input);
    std::string input = to_lower_tool(input_original);

    if (
        (contains_tool(input, "notepad") || contains_tool(input, "notpad")) &&
        (contains_tool(input, "type ") || contains_tool(input, "write "))
    ) {
        size_t pos_type = input.find("type ");
        size_t pos_write = input.find("write ");

        if (pos_type != std::string::npos) {
            return open_notepad_and_type(input_original.substr(pos_type + 5));
        }

        if (pos_write != std::string::npos) {
            return open_notepad_and_type(input_original.substr(pos_write + 6));
        }
    }

    if (
        contains_tool(input, "google docs") &&
        (contains_tool(input, "type ") || contains_tool(input, "write "))
    ) {
        size_t pos_type = input.find("type ");
        size_t pos_write = input.find("write ");

        if (pos_type != std::string::npos) {
            return open_google_docs_and_type(input_original.substr(pos_type + 5));
        }

        if (pos_write != std::string::npos) {
            return open_google_docs_and_type(input_original.substr(pos_write + 6));
        }
    }

    if (starts_with_tool(input, "type ")) {
        return type_text_active_window(input_original.substr(5));
    }

    if (starts_with_tool(input, "write ")) {
        return type_text_active_window(input_original.substr(6));
    }

    if (starts_with_tool(input, "press ")) {
        return press_key_command(input_original.substr(6));
    }

    if (starts_with_tool(input, "hit ")) {
        return press_key_command(input_original.substr(4));
    }

    if (input == "paste") {
        return press_key_command("ctrl v");
    }

    if (input == "select address bar") {
        return press_key_command("ctrl l");
    }

    if (input == "select all") {
        return press_key_command("ctrl a");
    }

    if (contains_tool(input, "word") || contains_tool(input, "microsoft word")) {
        return open_application("winword");
    }

    if (contains_tool(input, "vscode") || contains_tool(input, "vs code")) {
        return open_application("code");
    }

    if (
        contains_tool(input, "open") ||
        contains_tool(input, "opne") ||
        contains_tool(input, "tab") ||
        contains_tool(input, "website") ||
        contains_tool(input, "search") ||
        contains_tool(input, "google") ||
        contains_tool(input, "youtube") ||
        contains_tool(input, "cern") ||
        contains_tool(input, "github") ||
        contains_tool(input, "groq") ||
        contains_tool(input, "chatgpt") ||
        contains_tool(input, "docs") ||
        contains_tool(input, ".com") ||
        contains_tool(input, ".org")
    ) {
        return open_tab_or_search(input_original);
    }

    handled = false;
    return "";
}
