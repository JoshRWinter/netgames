HEADERS += Pong.h
HEADERS += Game.h
HEADERS += Dialog.h
HEADERS += network.h
HEADERS += PongServer.h
HEADERS += PongBot.h

SOURCES += main.cpp
SOURCES += Pong.cpp
SOURCES += PongServer.cpp
SOURCES += Game.cpp
SOURCES += Dialog.cpp
SOURCES += network.cpp
SOURCES += PongBot.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = pong
