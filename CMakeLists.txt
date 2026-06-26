cmake_minimum_required(VERSION 3.16)
project(AutomatedExamSeatingSystem LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt 6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/khaniya_new_ui/include
    ${CMAKE_SOURCE_DIR}/shreeshant_database/include
    ${CMAKE_SOURCE_DIR}/shresh_validation
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/nasala_hall_room/include
)

# Collect source files
file(GLOB_RECURSE UI_SOURCES "khaniya_new_ui/main/*.cpp" "khaniya_new_ui/include/*.h" "khaniya_new_ui/*.qrc")
file(GLOB_RECURSE DB_SOURCES "shreeshant_database/src/*.cpp" "shreeshant_database/include/*.hpp")
file(GLOB_RECURSE VAL_SOURCES "shresh_validation/*.cpp" "shresh_validation/*.h")
file(GLOB_RECURSE NASALA_SOURCES "nasala_hall_room/src/*.cpp" "nasala_hall_room/src/*.c" "nasala_hall_room/include/*.h")

# The engine
file(GLOB_RECURSE ENGINE_SOURCES "src/engine/*.cpp" "include/engine/*.h" "src/io/*.cpp" "include/io/*.h" "src/SeatCodeUtils.cpp" "include/SeatCodeUtils.h")

qt_add_executable(ExamSystem 
    ${UI_SOURCES}
    ${DB_SOURCES}
    ${VAL_SOURCES}
    ${NASALA_SOURCES}
    ${ENGINE_SOURCES}
)

target_link_libraries(ExamSystem PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
