# Architecture Notes

## Diagrams (xem file .png cùng thư mục)

- **class_diagram.png**       – toàn bộ class và quan hệ kế thừa/composition
- **sequence_diagram_agent.png** – luồng một lần gọi AgentLoop::run()
- **component_diagram.png**   – tổng quan các module và dependency

## Sequence: AgentLoop::run(task)

```
HarnessRunner          AgentLoop          LLMClient         ToolRegistry
     |                     |                  |                  |
     |── runAgent(loop,task)►|                  |                  |
     |    inject step_hook   |                  |                  |
     |                     |── observe(task) ──►|                  |
     |                     |◄── observation ───|                  |
     |                     |── think(obs) ────►|                  |
     |                     |                  |── POST /api/chat ►|
     |                     |◄── llm_response ─|                  |
     |                     |── act(response) ─────────────────►  |
     |                     |                                     |── execute()
     |                     |◄── tool_result ──────────────────── |
     |◄── step_hook(step) ─|                  |                  |
     |    [loop detection]  |                  |                  |
     |    [repeat N times]  |                  |                  |
     |◄── Trajectory ───── |                  |                  |
```

## Key Design Decisions

1. **tool.h declares all 5 concrete tools** – giữ cho mỗi file .cpp chỉ có implementation,
   không cần header riêng cho từng tool. Matches file structure pattern.

2. **MemoryTool có state (memory_ map)** – được lưu trong ToolRegistry qua unique_ptr,
   nên state tồn tại suốt lifetime của registry trong một session.

3. **step_hook_ là std::function** – Observer pattern linh hoạt, không cần HarnessRunner
   kế thừa interface nào. AgentLoop không biết về HarnessRunner.

4. **LoopDetector injected vào AgentLoop** – dễ thay đổi threshold hoặc thuật toán
   mà không sửa AgentLoop core logic.
