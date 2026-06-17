#pragma once
#include <memory>
#include <string>
#include <vector>

#include "environment.h"
#include "evaluator.h"
#include "trajectory.h"
#include "../agent/agent_loop.h"

// ════════════════════════════════════════════════════════════════
//  HarnessRunner  (Observer + Composite)
//  Điều phối toàn bộ benchmark:
//   1. Setup environment
//   2. Chạy agent trên từng task (inject step_hook)
//   3. Đánh giá kết quả qua nhiều Evaluator
//   4. Export batch results ra JSON
// ════════════════════════════════════════════════════════════════

class HarnessRunner {
    std::unique_ptr<Environment>              env_;
    std::vector<std::unique_ptr<Evaluator>>   evaluators_;
    std::vector<Trajectory>                   batch_results_;

    // Observer callback — được inject vào AgentLoop làm step_hook
    void onStepRecorded(const Step& step);

public:
    explicit HarnessRunner(std::unique_ptr<Environment> env);

    // Thêm evaluator vào pipeline (Strategy Pattern)
    void addEvaluator(std::unique_ptr<Evaluator> ev);

    void setupEnvironment();
    void teardownEnvironment();

    // Chạy một agent trên một task; trả về Trajectory
    Trajectory runAgent(AgentLoop& loop, const Task& task);

    // Chạy agent trên toàn bộ danh sách tasks
    void runBatch(AgentLoop& loop, const std::vector<Task>& tasks);

    // Xuất kết quả + điểm đánh giá ra file JSON
    void exportBatchResults(const std::string& filepath) const;

    const std::vector<Trajectory>& results() const { return batch_results_; }
};
