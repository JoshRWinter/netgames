HEADERS += Pong.h

SOURCES += main.cpp
SOURCES += Pong.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = pong
