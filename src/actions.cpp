#include "../include/actions.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;

// Change this later to your actual project folder.
const std::string PROJECT_PATH = "/mnt/c/Users/chris/OneDrive/Υπολογιστής/ATHENA";

std::string now_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void log_event(const std::string& message) {
    fs::create_directories("logs");

    std::ofstream file("logs/athena_log.txt", std::ios::app);
    file << "[" << now_time() << "] " << message << "\n";
}

std::string run_command_capture(const std::string& command) {
    std::string result;
    char buffer[256];

    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        return "Failed to run command.";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    if (result.empty()) {
        return "Command finished with no output.";
    }

    return result;
}

std::string open_project() {
    if (!fs::exists(PROJECT_PATH)) {
        return "Project path does not exist: " + PROJECT_PATH;
    }

    std::string command = "code \"" + PROJECT_PATH + "\"";
    std::system(command.c_str());

    log_event("Opened project.");
    return "Opened ATHENA project in VS Code.";
}

std::string create_note() {
    fs::create_directories("notes");

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::ostringstream date;
    date << std::put_time(std::localtime(&now_c), "%Y-%m-%d");

    std::string note_path = "notes/" + date.str() + "_note.md";

    if (!fs::exists(note_path)) {
        std::ofstream note(note_path);
        note << "# ATHENA Note — " << date.str() << "\n\n";
        note << "## Tasks\n\n";
        note << "- [ ] \n\n";
        note << "## Notes\n\n";
    }

    std::string command = "code \"" + note_path + "\"";
    std::system(command.c_str());

    log_event("Created/opened note: " + note_path);
    return "Opened today's note: " + note_path;
}

std::string git_status() {
    if (!fs::exists(PROJECT_PATH)) {
        return "Project path does not exist: " + PROJECT_PATH;
    }

    std::string command = "cd \"" + PROJECT_PATH + "\" && git status";
    std::string output = run_command_capture(command);

    log_event("Checked git status.");
    return output;
}

std::string start_focus() {
    log_event("Started focus mode.");
    return "Focus mode started. Work now.";
}

std::string start_research_mode() {
    std::ostringstream response;

    response << open_project() << "\n";
    response << create_note() << "\n";
    response << start_focus() << "\n\n";
    response << "Git status:\n";
    response << git_status();

    log_event("Started research mode.");
    return response.str();
}

std::string lock_pc() {
    std::system("powershell.exe -Command \"rundll32.exe user32.dll,LockWorkStation\"");

    log_event("Locked PC.");
    return "PC locked.";
}