cmake_minimum_required(VERSION 3.16)
project(KUExamSeating VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 QUIET COMPONENTS Core Gui Widgets PrintSupport)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
endif()

set(HEADERS
    include/mainwindow.h
    include/studentmodel.h
    include/hallmap.h
    include/subblockdialog.h
    include/dashboardpage.h
    include/studentspage.h
    include/seatingpage.h
    include/analyticspage.h
    include/exportmanager.h
)

set(SOURCES
    main/main.cpp
    main/mainwindow.cpp
    main/studentmodel.cpp
    main/hallmap.cpp
    main/subblockdialog.cpp
    main/dashboardpage.cpp
    main/studentspage.cpp
    main/seatingpage.cpp
    main/analyticspage.cpp
    main/exportmanager.cpp
)

if(Qt6_FOUND)
    qt_add_executable(KUExamSeating ${SOURCES} ${HEADERS} resources.qrc)
    target_link_libraries(KUExamSeating PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)
else()
    add_executable(KUExamSeating ${SOURCES} ${HEADERS} resources.qrc)
    target_link_libraries(KUExamSeating PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)
endif()

target_include_directories(KUExamSeating PRIVATE include)
