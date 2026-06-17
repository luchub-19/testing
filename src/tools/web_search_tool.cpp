#include "tool.h"
#include <cstdio>
#include <nlohmann/json.hpp>

std::string WebSearchTool::getName()        const { return "web_search"; }
std::string WebSearchTool::getDescription() const {
    return "Tìm kiếm web qua DuckDuckGo (cần curl). "
           "Args: query string hoặc JSON {\"query\":\"...\"}";
}

std::optional<std::string> WebSearchTool::execute(const std::string& args) {
    std::string query = args;
    try {
        auto j = nlohmann::json::parse(args);
        if (j.contains("query")) query = j["query"].get<std::string>();
    } catch (...) {}

    if (query.empty()) return std::nullopt;

    // URL-encode cơ bản: space → +, giữ nguyên alphanumeric
    std::string encoded;
    for (unsigned char c : query) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.') {
            encoded += (char)c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", c);
            encoded += buf;
        }
    }

    std::string cmd = "curl -s --max-time 8 "
        "\"https://api.duckduckgo.com/?q=" + encoded +
        "&format=json&no_html=1&skip_disambig=1\" 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error: curl unavailable";

    std::string raw;
    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) raw += buf;
    pclose(pipe);

    if (raw.empty()) return "No results for: " + query;

    try {
        auto j    = nlohmann::json::parse(raw);
        std::string answer   = j.value("Answer", "");
        std::string abstract = j.value("AbstractText", "");
        std::string source   = j.value("AbstractSource", "");

        if (!answer.empty())   return "Answer: " + answer;
        if (!abstract.empty()) return abstract + (source.empty() ? "" : " [" + source + "]");
        return "No direct answer found for: " + query;
    } catch (...) {
        return raw;
    }
}
