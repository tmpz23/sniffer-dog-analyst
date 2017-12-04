#-------------------------------------------------
#
# Project created by QtCreator 2017-11-19T02:02:07
#
#-------------------------------------------------

QT       += core gui \
            sql \
            network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sniffer_Dog_Analyst
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        class/googlecachepage.cpp \
        class/proxy.cpp \
        class/pivot.cpp \
        class/result.cpp \
        class/scrapper.cpp \
        class/searchorchestrator.cpp \
        helpers/qreplytimeout.cpp \
        models/dbmanager.cpp \
        models/proxyqlistmodel.cpp

HEADERS += \
        mainwindow.h \
        class/googlecachepage.h \
        class/proxy.h \
        class/pivot.h \
        class/result.h \
        class/scrapper.h \
        class/searchorchestrator.h \
        helpers/logcolor.h \
        helpers/qreplytimeout.h \
        models/dbmanager.h \
        models/proxyqlistmodel.h

FORMS += \
        mainwindow.ui

RC_FILE += \
        ressources.rc
