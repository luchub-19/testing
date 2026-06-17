#include "skill_loader.h"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

SkillLoader::SkillLoader(const std::string& dir)
    : skill_dir_(dir) {}

void SkillLoader::loadSkillsFromDisk() {
    if (!fs::exists(skill_dir_)) {
        std::cerr << "[SkillLoader] Directory not found: " << skill_dir_ << "\n";
        return;
    }

    skills_.clear();
    for (const auto& entry : fs::directory_iterator(skill_dir_)) {
        if (entry.path().extension() != ".md") continue;

        std::ifstream f(entry.path());
        if (!f) {
            std::cerr << "[SkillLoader] Cannot read: " << entry.path() << "\n";
            continue;
        }

        std::string content(std::istreambuf_iterator<char>(f),
                            std::istreambuf_iterator<char>());
        std::string stem = entry.path().stem().string();
        skills_[stem] = std::move(content);
        std::cout << "[SkillLoader] Loaded: " << stem << ".md\n";
    }

    std::cout << "[SkillLoader] " << skills_.size() << " skill(s) ready.\n";
}

std::string SkillLoader::selectSkill(const std::string& keyword) const {
    if (keyword.empty() || skills_.empty()) return "";

    // 1. Exact name match
    auto it = skills_.find(keyword);
    if (it != skills_.end()) return it->second;

    auto toLow = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    std::string kw_low = toLow(keyword);

    // 2. Partial name match (cả hai chiều)
    for (const auto& [name, content] : skills_) {
        std::string name_low = toLow(name);
        if (name_low.find(kw_low) != std::string::npos ||
            kw_low.find(name_low) != std::string::npos) {
            return content;
        }
    }

    // 3. Keyword xuất hiện trong nội dung skill
    for (const auto& [name, content] : skills_) {
        if (toLow(content).find(kw_low) != std::string::npos) {
            return content;
        }
    }

    return "";  // Không tìm thấy
}
