#pragma once
#include <string>

// Nếu Task đã được định nghĩa trong trajectory.h, xóa file này
// và thêm #include "task.h" vào đó là xong.
struct Task {
    std::string id;
    std::string instruction;   // Prompt gốc cho agent
    std::string eval_type;     // "keyword" | "functional"
    std::string eval_script;   // Keywords CSV hoặc đường dẫn script đánh giá
    int         max_steps = 10;
};
