#include "tool.h"
#include <nlohmann/json.hpp>
#include <sstream>

std::string MemoryTool::getName()        const { return "memory"; }
std::string MemoryTool::getDescription() const {
    return "Lưu/đọc dữ liệu vào bộ nhớ agent trong phiên làm việc. "
           "Args JSON: {\"action\":\"store\"|\"recall\"|\"list\"|\"clear\", "
           "\"key\":\"...\", \"value\":\"...\"}";
}

std::optional<std::string> MemoryTool::execute(const std::string& args) {
    try {
        auto j = nlohmann::json::parse(args);
        std::string action = j.value("action", "");

        // ── store ─────────────────────────────────────────────
        if (action == "store") {
            std::string key = j.value("key",   "");
            std::string val = j.value("value", "");
            if (key.empty()) return "Error: 'key' is required";
            memory_[key] = val;
            return "Stored: " + key;
        }

        // ── recall ────────────────────────────────────────────
        if (action == "recall") {
            std::string key = j.value("key", "");
            if (key.empty()) return "Error: 'key' is required";
            auto it = memory_.find(key);
            if (it == memory_.end()) return "Not found: " + key;
            return it->second;
        }

        // ── list ──────────────────────────────────────────────
        if (action == "list") {
            if (memory_.empty()) return "(memory is empty)";
            std::ostringstream oss;
            for (const auto& [k, v] : memory_) oss << k << ": " << v << "\n";
            return oss.str();
        }

        // ── clear ─────────────────────────────────────────────
        if (action == "clear") {
            size_t n = memory_.size();
            memory_.clear();
            return "Cleared " + std::to_string(n) + " entries";
        }

        return "Error: unknown action '" + action + "'";

    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
