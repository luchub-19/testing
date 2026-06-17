#include "tool.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

std::string FileTool::getName()        const { return "file"; }
std::string FileTool::getDescription() const {
    return "Đọc/ghi/liệt kê file. "
           "Args JSON: {\"action\":\"read\"|\"write\"|\"append\"|\"list\", "
           "\"path\":\"...\", \"content\":\"...\"}";
}

std::optional<std::string> FileTool::execute(const std::string& args) {
    try {
        auto j = nlohmann::json::parse(args);
        std::string action = j.value("action", "read");
        std::string path   = j.value("path",   "");

        if (path.empty()) return "Error: 'path' is required";

        // ── read ──────────────────────────────────────────────
        if (action == "read") {
            std::ifstream f(path);
            if (!f) return "Error: cannot open " + path;
            return std::string(std::istreambuf_iterator<char>(f),
                               std::istreambuf_iterator<char>());
        }

        // ── write ─────────────────────────────────────────────
        if (action == "write") {
            std::string content = j.value("content", "");
            std::ofstream f(path, std::ios::trunc);
            if (!f) return "Error: cannot write to " + path;
            f << content;
            return "Written " + std::to_string(content.size()) + " bytes to " + path;
        }

        // ── append ────────────────────────────────────────────
        if (action == "append") {
            std::string content = j.value("content", "");
            std::ofstream f(path, std::ios::app);
            if (!f) return "Error: cannot append to " + path;
            f << content;
            return "Appended " + std::to_string(content.size()) + " bytes to " + path;
        }

        // ── list ──────────────────────────────────────────────
        if (action == "list") {
            if (!fs::exists(path)) return "Error: path not found: " + path;
            std::string result;
            for (const auto& entry : fs::directory_iterator(path)) {
                result += entry.path().filename().string();
                result += fs::is_directory(entry) ? "/\n" : "\n";
            }
            return result.empty() ? "(empty)" : result;
        }

        return "Error: unknown action '" + action + "'";

    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
