cmake_minimum_required(VERSION 3.16)
project(AutomatedExamSeatingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect all source files from src/
set(SOURCES
    src/main.cpp
    src/SeatCodeUtils.cpp
    src/Venue.cpp
)

add_executable(SeatCodeDemo ${SOURCES})

# Headers live in include/
target_include_directories(SeatCodeDemo PRIVATE include)
