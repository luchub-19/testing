#pragma once
#include <map>
#include <optional>
#include <string>

// ════════════════════════════════════════════════════════════════
//  Abstract Tool  (Strategy Pattern)
//  Mỗi tool nhận args dạng plain string (thường là JSON),
//  trả về std::nullopt nếu thất bại.
// ════════════════════════════════════════════════════════════════

class Tool {
public:
    virtual ~Tool() = default;
    virtual std::string getName()        const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::optional<std::string>  execute(const std::string& args) = 0;
};

// ── ExecTool ──────────────────────────────────────────────────
// Thực thi lệnh shell, trả về stdout.
// Args: lệnh shell thuần hoặc JSON {"command": "ls -la"}
class ExecTool : public Tool {
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};

// ── FileTool ──────────────────────────────────────────────────
// Đọc / ghi / liệt kê file.
// Args JSON: {"action":"read"|"write"|"append"|"list", "path":"...", "content":"..."}
class FileTool : public Tool {
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};

// ── WebSearchTool ─────────────────────────────────────────────
// Tìm kiếm web qua DuckDuckGo Instant Answer API (dùng curl).
// Args: query string thuần hoặc JSON {"query": "..."}
class WebSearchTool : public Tool {
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};

// ── MemoryTool ────────────────────────────────────────────────
// Key-value store trong bộ nhớ, tồn tại suốt vòng đời ToolRegistry.
// Args JSON: {"action":"store"|"recall"|"list"|"clear", "key":"...", "value":"..."}
class MemoryTool : public Tool {
    std::map<std::string, std::string> memory_;
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};

// ── CalculatorTool ────────────────────────────────────────────
// Tính biểu thức toán học cơ bản: + - * / ( )
// Args: biểu thức thuần "2*(3+4)" hoặc JSON {"expression": "2*(3+4)"}
class CalculatorTool : public Tool {
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};
