#include "../include/ai.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        return "You are ATHENA, Chris's personal AI assistant. Speak naturally, directly, and usefully.";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string extract_text_from_response(const json& response_json, const std::string& raw_response) {
    if (response_json.contains("output_text") && response_json["output_text"].is_string()) {
        return response_json["output_text"].get<std::string>();
    }

    if (
        response_json.contains("output") &&
        response_json["output"].is_array() &&
        !response_json["output"].empty()
    ) {
        for (const auto& item : response_json["output"]) {
            if (
                item.contains("content") &&
                item["content"].is_array()
            ) {
                for (const auto& content : item["content"]) {
                    if (content.contains("text") && content["text"].is_string()) {
                        return content["text"].get<std::string>();
                    }
                }
            }
        }
    }

    return "Could not find text in response:\n" + raw_response;
}

std::string ask_athena(
    const std::vector<std::pair<std::string, std::string>>& conversation
) {
    const char* api_key = std::getenv("OPENAI_API_KEY");

    if (!api_key) {
        return "OPENAI_API_KEY is not set.";
    }

    std::string personality = read_file("personality/athena_personality.md");

    json input_messages = json::array();

    input_messages.push_back({
        {"role", "system"},
        {"content", personality}
    });

    for (const auto& message : conversation) {
        input_messages.push_back({
            {"role", message.first},
            {"content", message.second}
        });
    }

    json request_body = {
        {"model", "gpt-5.4-mini"},
        {"input", input_messages}
    };

    std::string response_data;

    CURL* curl = curl_easy_init();

    if (!curl) {
        return "Failed to initialize curl.";
    }

    struct curl_slist* headers = nullptr;
    std::string auth_header = std::string("Authorization: Bearer ") + api_key;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());

    std::string request_text = request_body.dump();

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/responses");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_text.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode result = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        return std::string("curl error: ") + curl_easy_strerror(result);
    }

    try {
        json response_json = json::parse(response_data);
        return extract_text_from_response(response_json, response_data);
    }
    catch (const std::exception& e) {
        return std::string("JSON parse error: ") + e.what() + "\nRaw response:\n" + response_data;
    }
}