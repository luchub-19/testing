# AI Agent Framework – OOP Project 2026

**Sinh viên:** 25127329 – 25127076  
**Môn học:** Lập Trình Hướng Đối Tượng (C++)  
**Mô hình:** ReAct Agent + Tool Registry + Evaluator Harness

---

## Tổng quan kiến trúc

Framework mô phỏng một AI Agent theo vòng lặp **ReAct** (Reason → Act → Observe):

```
Task ──► AgentLoop ──► LLMClient (Ollama/gemma4)
              │
              ├──► ToolRegistry ──► [ExecTool | FileTool | CalculatorTool
              │                      WebSearchTool | MemoryTool]
              ├──► SkillLoader  (system prompt bổ sung từ .md)
              └──► LoopDetector (phát hiện agent bị kẹt)

Trajectory ──► HarnessRunner ──► [KeywordEvaluator | FunctionalEvaluator]
                                        └──► results.json
```

**Design Patterns áp dụng:**
| Pattern | Nơi dùng |
|---------|----------|
| Strategy | `Evaluator`, `Tool` |
| Template Method | `AgentLoop::run()` |
| Registry | `ToolRegistry` |
| Observer | `AgentLoop::step_hook_` → `HarnessRunner` |

---

## Yêu cầu hệ thống

| Phần mềm | Phiên bản | Cài đặt (Arch Linux) |
|----------|-----------|----------------------|
| CMake | ≥ 3.17 | `sudo pacman -S cmake` |
| GCC/Clang | C++17 | `sudo pacman -S gcc` |
| nlohmann-json | bất kỳ | `sudo pacman -S nlohmann-json` |
| cpp-httplib | bất kỳ | `sudo pacman -S cpp-httplib` |
| Google Test | bất kỳ | `sudo pacman -S gtest` |
| Ollama | bất kỳ | Xem [ollama.ai](https://ollama.ai) |

---

## Cài đặt & Build

```bash
# 1. Cài dependencies
sudo pacman -S cmake gcc nlohmann-json cpp-httplib gtest

# 2. Cài Ollama + pull model
curl -fsSL https://ollama.ai/install.sh | sh
ollama pull gemma4:e4b   # model dùng trong project

# 3. Clone / giải nén project
cd Agent_25127329_25127076

# 4. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Sau khi build thành công, thư mục `build/` chứa:
- `run_eval`   – executable chạy benchmark
- `run_tests`  – executable chạy unit tests

---

## Cách chạy

### Chạy benchmark

```bash
# Đảm bảo Ollama đang chạy
ollama serve &

# Từ thư mục gốc project
cd build
./run_eval ../benchmark/tasks.json ../benchmark/results.json
```

Kết quả được lưu vào `benchmark/results.json`.

### Tuỳ chỉnh tasks

Sửa `benchmark/tasks.json`:

```json
[
  {
    "id": "my_task_01",
    "instruction": "What is 123 * 456?",
    "eval_type": "keyword",
    "eval_script": "56088",
    "max_steps": 5
  },
  {
    "id": "my_task_02",
    "instruction": "Write hello world to /tmp/hw.txt then read it",
    "eval_type": "keyword",
    "eval_script": "hello world",
    "max_steps": 8
  }
]
```

**eval_type options:**
- `"keyword"` – kiểm tra từ khoá CSV trong trajectory
- `"functional"` – chạy script ngoài: `<eval_script> <traj.json>`, exit 0 = pass

---

## Chạy unit tests

```bash
cd build
./run_tests                          # chạy tất cả
./run_tests --gtest_filter="Calcu*"  # chạy riêng một nhóm
./run_tests --gtest_filter="*Json*"  # chạy test có "Json" trong tên
ctest --output-on-failure            # qua CTest
```

**Test coverage:**

| File test | Covers |
|-----------|--------|
| `test_calculator` | operators, precedence, parens, negative, JSON args, errors |
| `test_loop_detector` | repetition threshold, ping-pong pattern, edge cases |
| `test_trajectory` | `exportToJson()` format, all fields, empty trajectory |
| `test_keyword_evaluator` | scoring, case-insensitive, multi-step, whitespace trim |
| `test_tool_registry` | register, execute, whitelist, `hasTool`, descriptions |

---

## Cấu trúc file

```
Agent_25127329_25127076/
├── CMakeLists.txt
├── README.md
├── benchmark/
│   ├── tasks.json          ← task đầu vào
│   ├── results.json        ← sinh ra sau khi chạy
│   └── run_eval.cpp        ← entry point
├── skills/
│   ├── task_planner.md
│   ├── error_recovery.md
│   └── code_executor.md
├── src/
│   ├── harness/
│   │   ├── task.h
│   │   ├── trajectory.h/.cpp
│   │   ├── environment.h
│   │   ├── native_environment.cpp
│   │   ├── sandbox_environment.cpp
│   │   ├── evaluator.h
│   │   ├── keyword_evaluator.cpp
│   │   ├── functional_evaluator.cpp
│   │   └── harness_runner.h/.cpp
│   ├── client/
│   │   ├── llm_client.h        ← abstract
│   │   └── ollama_client.h/.cpp
│   ├── tools/
│   │   ├── tool.h              ← abstract + 5 concrete
│   │   ├── tool_registry.h/.cpp
│   │   ├── exec_tool.cpp
│   │   ├── file_tool.cpp
│   │   ├── calculator_tool.cpp
│   │   ├── web_search_tool.cpp
│   │   └── memory_tool.cpp
│   └── agent/
│       ├── loop_detector.h/.cpp
│       ├── skill_loader.h/.cpp
│       └── agent_loop.h/.cpp
├── tests/
│   ├── test_calculator.cpp
│   ├── test_loop_detector.cpp
│   ├── test_trajectory.cpp
│   ├── test_keyword_evaluator.cpp
│   └── test_tool_registry.cpp
└── docs/
    ├── class_diagram.png
    ├── sequence_diagram_agent.png
    └── component_diagram.png
```

---

## Thêm tool mới

1. Khai báo class trong `src/tools/tool.h`:
```cpp
class MyTool : public Tool {
public:
    std::string getName()        const override;
    std::string getDescription() const override;
    std::optional<std::string>  execute(const std::string& args) override;
};
```

2. Tạo `src/tools/my_tool.cpp` với implementation.

3. Thêm vào `CMakeLists.txt` (dòng `TOOLS_SRC`):
```cmake
src/tools/my_tool.cpp
```

4. Đăng ký trong `benchmark/run_eval.cpp`:
```cpp
registry->registerTool(std::make_unique<MyTool>());
```

---

## Thêm skill mới

Tạo file `.md` trong `skills/`:
```markdown
# My Skill Name

Mô tả khi nào skill này được dùng.

## Strategy
1. Bước 1
2. Bước 2
```

`SkillLoader` tự động load file này. `AgentLoop` sẽ inject nội dung vào system prompt khi keyword của task match với tên file.

---

## Lưu ý

- `WebSearchTool` yêu cầu `curl` và kết nối internet.
- `SandboxEnvironment` dùng `mkdtemp` (POSIX/Linux only).
- `FunctionalEvaluator` dùng `WEXITSTATUS` (POSIX/Linux only).
- Model mặc định: `gemma4:e4b` qua Ollama tại `localhost:11434`.
