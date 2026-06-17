// benchmark/run_eval.cpp
// Entry point: load tasks.json -> chay agent -> xuat ket qua
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "agent/agent_loop.h"
#include "agent/loop_detector.h"
#include "agent/skill_loader.h"
#include "client/ollama_client.h"
#include "harness/environment.h"
#include "harness/evaluator.h"
#include "harness/harness_runner.h"
#include "harness/task.h"
#include "tools/tool.h"             // ExecTool, FileTool, CalculatorTool, ...
#include "tools/tool_registry.h"

using json = nlohmann::json;

std::vector<Task> loadTasks(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open: " + path);
    auto j = json::parse(f);
    std::vector<Task> tasks;
    for (const auto& jt : j) {
        Task t;
        t.id          = jt.value("id",          "");
        t.instruction = jt.value("instruction",  "");
        t.eval_type   = jt.value("eval_type",    "keyword");
        t.eval_script = jt.value("eval_script",  "");
        t.max_steps   = jt.value("max_steps",    10);
        tasks.push_back(t);
    }
    std::cout << "[Main] Loaded " << tasks.size() << " task(s) from " << path << "\n";
    return tasks;
}

int main(int argc, char* argv[]) {
    std::string tasks_path  = "benchmark/tasks.json";
    std::string output_path = "benchmark/results.json";
    if (argc > 1) tasks_path  = argv[1];
    if (argc > 2) output_path = argv[2];

    // ── Tool Registry ─────────────────────────────────────────
    auto registry = std::make_shared<ToolRegistry>();
    registry->registerTool(std::make_unique<ExecTool>());
    registry->registerTool(std::make_unique<FileTool>());
    registry->registerTool(std::make_unique<CalculatorTool>());
    registry->registerTool(std::make_unique<WebSearchTool>());
    registry->registerTool(std::make_unique<MemoryTool>());

    // ── AgentLoop ─────────────────────────────────────────────
    AgentLoop loop(
        std::make_unique<OllamaClient>("gemma4:e4b"),
        registry,
        std::make_unique<SkillLoader>("skills"),
        std::make_unique<LoopDetector>(3, 4)
    );

    // ── HarnessRunner ─────────────────────────────────────────
    HarnessRunner harness(std::make_unique<NativeEnvironment>());
    harness.addEvaluator(std::make_unique<KeywordEvaluator>());
    harness.addEvaluator(std::make_unique<FunctionalEvaluator>());

    // ── Load & Run ────────────────────────────────────────────
    auto tasks = loadTasks(tasks_path);
    harness.runBatch(loop, tasks);
    harness.exportBatchResults(output_path);

    return 0;
}
