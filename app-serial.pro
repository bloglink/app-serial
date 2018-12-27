#-------------------------------------------------
#
# Project created by QtCreator 2018-04-10T13:12:24
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = app-serial
TEMPLATE = app

RC_FILE += qrc/appsource.rc

HEADERS += \
    app/appserial.h \
    app/boxqitems.h \
    app/boxqmodel.h

SOURCES += \
    app/appserial.cpp \
    app/main.cpp \
    app/boxqitems.cpp \
    app/boxqmodel.cpp

RESOURCES +=

