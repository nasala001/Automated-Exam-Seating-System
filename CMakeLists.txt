cmake_minimum_required(VERSION 3.16)
project(ExamSeating VERSION 1.0 LANGUAGES CXX)

# ── C++20 standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Include directories ───────────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/include)

# ── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Room.cpp
    src/Hall.cpp
    src/HallManager.cpp
    # TODO: add these as you implement them:
    # src/sqlite3.c          <- SQLite amalgamation
    # src/models/Student.cpp
    # src/models/Exam.cpp
    # src/engine/SeatPlanner.cpp
    # src/io/CSVImporter.cpp
    # src/rules/RuleEngine.cpp
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(ExamSeating ${SOURCES})

# ── SQLite (add when you drop in sqlite3.c / sqlite3.h) ──────────────────────
# target_sources(ExamSeating PRIVATE src/sqlite3.c)
# target_link_libraries(ExamSeating PRIVATE dl pthread)

# ── GoogleTest (enable when you add tests/) ───────────────────────────────────
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
# )
# FetchContent_MakeAvailable(googletest)
# enable_testing()
# add_subdirectory(tests)

# ── Qt6 (enable when Pratisthit starts UI integration) ───────────────────────
# find_package(Qt6 REQUIRED COMPONENTS Widgets)
# target_link_libraries(ExamSeating PRIVATE Qt6::Widgets)

# ── Compiler warnings ─────────────────────────────────────────────────────────
if (MSVC)
    target_compile_options(ExamSeating PRIVATE /W4)
else()
    target_compile_options(ExamSeating PRIVATE -Wall -Wextra -Wpedantic)
endif()