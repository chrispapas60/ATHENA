#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include <string>

std::string open_project();
std::string create_note();
std::string git_status();
std::string start_focus();
std::string start_research_mode();
std::string lock_pc();
void log_event(const std::string& message);

#endif