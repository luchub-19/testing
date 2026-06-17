#pragma once
#include "tool.h"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// ════════════════════════════════════════════════════════════════
//  ToolRegistry  (Registry Pattern)
//  Lưu trữ và điều phối tất cả Tool.
//  AgentLoop giữ shared_ptr<ToolRegistry> để tra cứu + thực thi.
// ════════════════════════════════════════════════════════════════

class ToolRegistry {
    std::map<std::string, std::unique_ptr<Tool>> tools_;
    std::vector<std::string>                     allowed_tools_; // rỗng = cho phép tất cả

public:
    // Đăng ký một tool; tự lấy tên qua tool->getName()
    void registerTool(std::unique_ptr<Tool> tool);

    // Giới hạn tool nào được gọi (dùng cho sandbox / per-task config)
    void setAllowedTools(const std::vector<std::string>& names);

    // Tìm và chạy tool; trả nullopt nếu không tồn tại / bị chặn
    std::optional<std::string> executeTool(const std::string& name,
                                           const std::string& args);

    // Trả về danh sách tool + mô tả dạng text (nhúng vào system prompt)
    std::string getToolDescriptions() const;

    bool hasTool(const std::string& name) const;
};
