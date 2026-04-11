QT       += core network
QT       -= gui

CONFIG   += console c++11
CONFIG   -= app_bundle

TARGET = TcpCommandServer
TEMPLATE = app

SOURCES += main.cpp \
           commandserver.cpp

HEADERS += commandserver.h \
           protocol.h
