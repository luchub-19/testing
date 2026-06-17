#include "trajectory.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string Trajectory::exportToJson() const {
    json steps_arr = json::array();
    for (const auto& s : steps) {
        steps_arr.push_back({
            {"step_id",     s.step_id},
            {"thought",     s.thought},
            {"action_type", s.action_type},
            {"tool_name",   s.tool_name},
            {"tool_args",   s.tool_args},
            {"tool_result", s.tool_result},
            {"tokens_used", s.tokens_used},
            {"latency_ms",  s.latency_ms}
        });
    }

    json j = {
        {"task_id",       task_id},
        {"model",         model},
        {"success",       success},
        {"total_tokens",  total_tokens},
        {"total_time_ms", total_time_ms},
        {"steps",         steps_arr}
    };

    return j.dump(2);
}
