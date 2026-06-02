#include "../include/ai.hpp"
#include "../include/voice.hpp"
#include "../include/voice_input.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main() {
    std::cout << "ATHENA voice chat online.\n";
    std::cout << "Type normally, or press Enter on an empty line to speak.\n";
    std::cout << "Type 'exit' to quit.\n";

    std::vector<std::pair<std::string, std::string>> conversation;

    while (true) {
        std::cout << "\nChris > ";

        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") {
            std::cout << "ATHENA offline.\n";
            break;
        }

        if (input.empty()) {
            input = listen_and_transcribe();
            std::cout << "You said: " << input << "\n";
        }

        if (input.rfind("[voice error:", 0) == 0) {
            std::cout << input << "\n";
            continue;
        }

        conversation.push_back({"user", input});

        if (conversation.size() > 12) {
            conversation.erase(conversation.begin());
        }

        std::string reply = ask_athena(conversation);

        conversation.push_back({"assistant", reply});

        std::cout << "\nATHENA > " << reply << "\n";

        bool voice_ok = speak_text(reply);

        if (!voice_ok) {
            std::cout << "\n[Voice output failed]\n";
        }
    }

    return 0;
}