#-------------------------------------------------
#
# Project created by QtCreator 2014-01-14T13:53:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app

win32:LIBS += -lws2_32

SOURCES += main.cpp\
        mainwindow.cpp \
    client.cpp \
    helpFunctions.cpp \
    connect_ui.cpp \
    download.cpp \
    find.cpp \
    replace.cpp \
    highlight_c.cpp

HEADERS  += mainwindow.h \
    client.h \
    helpFunctions.h \
    connect_ui.h \
    download.h \
    find.h \
    replace.h \
    highlight_c.h

FORMS    += mainwindow.ui \
    connect_ui.ui \
    download.ui \
    find.ui \
    replace.ui

RESOURCES += \
    Resource.qrc
