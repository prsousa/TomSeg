#-------------------------------------------------
#
# Project created by QtCreator 2016-09-13T21:42:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TomSeg
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myqgraphicsscene.cpp \
    segmentation-manager/point.cpp \
    segmentation-manager/region.cpp \
    segmentation-manager/seed.cpp \
    segmentation-manager/segmentors/proportional-region-growing.cpp \
    segmentation-manager/segmentationmanager.cpp \
    segmentation-manager/slice.cpp \
    segmenterthread.cpp \
    cliapplication.cpp \
    segmentation-manager/differentiators/differentiator.cpp \
    segmentation-manager/preprocessors/aligner.cpp \
    segmentation-manager/exporter.cpp


HEADERS  += mainwindow.h \
    myqgraphicsscene.h \
    segmentation-manager/point.h \
    segmentation-manager/region.h \
    segmentation-manager/seed.h \
    segmentation-manager/segmentors/segmenter.h \
    segmentation-manager/segmentors/proportional-region-growing.h \
    segmentation-manager/segmentationmanager.h \
    segmentation-manager/slice.h \
    segmenterthread.h \
    cliapplication.h \
    segmentation-manager/differentiators/differentiator.h \
    segmentation-manager/preprocessors/aligner.h \
    segmentation-manager/exporter.h


FORMS    += mainwindow.ui

# <OpenCV>
INCLUDEPATH += /usr/local/include/

LIBS += /usr/local/lib/libopencv_calib3d.dylib /usr/local/lib/libopencv_contrib.dylib /usr/local/lib/libopencv_core.dylib /usr/local/lib/libopencv_features2d.dylib /usr/local/lib/libopencv_flann.dylib /usr/local/lib/libopencv_gpu.dylib /usr/local/lib/libopencv_highgui.dylib /usr/local/lib/libopencv_imgproc.dylib /usr/local/lib/libopencv_legacy.dylib /usr/local/lib/libopencv_ml.dylib /usr/local/lib/libopencv_nonfree.dylib /usr/local/lib/libopencv_objdetect.dylib /usr/local/lib/libopencv_ocl.dylib /usr/local/lib/libopencv_photo.dylib /usr/local/lib/libopencv_stitching.dylib /usr/local/lib/libopencv_superres.dylib /usr/local/lib/libopencv_ts.a /usr/local/lib/libopencv_video.dylib /usr/local/lib/libopencv_videostab.dylib
# </OpenCV>

RESOURCES += \
    icons.qrc
