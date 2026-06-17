#include "harness_runner.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

HarnessRunner::HarnessRunner(std::unique_ptr<Environment> env)
    : env_(std::move(env)) {}

void HarnessRunner::addEvaluator(std::unique_ptr<Evaluator> ev) {
    evaluators_.push_back(std::move(ev));
}

void HarnessRunner::setupEnvironment() {
    if (env_) env_->setup();
}

void HarnessRunner::teardownEnvironment() {
    if (env_) env_->teardown();
}

// ── Observer callback ─────────────────────────────────────────
void HarnessRunner::onStepRecorded(const Step& step) {
    std::cout << "  [Step " << step.step_id << "] "
              << step.action_type;
    if (!step.tool_name.empty()) std::cout << " -> " << step.tool_name;
    std::cout << "\n";
}

// ── Run single agent ──────────────────────────────────────────
Trajectory HarnessRunner::runAgent(AgentLoop& loop, const Task& task) {
    // Inject Observer hook
    loop.setStepHook([this](const Step& s) { onStepRecorded(s); });

    std::cout << "\n[Harness] Running task: " << task.id << "\n";
    Trajectory traj = loop.run(task);

    // Chạy tất cả evaluators
    std::cout << "[Harness] Evaluating...\n";
    for (auto& ev : evaluators_) {
        float score = ev->evaluate(traj, *env_, task);
        (void)score; // score được print trong evaluator
    }

    batch_results_.push_back(traj);
    return traj;
}

// ── Run batch ─────────────────────────────────────────────────
void HarnessRunner::runBatch(AgentLoop& loop, const std::vector<Task>& tasks) {
    setupEnvironment();
    for (const auto& task : tasks) {
        runAgent(loop, task);
    }
    teardownEnvironment();
}

// ── Export results ────────────────────────────────────────────
void HarnessRunner::exportBatchResults(const std::string& filepath) const {
    json out = json::array();
    for (const auto& traj : batch_results_) {
        out.push_back(json::parse(traj.exportToJson()));
    }

    std::ofstream f(filepath);
    if (!f) {
        std::cerr << "[Harness] Cannot write: " << filepath << "\n";
        return;
    }
    f << out.dump(2);
    std::cout << "[Harness] Results saved to: " << filepath << "\n";
}
