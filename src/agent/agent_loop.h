#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../client/llm_client.h"
#include "../harness/trajectory.h"
#include "../tools/tool_registry.h"
#include "loop_detector.h"
#include "skill_loader.h"

// ════════════════════════════════════════════════════════════════
//  AgentLoop  —  vòng lặp ReAct (Observe → Think → Act)
//  Template Method Pattern: run() gọi observe/think/act tuần tự.
//  Observer Pattern: step_hook_ cho phép HarnessRunner nhận event.
// ════════════════════════════════════════════════════════════════

class AgentLoop {
    std::unique_ptr<LLMClient>          llm_;
    std::shared_ptr<ToolRegistry>       tools_;
    std::unique_ptr<SkillLoader>        skills_;
    std::unique_ptr<LoopDetector>       loop_detector_;
    std::function<void(const Step&)>    step_hook_;
    std::vector<std::string>            conversation_history_;

    // ── Template Method steps ─────────────────────────────────
    std::string observe(const Task& task) const;
    std::string think(const std::string& observation, const Task& task);
    Step        act(const std::string& llm_response, int step_id);

    // Phân tích response của LLM → (action_type, tool_name, tool_args, thought)
    void parseResponse(const std::string& response,
                       std::string& thought,
                       std::string& action_type,
                       std::string& tool_name,
                       std::string& tool_args) const;

    std::string buildSystemPrompt(const Task& task) const;

public:
    AgentLoop(std::unique_ptr<LLMClient>    llm,
              std::shared_ptr<ToolRegistry> tools,
              std::unique_ptr<SkillLoader>  skills,
              std::unique_ptr<LoopDetector> loop_detector);

    // Chạy agent trên một task, trả về toàn bộ Trajectory
    Trajectory run(const Task& task);

    // Đặt hook nhận thông báo sau mỗi Step (dùng bởi HarnessRunner)
    void setStepHook(std::function<void(const Step&)> hook);
};
