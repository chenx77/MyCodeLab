#include(../examples.pri)
TEMPLATE = app
HEADERS = winimage.h \
    transformer.h \
    gauss.h \
    interpolator.h \
    Inflate.h \
    Spherize.h \
    Differ.h \
    Interpolation.h \
    Zoom.h \
    rgb_hls.h \
    hsl.h \
    blending_color.h \
    detector.h \
    circle_detector.h \
    line_detector.h \
    edge_detector.h \
    merge_find.h \
    rotater.h
SOURCES = winimage.cpp \
    transformer.cpp \
    gauss.cpp \
    Interpolation.cpp \
    detector.cpp \
    line_detector.cpp \
    circle_detector.cpp \
    main.cpp \
    edge_detector.cpp \
    merge_find.cpp \
    rotater.cpp
QT += widgets
!isEmpty(QT.printsupport.name):QT += printsupport

OTHER_FILES += \
    note.txt
