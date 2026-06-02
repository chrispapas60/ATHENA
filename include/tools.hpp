#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>

std::string open_website(const std::string& url);
std::string open_application(const std::string& app);
std::string handle_basic_tool_command(const std::string& user_input, bool& handled);

#endif
