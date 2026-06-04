cmake_minimum_required(VERSION 3.16)
project(examseatingui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(examseatingui
    main.cpp
    mainwindow.h
    mainwindow.cpp
    pages/DashboardPage.h
    pages/DashboardPage.cpp
    pages/StudentPage.h
    pages/StudentPage.cpp
    pages/RoomPage.h
    pages/RoomPage.cpp
    pages/ResultPage.h
    pages/ResultPage.cpp
)

target_include_directories(examseatingui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pages
)

target_link_libraries(examseatingui PRIVATE Qt6::Widgets)