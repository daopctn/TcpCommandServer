QT       += core network
QT       -= gui

CONFIG   += console c++11
CONFIG   -= app_bundle

TARGET = TcpStreamServer
TEMPLATE = app

INCLUDEPATH += src

SOURCES += src/main.cpp \
           src/streamserver.cpp

HEADERS += src/streamserver.h \
           src/protocol.h \
           src/config.h
