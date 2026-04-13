QT       += core network
QT       -= gui

CONFIG   += console c++11
CONFIG   -= app_bundle

TARGET = TcpCommandServer
TEMPLATE = app

INCLUDEPATH += src

SOURCES += src/main.cpp \
           src/commandserver.cpp

HEADERS += src/commandserver.h \
           src/protocol.h \
           src/config.h
