#include "evaluator.h"
#include <algorithm>
#include <iostream>
#include <sstream>

// Tách CSV → danh sách keyword, trim whitespace hai đầu
std::vector<std::string> KeywordEvaluator::parseKeywords(const std::string& script) const {
    std::vector<std::string> kws;
    std::istringstream ss(script);
    std::string token;

    while (std::getline(ss, token, ',')) {
        auto l = token.find_first_not_of(" \t\r\n");
        auto r = token.find_last_not_of(" \t\r\n");
        if (l != std::string::npos) {
            kws.push_back(token.substr(l, r - l + 1));
        }
    }
    return kws;
}

// Tìm kiếm substring không phân biệt hoa thường
bool KeywordEvaluator::findKeyword(const std::string& text, const std::string& kw) const {
    auto toLow = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    return toLow(text).find(toLow(kw)) != std::string::npos;
}

float KeywordEvaluator::evaluate(const Trajectory& traj,
                                  const Environment& /*env*/,
                                  const Task&        task) {
    auto kws = parseKeywords(task.eval_script);
    if (kws.empty()) {
        std::cout << "[KeywordEval] Không có keyword nào để kiểm tra.\n";
        return 0.0f;
    }

    // Gom toàn bộ text từ mọi step trong trajectory
    std::string corpus;
    for (const auto& step : traj.steps) {
        corpus += step.thought     + ' ';
        corpus += step.tool_result + ' ';
    }

    int found = 0;
    for (const auto& kw : kws) {
        if (findKeyword(corpus, kw)) {
            std::cout << "[KeywordEval] \u2713 \"" << kw << "\"\n";
            ++found;
        } else {
            std::cout << "[KeywordEval] \u2717 \"" << kw << "\"\n";
        }
    }

    float score = static_cast<float>(found) / static_cast<float>(kws.size());
    std::cout << "[KeywordEval] Score: " << score
              << "  (" << found << "/" << kws.size() << " keywords)\n";
    return score;
}
