#-------------------------------------------------
#
# PT 08/2015 _ Shared Library for Linux/Windows
#
#-------------------------------------------------
QT        -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT -= widgets

TARGET = lcr
TEMPLATE = lib

DEFINES += lcr

SOURCES += usb.cpp \
    API.cpp \
    BMPParser.cpp \
    firmware.cpp

HEADERS  += usb.h \
    API.h \
    BMPParser.h \
    firmware.h

INCLUDEPATH += "hidapi-master\\hidapi"

linux-g++{
#QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/lib
}

win32:CONFIG(release, debug|release): LIBS += -L../lcr-api/hidapi-master/windows/release/ -lhidapi
else:win32:CONFIG(debug, debug|release): LIBS += -L../lcr-api/hidapi-master/windows/Debug/ -lhidapi

macx: SOURCES += hidapi-master/mac/hid.c

unix: !macx: SOURCES += hidapi-master/linux/hid.c \
                        hidapi-master/hidapi/hidapi.h

macx: LIBS += -framework CoreFoundation -framework IOkit

#win32: LIBS += -lSetupAPI

unix: !macx: LIBS += -lusb-1.0 -ludev

INCLUDEPATH += ../hidapi-master/hidapi
DEPENDPATH += ../hidapi-master/hidapi
