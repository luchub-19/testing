#pragma once
#include <string>
#include <vector>

// ════════════════════════════════════════════════════════════════
//  Abstract LLMClient
//  Concrete implementations: OllamaClient, (OpenAI, Gemini, ...)
// ════════════════════════════════════════════════════════════════

class LLMClient {
public:
    virtual ~LLMClient() = default;

    // Gửi prompt + lịch sử hội thoại, trả về response text của model.
    // history: danh sách các turn trước, xen kẽ user/assistant.
    virtual std::string chat(const std::string&              prompt,
                             const std::vector<std::string>& history) = 0;
};
