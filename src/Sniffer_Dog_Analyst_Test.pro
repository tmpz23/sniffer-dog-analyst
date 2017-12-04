QT += testlib \
    core \
    gui \
    sql \
    network

CONFIG += qt warn_on depend_includepath testcase

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += QT_DEPRECATED_WARNINGS

TARGET = Sniffer_Dog_Analyst_Tests
TEMPLATE = app

SOURCES += \
        #tst_mainwindow.cpp \
        test/resulttest.cpp \
        class/result.cpp

HEADERS += \
        test/resulttest.h \
        class/result.h
