# Error Recovery Skill

Use this skill when a tool returns an error or unexpected result.

## Recovery Steps
1. Read the error message carefully
2. Try the simplest fix first (e.g., wrong path, wrong argument format)
3. If tool fails twice with same args, switch to an alternative tool
4. If stuck after 3 attempts, give a partial answer and explain what failed

## Common Fixes
- `exec` error: check if command exists with `which <cmd>`
- `file` read error: check path with `exec: ls <directory>`
- `calculator` error: simplify the expression into smaller parts
