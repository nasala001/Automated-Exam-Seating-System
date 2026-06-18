QT       += core gui widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = KUExamSeating
TEMPLATE = app

INCLUDEPATH += include

HEADERS += \
    include/mainwindow.h \
    include/studentmodel.h \
    include/hallmap.h \
    include/subblockdialog.h \
    include/dashboardpage.h \
    include/studentspage.h \
    include/seatingpage.h \
    include/analyticspage.h \
    include/exportmanager.h

SOURCES += \
    main/main.cpp \
    main/mainwindow.cpp \
    main/studentmodel.cpp \
    main/hallmap.cpp \
    main/subblockdialog.cpp \
    main/dashboardpage.cpp \
    main/studentspage.cpp \
    main/seatingpage.cpp \
    main/analyticspage.cpp \
    main/exportmanager.cpp

RESOURCES += resources.qrc
