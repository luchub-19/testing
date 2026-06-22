#include "ollama_client.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>

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
    // ── Xây messages array ────────────────────────────────────
    // history xen kẽ: [user_0, assistant_0, user_1, assistant_1, ...]
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

    // ── Ghi request body ra file tạm ─────────────────────────
    std::string tmp_req = "/tmp/ollama_request_" +
                          std::to_string(std::rand()) + ".json";
    {
        std::ofstream f(tmp_req);
        if (!f) throw std::runtime_error(
            "[OllamaClient] Cannot write temp file: " + tmp_req);
        f << body.dump();
    }

    // ── Gọi Ollama qua curl (không cần httplib) ───────────────
    std::string cmd =
        "curl -s --max-time 120 "
        "-X POST \"" + base_url_ + "/api/chat\" "
        "-H \"Content-Type: application/json\" "
        "-d @" + tmp_req + " 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::remove(tmp_req.c_str());
        throw std::runtime_error("[OllamaClient] popen failed – is curl installed?");
    }

    std::string raw;
    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) raw += buf;
    pclose(pipe);
    std::remove(tmp_req.c_str());

    // ── Parse response ────────────────────────────────────────
    if (raw.empty()) {
        throw std::runtime_error(
            "[OllamaClient] Empty response – Ollama đang chạy chưa?\n"
            "  → Chạy: ollama serve\n"
            "  → Kiểm tra model: ollama list");
    }

    try {
        auto j = json::parse(raw);

        // Ollama trả error JSON nếu model không tồn tại
        if (j.contains("error")) {
            throw std::runtime_error(
                "[OllamaClient] Ollama error: " + j["error"].get<std::string>());
        }

        return j["message"]["content"].get<std::string>();

    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(
            std::string("[OllamaClient] JSON parse error: ") + e.what() +
            "\nRaw response: " + raw.substr(0, 200));
    }
}