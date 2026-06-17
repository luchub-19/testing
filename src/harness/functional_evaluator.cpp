#include "evaluator.h"
#include <iostream>
#include <fstream>
#include <cstdlib>     // std::system
#include <cstdio>      // std::remove
#include <sys/wait.h>  // WIFEXITED, WEXITSTATUS (POSIX/Linux)

// Ghi traj JSON ra file tạm, chạy eval_script với file đó làm argument
// Trả về exit code của script (hoặc -1 nếu có lỗi)
int FunctionalEvaluator::runScript(const std::string& cmd,
                                    const std::string& traj_path) const {
    std::string full_cmd = cmd + " " + traj_path;
    int ret = std::system(full_cmd.c_str());

    if (ret == -1) {
        std::cerr << "[FunctionalEval] system() failed to fork.\n";
        return -1;
    }
    if (!WIFEXITED(ret)) {
        std::cerr << "[FunctionalEval] Script terminated by signal.\n";
        return -1;
    }
    return WEXITSTATUS(ret);
}

float FunctionalEvaluator::evaluate(const Trajectory& traj,
                                     const Environment& /*env*/,
                                     const Task&        task) {
    if (task.eval_script.empty()) {
        std::cerr << "[FunctionalEval] eval_script bị trống!\n";
        return 0.0f;
    }

    // Ghi trajectory ra /tmp  (task_id dùng làm phần tên file)
    std::string tmp_path = "/tmp/eval_" + traj.task_id + ".json";
    {
        std::ofstream ofs(tmp_path);
        if (!ofs) {
            std::cerr << "[FunctionalEval] Không thể tạo file tạm: " << tmp_path << "\n";
            return 0.0f;
        }
        ofs << traj.exportToJson();
    }

    int exit_code = runScript(task.eval_script, tmp_path);

    // Xóa file tạm dù script thành công hay thất bại
    std::remove(tmp_path.c_str());

    float score = (exit_code == 0) ? 1.0f : 0.0f;
    std::cout << "[FunctionalEval] exit code: " << exit_code
              << "  \u2192  score: " << score << "\n";
    return score;
}
