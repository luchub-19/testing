#pragma once
#include <string>
#include <vector>
#include "trajectory.h"   // Step, Trajectory
#include "environment.h"  // Environment
#include "task.h"         // Task

// ════════════════════════════════════════════════════════════════
//  Abstract Evaluator  (Strategy Pattern)
//  evaluate() trả về điểm [0.0, 1.0]
// ════════════════════════════════════════════════════════════════

class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual float evaluate(const Trajectory& traj,
                           const Environment& env,
                           const Task&        task) = 0;
};

// ── KeywordEvaluator ──────────────────────────────────────────
// Đếm tỉ lệ keyword xuất hiện trong toàn bộ trajectory (thoughts + tool results).
//
// eval_script format: danh sách keyword cách nhau bằng dấu phẩy
//   "result, success, completed"
//
// Score = số_keyword_tìm_thấy / tổng_số_keyword
class KeywordEvaluator : public Evaluator {
    // Tách eval_script CSV → vector keywords (có trim whitespace)
    std::vector<std::string> parseKeywords(const std::string& script) const;
    // Tìm kiếm case-insensitive
    bool findKeyword(const std::string& text, const std::string& kw) const;
public:
    float evaluate(const Trajectory& traj,
                   const Environment& env,
                   const Task&        task) override;
};

// ── FunctionalEvaluator ───────────────────────────────────────
// Chạy một external script/executable để đánh giá kết quả agent.
//
// eval_script format: lệnh shell hoặc đường dẫn executable
//   "python3 tests/check_output.py"
//   "/usr/local/bin/my_checker"
//
// Framework sẽ gọi: <eval_script> <path_to_trajectory.json>
// Exit code 0  →  score 1.0  (pass)
// Exit code ≠ 0 →  score 0.0  (fail)
class FunctionalEvaluator : public Evaluator {
    // Ghi traj JSON ra file tạm, chạy script, trả exit code
    int runScript(const std::string& cmd, const std::string& traj_path) const;
public:
    float evaluate(const Trajectory& traj,
                   const Environment& env,
                   const Task&        task) override;
};
