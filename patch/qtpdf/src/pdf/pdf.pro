TARGET = QtPdf
QT += gui core core-private
QT_PRIVATE += network
TEMPLATE = lib
CONFIG += c++11
CONFIG -= precompile_header # Not supported by upstream header files
win32: DEFINES += NOMINMAX

include($$PWD/../3rdparty/pdfium/pdfium.pri)

load(qt_module)


QMAKE_DOCS = $$PWD/doc/qtpdf.qdocconf

gcc {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON += -wd"4100"
}

SOURCES += \
    qpdfdestination.cpp \
    qpdflinkmodel.cpp \
    jsbridge.cpp \
    qpdfbookmarkmodel.cpp \
    qpdfdocument.cpp \
    qpdfpagenavigation.cpp \
    qpdfpagerenderer.cpp \
    qpdfsearchmodel.cpp \
    qpdfsearchresult.cpp \
    qpdfselection.cpp

HEADERS += \
    qpdfbookmarkmodel.h \
    qpdfdestination.h \
    qpdfdestination_p.h \
    qpdfdocument.h \
    qpdfdocument_p.h \
    qpdfdocumentrenderoptions.h \
    qpdflinkmodel_p.h \
    qpdflinkmodel_p_p.h \
    qpdfnamespace.h \
    qpdfpagenavigation.h \
    qpdfpagerenderer.h \
    qpdfsearchmodel.h \
    qpdfsearchmodel_p.h \
    qpdfsearchresult.h \
    qpdfsearchresult_p.h \
    qpdfselection.h \
    qpdfselection_p.h \
    qtpdfglobal.h
