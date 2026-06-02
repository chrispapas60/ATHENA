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

std::string ask_athena(
    const std::vector<std::pair<std::string, std::string>>& conversation
) {
    const char* api_key = std::getenv("GROQ_API_KEY");

    if (!api_key) {
        return "GROQ_API_KEY is not set.";
    }

    std::string personality = read_file("personality/athena_personality.md");

    json messages = json::array();

    messages.push_back({
        {"role", "system"},
        {"content", personality}
    });

    for (const auto& message : conversation) {
        messages.push_back({
            {"role", message.first},
            {"content", message.second}
        });
    }

    json request_body = {
        {"model", "llama-3.1-8b-instant"},
        {"messages", messages},
        {"temperature", 0.7},
        {"max_tokens", 500}
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

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.groq.com/openai/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_text.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    long http_code = 0;

    CURLcode result = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        return std::string("Groq curl error: ") + curl_easy_strerror(result);
    }

    if (http_code < 200 || http_code >= 300) {
        return "Groq API error HTTP " + std::to_string(http_code) + ":\n" + response_data;
    }

    try {
        json response_json = json::parse(response_data);

        if (
            response_json.contains("choices") &&
            response_json["choices"].is_array() &&
            !response_json["choices"].empty() &&
            response_json["choices"][0].contains("message") &&
            response_json["choices"][0]["message"].contains("content")
        ) {
            return response_json["choices"][0]["message"]["content"].get<std::string>();
        }

        return "Could not find Groq response text:\n" + response_data;
    }
    catch (const std::exception& e) {
        return std::string("Groq JSON parse error: ") + e.what() +
               "\nRaw response:\n" + response_data;
    }
}
