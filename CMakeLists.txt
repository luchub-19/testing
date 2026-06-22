cmake_minimum_required(VERSION 3.17)
project(AI_Agent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ──────────────────────────────────────────────
# pacman -S nlohmann-json gtest   |   apt install nlohmann-json3-dev libgtest-dev
find_package(nlohmann_json REQUIRED)
find_package(GTest         REQUIRED)

# ── Source groups ─────────────────────────────────────────────
set(HARNESS_SRC
    src/harness/trajectory.cpp
    src/harness/native_environment.cpp
    src/harness/sandbox_environment.cpp
    src/harness/keyword_evaluator.cpp
    src/harness/functional_evaluator.cpp
    src/harness/harness_runner.cpp
)
set(CLIENT_SRC
    src/client/ollama_client.cpp   # dùng curl qua popen, không cần httplib
)
set(TOOLS_SRC
    src/tools/tool_registry.cpp
    src/tools/exec_tool.cpp
    src/tools/file_tool.cpp
    src/tools/web_search_tool.cpp
    src/tools/memory_tool.cpp
    src/tools/calculator_tool.cpp
)
set(AGENT_SRC
    src/agent/loop_detector.cpp
    src/agent/skill_loader.cpp
    src/agent/agent_loop.cpp
)
set(CORE_SRC ${HARNESS_SRC} ${TOOLS_SRC} ${AGENT_SRC})

# ── Benchmark executable ──────────────────────────────────────
add_executable(run_eval
    benchmark/run_eval.cpp
    ${CORE_SRC}
    ${CLIENT_SRC}
)
target_include_directories(run_eval PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(run_eval PRIVATE
    nlohmann_json::nlohmann_json
    pthread
)

# ── Unit tests (không cần OllamaClient → không cần curl/httplib) ──
enable_testing()

add_executable(run_tests
    tests/test_calculator.cpp
    tests/test_loop_detector.cpp
    tests/test_trajectory.cpp
    tests/test_keyword_evaluator.cpp
    tests/test_tool_registry.cpp
    ${CORE_SRC}
)
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(run_tests PRIVATE
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(run_tests)