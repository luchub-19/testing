#include "agent_loop.h"
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

using json  = nlohmann::json;
using Clock = std::chrono::steady_clock;

// ── Constructor ───────────────────────────────────────────────
AgentLoop::AgentLoop(std::unique_ptr<LLMClient>    llm,
                     std::shared_ptr<ToolRegistry> tools,
                     std::unique_ptr<SkillLoader>  skills,
                     std::unique_ptr<LoopDetector> loop_detector)
    : llm_(std::move(llm))
    , tools_(std::move(tools))
    , skills_(std::move(skills))
    , loop_detector_(std::move(loop_detector)) {

    if (skills_) skills_->loadSkillsFromDisk();
}

void AgentLoop::setStepHook(std::function<void(const Step&)> hook) {
    step_hook_ = std::move(hook);
}

// ── System prompt ─────────────────────────────────────────────
std::string AgentLoop::buildSystemPrompt(const Task& task) const {
    std::ostringstream oss;
    oss << "You are a ReAct agent. Solve the task step by step.\n\n"
        << "## Available Tools\n"
        << tools_->getToolDescriptions() << "\n"
        << "## Response Format (follow EXACTLY)\n"
        << "Thought: <your reasoning>\n"
        << "Action: <tool_name>  OR  Action: final_answer\n"
        << "Action Input: <args string or JSON>\n\n"
        << "When you have the final answer, use:\n"
        << "Action: final_answer\n"
        << "Action Input: <your answer>\n";

    // Thêm skill instruction nếu có
    if (skills_ && !skills_->empty()) {
        std::string skill = skills_->selectSkill(task.instruction);
        if (!skill.empty()) oss << "\n## Additional Guidance\n" << skill << "\n";
    }
    return oss.str();
}

// ── Observe ───────────────────────────────────────────────────
std::string AgentLoop::observe(const Task& task) const {
    return "Task: " + task.instruction;
}

// ── Think ─────────────────────────────────────────────────────
std::string AgentLoop::think(const std::string& observation, const Task& task) {
    std::string sys = buildSystemPrompt(task);
    // Đẩy system prompt vào lần đầu
    if (conversation_history_.empty()) {
        conversation_history_.push_back(sys);
    }
    return llm_->chat(observation, conversation_history_);
}

// ── Parse LLM response ────────────────────────────────────────
void AgentLoop::parseResponse(const std::string& response,
                               std::string& thought,
                               std::string& action_type,
                               std::string& tool_name,
                               std::string& tool_args) const {
    thought = action_type = tool_name = tool_args = "";

    auto lineOf = [&](const std::string& prefix) -> std::string {
        auto pos = response.find(prefix);
        if (pos == std::string::npos) return "";
        auto start = pos + prefix.size();
        auto end   = response.find('\n', start);
        std::string val = response.substr(start, end == std::string::npos
                                                     ? std::string::npos
                                                     : end - start);
        // trim
        while (!val.empty() && val.front() == ' ') val.erase(val.begin());
        while (!val.empty() && val.back()  == '\r') val.pop_back();
        return val;
    };

    thought     = lineOf("Thought:");
    std::string action = lineOf("Action:");
    tool_args   = lineOf("Action Input:");

    if (action == "final_answer") {
        action_type = "final_answer";
        tool_name   = "";
    } else {
        action_type = "tool";
        tool_name   = action;
    }
}

// ── Act ───────────────────────────────────────────────────────
Step AgentLoop::act(const std::string& llm_response, int step_id) {
    Step step;
    step.step_id = step_id;

    parseResponse(llm_response,
                  step.thought,
                  step.action_type,
                  step.tool_name,
                  step.tool_args);

    if (step.action_type == "final_answer") {
        step.tool_result = step.tool_args;
        return step;
    }

    auto t0 = Clock::now();
    auto result = tools_->executeTool(step.tool_name, step.tool_args);
    auto t1 = Clock::now();

    step.latency_ms = (int)std::chrono::duration_cast<
                          std::chrono::milliseconds>(t1 - t0).count();
    step.tool_result = result.value_or("[no result]");

    std::cout << "[AgentLoop] Step " << step_id
              << " | " << step.tool_name
              << " | " << step.latency_ms << "ms\n";
    return step;
}

// ── Main ReAct loop ───────────────────────────────────────────
Trajectory AgentLoop::run(const Task& task) {
    Trajectory traj;
    traj.task_id = task.id;
    conversation_history_.clear();

    auto run_start = Clock::now();

    for (int i = 0; i < task.max_steps; ++i) {
        // Observe
        std::string observation = (i == 0)
            ? observe(task)
            : "Observation: " + traj.steps.back().tool_result;

        // Think
        std::string response = think(observation, task);

        // Cập nhật history
        conversation_history_.push_back(observation);
        conversation_history_.push_back(response);

        // Act
        Step step = act(response, i + 1);
        traj.steps.push_back(step);

        // Thông báo cho Observer (HarnessRunner)
        if (step_hook_) step_hook_(step);

        // Loop detection
        if (loop_detector_) {
            if (loop_detector_->detectLoop(traj.steps) ||
                loop_detector_->isPingPongLoop(traj.steps)) {
                std::cerr << "[AgentLoop] Loop detected — stopping early.\n";
                break;
            }
        }

        // Kết thúc nếu agent trả final_answer
        if (step.action_type == "final_answer") {
            traj.success = true;
            break;
        }
    }

    auto run_end = Clock::now();
    traj.total_time_ms = (int)std::chrono::duration_cast<
                             std::chrono::milliseconds>(run_end - run_start).count();
    return traj;
}
