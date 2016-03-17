#-------------------------------------------------
#
# Project created by QtCreator 2016-03-16T09:03:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogAnalyzer
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
        qtexteditsearchwidget.cpp

HEADERS  += mainwindow.h \
        qtexteditsearchwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    media.qrc
