QT += core
QT -= gui

TARGET = neho_scm
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# on récupère les dossiers des sources, des headers et des protobuf
INCLUDEPATH += $$PWD/src/hpp
INCLUDEPATH += $$PWD/src/cpp
INCLUDEPATH += $$PWD/proto

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

# on ajoute les librairies nécessaires au projet
# protobuff
unix:!macx: LIBS += /usr/local/lib/libprotobuf.a
unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libprotobuf.a

# snmp++
unix:!macx: LIBS += -L/usr/local/lib/ -lsnmp++
unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libsnmp++.a

# hasimov
unix:!macx: LIBS += -L/usr/lib/ -lhasimov

HEADERS += \
    src/cpp/iniparser.h \
    proto/Protobuf.pb.h \
    src/hpp/SNMPRequest.hpp \
    src/hpp/utime.hpp \
    src/hpp/globals.hpp \
    src/hpp/SnmpPlugin.hpp \
    src/hpp/SnmpMultiRequest.hpp \
    src/hpp/SnmpQueryManager.hpp

SOURCES += src/cpp/main.cpp \
    src/cpp/utime.cpp \
    src/cpp/SNMPRequest.cpp \
    src/cpp/SnmpPlugin.cpp \
    src/cpp/SnmpMultiRequest.cpp \
    src/cpp/SnmpQueryManager.cpp

target.path = /home/pi/
INSTALLS += target
