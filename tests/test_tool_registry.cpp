#include <gtest/gtest.h>
#include "tools/tool.h"
#include "tools/tool_registry.h"

class ToolRegistryTest : public ::testing::Test {
protected:
    ToolRegistry reg;
    void SetUp() override {
        reg.registerTool(std::make_unique<CalculatorTool>());
        reg.registerTool(std::make_unique<MemoryTool>());
        reg.registerTool(std::make_unique<ExecTool>());
    }
};

// ── registerTool & executeTool ────────────────────────────────
TEST_F(ToolRegistryTest, ExecuteKnownTool) {
    auto res = reg.executeTool("calculator", "3*7");
    ASSERT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "21");
}

TEST_F(ToolRegistryTest, ExecuteUnknownTool_ReturnsNullopt) {
    auto res = reg.executeTool("nonexistent", "args");
    EXPECT_FALSE(res.has_value());
}

// ── hasTool ───────────────────────────────────────────────────
TEST_F(ToolRegistryTest, HasTool_True) {
    EXPECT_TRUE(reg.hasTool("calculator"));
    EXPECT_TRUE(reg.hasTool("memory"));
}
TEST_F(ToolRegistryTest, HasTool_False) {
    EXPECT_FALSE(reg.hasTool("web_search"));
}

// ── allowed_tools whitelist ────────────────────────────────────
TEST_F(ToolRegistryTest, AllowedTools_BlocksOthers) {
    reg.setAllowedTools({"calculator"});
    EXPECT_TRUE(reg.executeTool("calculator", "1+1").has_value());
    EXPECT_FALSE(reg.executeTool("exec", "ls").has_value());
    EXPECT_FALSE(reg.executeTool("memory",
        R"({"action":"list"})").has_value());
}

TEST_F(ToolRegistryTest, EmptyAllowedList_AllowsAll) {
    // allowed_tools_ rỗng = không giới hạn
    reg.setAllowedTools({});
    EXPECT_TRUE(reg.executeTool("calculator", "5+5").has_value());
    EXPECT_TRUE(reg.executeTool("exec", "echo hi").has_value());
}

// ── getToolDescriptions ───────────────────────────────────────
TEST_F(ToolRegistryTest, DescriptionsNotEmpty) {
    std::string desc = reg.getToolDescriptions();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("calculator"), std::string::npos);
    EXPECT_NE(desc.find("memory"),     std::string::npos);
}

TEST_F(ToolRegistryTest, DescriptionsRespectAllowedList) {
    reg.setAllowedTools({"calculator"});
    std::string desc = reg.getToolDescriptions();
    EXPECT_NE(desc.find("calculator"), std::string::npos);
    EXPECT_EQ(desc.find("exec"),       std::string::npos);
}

// ── MemoryTool store/recall qua registry ─────────────────────
TEST_F(ToolRegistryTest, MemoryTool_StoreRecall) {
    auto store = reg.executeTool("memory",
        R"({"action":"store","key":"pi","value":"3.14"})");
    ASSERT_TRUE(store.has_value());

    auto recall = reg.executeTool("memory",
        R"({"action":"recall","key":"pi"})");
    ASSERT_TRUE(recall.has_value());
    EXPECT_EQ(recall.value(), "3.14");
}
