#include <gtest/gtest.h>
#include "tools/tool.h"

// ── Helpers ───────────────────────────────────────────────────
static std::string calc(const std::string& expr) {
    CalculatorTool tool;
    auto res = tool.execute(expr);
    return res.value_or("nullopt");
}

// ── Basic operations ──────────────────────────────────────────
TEST(CalculatorTool, Addition)       { EXPECT_EQ(calc("2+3"),    "5"); }
TEST(CalculatorTool, Subtraction)    { EXPECT_EQ(calc("10-3"),   "7"); }
TEST(CalculatorTool, Multiplication) { EXPECT_EQ(calc("4*5"),   "20"); }
TEST(CalculatorTool, Division)       { EXPECT_EQ(calc("10/2"),   "5"); }

// ── Precedence ────────────────────────────────────────────────
TEST(CalculatorTool, Precedence_MulBeforeAdd) {
    EXPECT_EQ(calc("2+3*4"), "14");
}
TEST(CalculatorTool, Parentheses_OverridePrecedence) {
    EXPECT_EQ(calc("(2+3)*4"), "20");
}
TEST(CalculatorTool, NestedParentheses) {
    EXPECT_EQ(calc("((2+3)*2)-4"), "6");
}

// ── Floating point ────────────────────────────────────────────
TEST(CalculatorTool, FloatResult) {
    CalculatorTool tool;
    auto res = tool.execute("7/2");
    ASSERT_TRUE(res.has_value());
    EXPECT_NEAR(std::stod(res.value()), 3.5, 1e-9);
}

// ── Negative numbers ──────────────────────────────────────────
TEST(CalculatorTool, NegativeNumber) {
    CalculatorTool tool;
    auto res = tool.execute("-5+3");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "-2");
}

// ── JSON args ─────────────────────────────────────────────────
TEST(CalculatorTool, JsonArgs) {
    CalculatorTool tool;
    auto res = tool.execute(R"({"expression": "10 * 10"})");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "100");
}

// ── Error cases ───────────────────────────────────────────────
TEST(CalculatorTool, DivisionByZero) {
    auto res = calc("1/0");
    EXPECT_NE(res.find("Error"), std::string::npos);
}
TEST(CalculatorTool, EmptyInput) {
    CalculatorTool tool;
    EXPECT_FALSE(tool.execute("").has_value());
}
