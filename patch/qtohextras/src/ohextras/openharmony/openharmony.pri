QT += widgets
DEFINES += QT_BUILD_OPENHARMONYEXTRAS_LIB
LIBS += -lohfileshare -lchild_process

SOURCES += \
    $$PWD/qohappcontext.cpp \
    $$PWD/qohfileshare.cpp \
    $$PWD/qohfunctions.cpp \
    $$PWD/qopenharmonyability.cpp \
    $$PWD/qohwidgethelper.cpp \
    $$PWD/qohprocess.cpp \
    $$PWD/qohabstractlocalsocket.cpp \
    $$PWD/qohlocalsocket.cpp \
    $$PWD/qohlocalserver.cpp \
    $$PWD/qohlocalconnection.cpp \
    $$PWD/qohchildprocess.cpp
           
HEADERS += \
    $$PWD/qohappcontext.h \
    $$PWD/qohfileshare.h \
    $$PWD/qohfunctions.h \
    $$PWD/qopenharmonyextrasglobal.h \
    $$PWD/qopenharmonyability.h \
    $$PWD/qohwidgethelper.h \
    $$PWD/qohprocess.h \
    $$PWD/qohabstractlocalsocket.h \
    $$PWD/qohabstractlocalsocket_p.h \
    $$PWD/qohlocalsocket.h \
    $$PWD/qohlocalserver.h \
    $$PWD/qohlocalconnection.h \
    $$PWD/qohchildprocess.h

