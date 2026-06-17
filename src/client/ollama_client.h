#pragma once
#include "llm_client.h"
#include <string>

// ════════════════════════════════════════════════════════════════
//  OllamaClient  —  concrete LLMClient cho Ollama local API
//  Gọi POST http://<base_url>/api/chat với JSON body.
//  Dùng cpp-httplib (header-only).
// ════════════════════════════════════════════════════════════════

class OllamaClient : public LLMClient {
    std::string base_url_;    // "http://localhost:11434"
    std::string model_name_;  // "gemma4:e4b"
    float       temperature_;
    int         max_tokens_;

    void handleError(int response_code) const;

public:
    explicit OllamaClient(const std::string& model    = "gemma4:e4b",
                          const std::string& base_url = "http://localhost:11434",
                          float temperature           = 0.7f,
                          int   max_tokens            = 2048);

    std::string chat(const std::string&              prompt,
                     const std::vector<std::string>& history) override;
};
