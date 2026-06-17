#include <gtest/gtest.h>
#include "agent/loop_detector.h"

// ── Helper: tạo Step nhanh ────────────────────────────────────
static Step makeStep(const std::string& tool, const std::string& args = "") {
    Step s;
    s.tool_name = tool;
    s.tool_args = args;
    return s;
}

class LoopDetectorTest : public ::testing::Test {
protected:
    LoopDetector det{3, 4};   // warning=3, critical=4
};

// ── detectLoop ────────────────────────────────────────────────
TEST_F(LoopDetectorTest, NoLoop_BelowThreshold) {
    std::vector<Step> h = { makeStep("exec"), makeStep("exec") };
    EXPECT_FALSE(det.detectLoop(h));
}

TEST_F(LoopDetectorTest, Loop_AtThreshold) {
    std::vector<Step> h = {
        makeStep("exec","ls"), makeStep("exec","ls"), makeStep("exec","ls")
    };
    EXPECT_TRUE(det.detectLoop(h));
}

TEST_F(LoopDetectorTest, NoLoop_DifferentArgs) {
    std::vector<Step> h = {
        makeStep("exec","ls"), makeStep("exec","pwd"), makeStep("exec","ls")
    };
    EXPECT_FALSE(det.detectLoop(h));
}

TEST_F(LoopDetectorTest, NoLoop_DifferentTools) {
    std::vector<Step> h = {
        makeStep("exec"), makeStep("file"), makeStep("exec")
    };
    EXPECT_FALSE(det.detectLoop(h));
}

TEST_F(LoopDetectorTest, Loop_BreaksOnDifferentStep) {
    // A B A A A  →  3 liên tiếp A ở cuối → loop
    std::vector<Step> h = {
        makeStep("exec","ls"), makeStep("file","read"),
        makeStep("exec","ls"), makeStep("exec","ls"), makeStep("exec","ls")
    };
    EXPECT_TRUE(det.detectLoop(h));
}

// ── isPingPongLoop ────────────────────────────────────────────
TEST_F(LoopDetectorTest, PingPong_Detected) {
    // A B A B
    std::vector<Step> h = {
        makeStep("exec","ls"), makeStep("file","read"),
        makeStep("exec","ls"), makeStep("file","read")
    };
    EXPECT_TRUE(det.isPingPongLoop(h));
}

TEST_F(LoopDetectorTest, PingPong_BelowCritical) {
    std::vector<Step> h = {
        makeStep("exec","ls"), makeStep("file","read"), makeStep("exec","ls")
    };
    EXPECT_FALSE(det.isPingPongLoop(h));
}

TEST_F(LoopDetectorTest, PingPong_NotAlternating) {
    // A A B B → không phải ping-pong
    std::vector<Step> h = {
        makeStep("exec"), makeStep("exec"),
        makeStep("file"), makeStep("file")
    };
    EXPECT_FALSE(det.isPingPongLoop(h));
}

TEST_F(LoopDetectorTest, PingPong_ThreeDistinctTools) {
    // A B C A → không phải A-B ping-pong
    std::vector<Step> h = {
        makeStep("exec"), makeStep("file"),
        makeStep("calc"), makeStep("exec")
    };
    EXPECT_FALSE(det.isPingPongLoop(h));
}
