#include "tool.h"
#include <cstdio>
#include <nlohmann/json.hpp>
#include <sys/wait.h>

std::string ExecTool::getName()        const { return "exec"; }
std::string ExecTool::getDescription() const {
    return "Thực thi lệnh shell và trả về stdout. "
           "Args: chuỗi lệnh trực tiếp hoặc JSON {\"command\":\"...\"}";
}

std::optional<std::string> ExecTool::execute(const std::string& args) {
    // Parse args: thử JSON trước, fallback về raw command string
    std::string cmd = args;
    try {
        auto j = nlohmann::json::parse(args);
        if (j.contains("command")) cmd = j["command"].get<std::string>();
    } catch (...) {}

    if (cmd.empty()) return std::nullopt;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error: popen failed for command: " + cmd;

    std::string output;
    char buf[1024];
    while (fgets(buf, sizeof(buf), pipe)) output += buf;
    int rc = pclose(pipe);

    // Bỏ newline thừa ở cuối
    if (!output.empty() && output.back() == '\n') output.pop_back();

    // Nếu không có stdout, trả về exit code
    if (output.empty()) {
        return "(exit code: " + std::to_string(WEXITSTATUS(rc)) + ")";
    }
    return output;
}
