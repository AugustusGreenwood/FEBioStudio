TEMPLATE = lib
TARGET = meshtools
DESTDIR = ../build/lib
CONFIG += debug c++14 staticlib warn_off
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
DEFINES += LINUX HAS_OCC TETLIBRARY HAS_NETGEN HAS_QUAZIP HAS_SSH
INCLUDEPATH += ../
INCLUDEPATH += /opt/netgen/include
INCLUDEPATH += /opt/netgen/include/include
INCLUDEPATH += /home/sci/mherron/Resources/opencascade-7.3.0/include/opencascade
INCLUDEPATH += /home/sci/mherron/Resources/tetgen1.5.0/

HEADERS += *.h
SOURCES += *.cpp
