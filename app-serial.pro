#-------------------------------------------------
#
# Project created by QtCreator 2018-04-10T13:12:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = app-serial
TEMPLATE = app

RC_FILE += qrc/appsource.rc

HEADERS += \
    app/appserial.h

SOURCES += \
    app/appserial.cpp \
    app/main.cpp

RESOURCES += \
    qrc/appsource.qrc

