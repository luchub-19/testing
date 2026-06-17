#include "tool_registry.h"
#include <algorithm>
#include <iostream>
#include <sstream>

void ToolRegistry::registerTool(std::unique_ptr<Tool> tool) {
    std::string name = tool->getName();
    tools_[name] = std::move(tool);
    std::cout << "[ToolRegistry] Registered: " << name << "\n";
}

void ToolRegistry::setAllowedTools(const std::vector<std::string>& names) {
    allowed_tools_ = names;
}

std::optional<std::string> ToolRegistry::executeTool(const std::string& name,
                                                       const std::string& args) {
    // Kiểm tra whitelist nếu có
    if (!allowed_tools_.empty()) {
        bool permitted = std::find(allowed_tools_.begin(),
                                   allowed_tools_.end(), name) != allowed_tools_.end();
        if (!permitted) {
            std::cerr << "[ToolRegistry] Tool '" << name << "' is not in allowed list.\n";
            return std::nullopt;
        }
    }

    auto it = tools_.find(name);
    if (it == tools_.end()) {
        std::cerr << "[ToolRegistry] Unknown tool: '" << name << "'\n";
        return std::nullopt;
    }

    return it->second->execute(args);
}

std::string ToolRegistry::getToolDescriptions() const {
    std::ostringstream oss;
    for (const auto& [name, tool] : tools_) {
        // Bỏ qua tool không nằm trong whitelist
        if (!allowed_tools_.empty()) {
            bool ok = std::find(allowed_tools_.begin(),
                                allowed_tools_.end(), name) != allowed_tools_.end();
            if (!ok) continue;
        }
        oss << "- " << name << ": " << tool->getDescription() << "\n";
    }
    return oss.str();
}

bool ToolRegistry::hasTool(const std::string& name) const {
    return tools_.count(name) > 0;
}
