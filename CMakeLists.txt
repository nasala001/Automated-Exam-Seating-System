cmake_minimum_required(VERSION 3.16)
project(AutomatedExamSeatingSystem VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# ---------------------------------------------------------
# COMBINED TARGET: AutomatedExamSeating
# ---------------------------------------------------------
set(SOURCES
    # UI Files
    ui/main.cpp
    ui/mainwindow.h
    ui/mainwindow.cpp
    ui/mainwindow.ui
    ui/pages/dashboardpage.h
    ui/pages/dashboardpage.cpp
    ui/pages/studentpage.h
    ui/pages/studentpage.cpp
    ui/pages/roompage.h
    ui/pages/roompage.cpp
    ui/pages/resultpage.h
    ui/pages/resultpage.cpp

    # Backend / Engine Files
    src/DatabaseManager.cpp
    src/SeatCodeUtils.cpp
    src/Student.cpp
    src/SeatingEngine.cpp
    src/Venue.cpp
)

qt_add_executable(AutomatedExamSeating ${SOURCES})

target_include_directories(AutomatedExamSeating PRIVATE 
    include
    ui
    ui/pages
)

target_link_libraries(AutomatedExamSeating PRIVATE Qt6::Widgets Qt6::Sql)
