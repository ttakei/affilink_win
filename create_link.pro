#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T05:29:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = create_link
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    searchentry.cpp \
    log.cpp \
    wwl.cpp \
    ini.cpp \
    layout.cpp \
    searchentrymanager.cpp \
    csv.cpp

HEADERS  += mainwindow.h \
    searchentry.h \
    log.h \
    wwl.h \
    ini.h \
    layout.h \
    searchentrymanager.h \
    csv.h

FORMS    += mainwindow.ui
