#pragma once
#include <string>
#include <vector>
#include "task.h"

// ── Step ──────────────────────────────────────────────────────
// Một bước suy luận trong vòng lặp ReAct (Thought → Action → Observation)
struct Step {
    int         step_id     = 0;
    std::string thought;        // LLM nghĩ gì ở bước này
    std::string action_type;    // "tool" | "final_answer"
    std::string tool_name;      // tên tool được gọi (rỗng nếu final_answer)
    std::string tool_args;      // đầu vào truyền cho tool (JSON string)
    std::string tool_result;    // output của tool hoặc câu trả lời cuối
    int         tokens_used = 0;
    int         latency_ms  = 0;
};

// ── Trajectory ────────────────────────────────────────────────
// Toàn bộ lịch sử thực thi của agent trên một Task
class Trajectory {
public:
    std::string       task_id;
    std::string       model;
    bool              success       = false;
    int               total_tokens  = 0;
    int               total_time_ms = 0;
    std::vector<Step> steps;

    // Xuất toàn bộ trajectory ra JSON (dùng cho FunctionalEvaluator & log)
    std::string exportToJson() const;
};
