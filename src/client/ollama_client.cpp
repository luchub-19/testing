#include "ollama_client.h"
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

using json = nlohmann::json;

OllamaClient::OllamaClient(const std::string& model,
                             const std::string& base_url,
                             float temperature,
                             int   max_tokens)
    : base_url_(base_url)
    , model_name_(model)
    , temperature_(temperature)
    , max_tokens_(max_tokens) {}

void OllamaClient::handleError(int code) const {
    throw std::runtime_error("[OllamaClient] HTTP error: " + std::to_string(code));
}

std::string OllamaClient::chat(const std::string&              prompt,
                                const std::vector<std::string>& history) {
    // Xây dựng messages array theo Ollama /api/chat format
    // history: [user_0, assistant_0, user_1, assistant_1, ...]
    json messages = json::array();
    for (std::size_t i = 0; i < history.size(); ++i) {
        std::string role = (i % 2 == 0) ? "user" : "assistant";
        messages.push_back({{"role", role}, {"content", history[i]}});
    }
    messages.push_back({{"role", "user"}, {"content", prompt}});

    json body = {
        {"model",    model_name_},
        {"messages", messages},
        {"stream",   false},
        {"options",  {
            {"temperature", temperature_},
            {"num_predict", max_tokens_}
        }}
    };

    // Tách host:port từ base_url_
    std::string host = base_url_;
    int port = 11434;
    auto colon = base_url_.rfind(':');
    if (colon != std::string::npos && colon > 6) {   // bỏ qua "http://"
        host = base_url_.substr(0, colon);
        port = std::stoi(base_url_.substr(colon + 1));
    }
    // Bỏ prefix http://
    if (host.substr(0, 7) == "http://")  host = host.substr(7);
    if (host.substr(0, 8) == "https://") host = host.substr(8);

    httplib::Client cli(host, port);
    cli.set_connection_timeout(30);
    cli.set_read_timeout(120);

    auto res = cli.Post("/api/chat",
                        body.dump(),
                        "application/json");

    if (!res)           throw std::runtime_error("[OllamaClient] No response from Ollama");
    if (res->status != 200) handleError(res->status);

    try {
        auto j = json::parse(res->body);
        return j["message"]["content"].get<std::string>();
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("[OllamaClient] Parse error: ") + e.what());
    }
}
