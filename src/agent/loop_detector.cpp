#include "loop_detector.h"
#include <iostream>

bool LoopDetector::detectLoop(const std::vector<Step>& history) const {
    int n = (int)history.size();
    if (n < warning_threshold_) return false;

    // Đếm số lần step cuối (tool_name + tool_args giống nhau) xuất hiện liên tiếp
    const auto& last = history.back();
    int count = 0;
    for (int i = n - 1; i >= 0; --i) {
        if (history[i].tool_name == last.tool_name &&
            history[i].tool_args == last.tool_args) {
            ++count;
        } else {
            break;
        }
    }

    if (count >= warning_threshold_) {
        std::cerr << "[LoopDetector] WARNING: \"" << last.tool_name
                  << "\" lặp " << count << " lần liên tiếp.\n";
        return true;
    }
    return false;
}

bool LoopDetector::isPingPongLoop(const std::vector<Step>& history) const {
    int n = (int)history.size();
    if (n < critical_threshold_) return false;

    // Kiểm tra pattern A->B->A->B trong 4 step cuối
    const auto& s1 = history[n - 1];  // B
    const auto& s2 = history[n - 2];  // A
    const auto& s3 = history[n - 3];  // B
    const auto& s4 = history[n - 4];  // A

    bool odd_match  = (s1.tool_name == s3.tool_name && s1.tool_args == s3.tool_args);
    bool even_match = (s2.tool_name == s4.tool_name && s2.tool_args == s4.tool_args);
    bool alternates = (s1.tool_name != s2.tool_name);

    if (odd_match && even_match && alternates) {
        std::cerr << "[LoopDetector] PING-PONG: \""
                  << s2.tool_name << "\" <-> \"" << s1.tool_name << "\"\n";
        return true;
    }
    return false;
}
