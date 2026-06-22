#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "harness/trajectory.h"

using json = nlohmann::json;

// ── Helper: tạo Trajectory mẫu ───────────────────────────────
static Trajectory makeSample() {
    Trajectory t;
    t.task_id       = "task_test_01";
    t.model         = "gemma4:e4b";
    t.success       = true;
    t.total_tokens  = 512;
    t.total_time_ms = 1234;

    Step s1;
    s1.step_id     = 1;
    s1.thought     = "I need to calculate";
    s1.action_type = "tool";
    s1.tool_name   = "calculator";
    s1.tool_args   = "2+3";
    s1.tool_result = "5";
    s1.tokens_used = 100;
    s1.latency_ms  = 300;

    Step s2;
    s2.step_id     = 2;
    s2.thought     = "The answer is 5";
    s2.action_type = "final_answer";
    s2.tool_result = "The answer is 5";

    t.steps = {s1, s2};
    return t;
}

// ── exportToJson tests ────────────────────────────────────────
TEST(Trajectory, ExportContainsTopLevelFields) {
    auto t   = makeSample();
    auto str = t.exportToJson();
    auto j   = json::parse(str);

    EXPECT_EQ(j["task_id"].get<std::string>(),  "task_test_01");
    EXPECT_EQ(j["model"].get<std::string>(),     "gemma4:e4b");
    EXPECT_EQ(j["success"].get<bool>(),          true);
    EXPECT_EQ(j["total_tokens"].get<int>(),      512);
    EXPECT_EQ(j["total_time_ms"].get<int>(),     1234);
}

TEST(Trajectory, ExportContainsSteps) {
    auto t = makeSample();
    auto j = json::parse(t.exportToJson());

    ASSERT_EQ(j["steps"].size(), 2u);

    auto& s0 = j["steps"][0];
    EXPECT_EQ(s0["step_id"].get<int>(),            1);
    EXPECT_EQ(s0["tool_name"].get<std::string>(),  "calculator");
    EXPECT_EQ(s0["tool_result"].get<std::string>(),"5");
    EXPECT_EQ(s0["tokens_used"].get<int>(),        100);
}

TEST(Trajectory, ExportIsValidJson) {
    auto t = makeSample();
    // dùng auto để tránh warning "ignoring return value"
    EXPECT_NO_THROW({
        auto j = nlohmann::json::parse(t.exportToJson());
        (void)j;
    });
}

TEST(Trajectory, EmptyTrajectory) {
    Trajectory t;
    t.task_id = "empty";
    auto j = json::parse(t.exportToJson());
    EXPECT_EQ(j["steps"].size(), 0u);
    EXPECT_FALSE(j["success"].get<bool>());
}