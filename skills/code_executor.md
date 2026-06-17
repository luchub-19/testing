# Code Executor Skill

Use this skill when the task requires writing and running code to get an answer.

## Strategy
1. Write the code to a temp file using `file` tool
2. Execute it using `exec` tool
3. Read stdout as the result
4. If it fails, check the error message and fix the code

## Example Workflow
```
Action: file
Action Input: {"action":"write","path":"/tmp/sol.py","content":"print(sum(range(1,101)))"}

Action: exec
Action Input: python3 /tmp/sol.py
```

## Supported Languages
- Python 3: `python3 /tmp/script.py`
- Bash:     `bash /tmp/script.sh`
- C++ (compile + run):
  ```
  g++ -std=c++17 -o /tmp/sol /tmp/sol.cpp && /tmp/sol
  ```

## Tips
- Always verify the output makes sense before giving final_answer
- For math problems, prefer the `calculator` tool over writing code
- Clean up temp files with `exec: rm /tmp/sol*` after you're done
