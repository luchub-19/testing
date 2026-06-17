#include <gtest/gtest.h>
#include "harness/evaluator.h"
#include "harness/environment.h"
#include "harness/task.h"

// NativeEnvironment dùng như env stub (evaluator không gọi method nào của nó)
class KeywordEvalTest : public ::testing::Test {
protected:
    KeywordEvaluator ev;
    NativeEnvironment env{"test_ws"};   // không gọi setup()/teardown()

    // Helper: tạo Trajectory với nội dung tuỳ ý
    Trajectory makeTraj(const std::string& thought,
                        const std::string& tool_result = "") {
        Trajectory t;
        t.task_id = "kw_test";
        Step s;
        s.thought     = thought;
        s.tool_result = tool_result;
        t.steps.push_back(s);
        return t;
    }

    Task makeTask(const std::string& eval_script) {
        Task t;
        t.eval_type   = "keyword";
        t.eval_script = eval_script;
        return t;
    }
};

// ── Score tests ───────────────────────────────────────────────
TEST_F(KeywordEvalTest, AllKeywordsFound_Score1) {
    auto traj = makeTraj("the result is 42", "success completed");
    auto task = makeTask("42, success, completed");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 1.0f);
}

TEST_F(KeywordEvalTest, HalfKeywordsFound_Score0_5) {
    auto traj = makeTraj("the result is 42");
    auto task = makeTask("42, missing_keyword");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 0.5f);
}

TEST_F(KeywordEvalTest, NoKeywordsFound_Score0) {
    auto traj = makeTraj("completely unrelated output");
    auto task = makeTask("alpha, beta, gamma");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 0.0f);
}

TEST_F(KeywordEvalTest, EmptyEvalScript_Score0) {
    auto traj = makeTraj("any output");
    auto task = makeTask("");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 0.0f);
}

// ── Case-insensitive ──────────────────────────────────────────
TEST_F(KeywordEvalTest, CaseInsensitive) {
    auto traj = makeTraj("The Answer Is SUCCESS");
    auto task = makeTask("answer, success");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 1.0f);
}

// ── Keyword in tool_result ────────────────────────────────────
TEST_F(KeywordEvalTest, KeywordInToolResult) {
    auto traj = makeTraj("running tool", "output: DONE");
    auto task = makeTask("DONE");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 1.0f);
}

// ── Whitespace trimming in keywords ──────────────────────────
TEST_F(KeywordEvalTest, WhitespaceTrimmedKeywords) {
    auto traj = makeTraj("result is 100");
    auto task = makeTask("  result  ,  100  ");
    EXPECT_FLOAT_EQ(ev.evaluate(traj, env, task), 1.0f);
}

// ── Multi-step trajectory ─────────────────────────────────────
TEST_F(KeywordEvalTest, SearchesAllSteps) {
    Trajectory t;
    t.task_id = "multi";
    Step s1; s1.thought = "first step";  s1.tool_result = "partial";
    Step s2; s2.thought = "second step"; s2.tool_result = "answer found";
    t.steps = {s1, s2};

    Task task = makeTask("partial, answer");
    EXPECT_FLOAT_EQ(ev.evaluate(t, env, task), 1.0f);
}
