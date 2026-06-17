#pragma once
#include <filesystem>
#include <map>
#include <string>

// ════════════════════════════════════════════════════════════════
//  SkillLoader
//  Đọc tất cả file .md trong thư mục skills/ vào memory.
//  AgentLoop gọi selectSkill() để lấy system-prompt bổ sung
//  phù hợp với từ khóa của Task (task_planner, error_recovery, ...).
// ════════════════════════════════════════════════════════════════

class SkillLoader {
    std::filesystem::path              skill_dir_;
    std::map<std::string, std::string> skills_;   // filename stem -> noi dung .md
public:
    explicit SkillLoader(const std::string& dir = "skills");

    // Quét thư mục skill_dir_, load toàn bộ .md vào skills_
    void loadSkillsFromDisk();

    // Trả về nội dung skill phù hợp nhất với keyword.
    // Ưu tiên: exact name > partial name > keyword trong content.
    // Trả về "" nếu không tìm thấy.
    std::string selectSkill(const std::string& keyword) const;

    bool        empty() const { return skills_.empty(); }
    std::size_t size()  const { return skills_.size(); }
};
