QT += widgets
DEFINES += QT_BUILD_OPENHARMONYEXTRAS_LIB
LIBS += -lohfileshare

SOURCES += \
    $$PWD/qohappcontext.cpp \
    $$PWD/qohfileshare.cpp \
    $$PWD/qohfunctions.cpp \
    $$PWD/qopenharmonyability.cpp \
    $$PWD/qohwidgethelper.cpp
           
HEADERS += \
    $$PWD/qohappcontext.h \
    $$PWD/qohfileshare.h \
    $$PWD/qohfunctions.h \
    $$PWD/qopenharmonyextrasglobal.h \
    $$PWD/qopenharmonyability.h \
    $$PWD/qohwidgethelper.h
