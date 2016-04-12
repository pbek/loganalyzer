#-------------------------------------------------
#
# Project created by QtCreator 2016-03-16T09:03:49
#
#-------------------------------------------------

QT       += core gui svg sql printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogAnalyzer
TEMPLATE = app
RC_FILE = LogAnalyzer.rc
CONFIG += c++11

SOURCES += main.cpp \
        version.h \
        release.h \
        mainwindow.cpp \
        qtexteditsearchwidget.cpp \
        libraries/simplecrypt/simplecrypt.cpp \
        services/cryptoservice.cpp \
        services/ezpublishservice.cpp \
        utils/misc.cpp \
        services/databaseservice.cpp \
        entities/logfilesource.cpp \
        dialogs/settingsdialog.cpp

HEADERS  += mainwindow.h \
        qtexteditsearchwidget.h \
        libraries/simplecrypt/simplecrypt.h \
        services/cryptoservice.h \
        services/ezpublishservice.h \
        utils/misc.h \
        services/databaseservice.h \
        entities/logfilesource.h \
        dialogs/settingsdialog.h

FORMS    += mainwindow.ui \
    dialogs/settingsdialog.ui

RESOURCES += \
    media.qrc
