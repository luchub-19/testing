#pragma once
#include <vector>
#include "../harness/trajectory.h"  // Step

// ════════════════════════════════════════════════════════════════
//  LoopDetector
//  AgentLoop gọi sau mỗi Step để kiểm tra xem agent có bị kẹt không.
//
//  Hai loại loop được phát hiện:
//   1. Repetition loop : cùng một tool+args lặp ≥ warning_threshold lần liên tiếp
//   2. Ping-pong loop  : luân phiên A→B→A→B trong critical_threshold steps gần nhất
// ════════════════════════════════════════════════════════════════

class LoopDetector {
    int warning_threshold_;   // default 3: repetition
    int critical_threshold_;  // default 4: ping-pong (cần ≥ 4 steps)
public:
    explicit LoopDetector(int warning = 3, int critical = 4)
        : warning_threshold_(warning), critical_threshold_(critical) {}

    // true nếu step cuối lặp lại ≥ warning_threshold lần liên tiếp
    bool detectLoop(const std::vector<Step>& history) const;

    // true nếu 4 steps cuối theo pattern A→B→A→B
    bool isPingPongLoop(const std::vector<Step>& history) const;
};
