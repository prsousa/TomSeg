#-------------------------------------------------
#
# Project created by QtCreator 2016-09-13T21:42:36
#
#-------------------------------------------------

QT       += core gui

# <Charts>
QT += charts
# </Charts>

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
    segmentation-manager/exporter.cpp \
    segmentation-manager/seedpropagater.cpp \
    histogramview.cpp


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
    segmentation-manager/exporter.h \
    segmentation-manager/seedpropagater.h \
    histogramview.h


FORMS    += mainwindow.ui

# <OPENMP>
LIBS += /usr/local/lib/libiomp5.dylib
QMAKE_CXXFLAGS += -fopenmp
# </OPENMP>

# <OpenCV>
INCLUDEPATH += /usr/local/include/

LIBS += /usr/local/lib/libopencv_calib3d.dylib /usr/local/lib/libopencv_contrib.dylib /usr/local/lib/libopencv_core.dylib /usr/local/lib/libopencv_features2d.dylib /usr/local/lib/libopencv_flann.dylib /usr/local/lib/libopencv_gpu.dylib /usr/local/lib/libopencv_highgui.dylib /usr/local/lib/libopencv_imgproc.dylib /usr/local/lib/libopencv_legacy.dylib /usr/local/lib/libopencv_ml.dylib /usr/local/lib/libopencv_nonfree.dylib /usr/local/lib/libopencv_objdetect.dylib /usr/local/lib/libopencv_ocl.dylib /usr/local/lib/libopencv_photo.dylib /usr/local/lib/libopencv_stitching.dylib /usr/local/lib/libopencv_superres.dylib /usr/local/lib/libopencv_ts.a /usr/local/lib/libopencv_video.dylib /usr/local/lib/libopencv_videostab.dylib
# </OpenCV>

# <CUDA>

CUDA_SOURCES += \
    segmentation-manager/segmentors/cuda-kernels.cu

CUDA_DIR = "/Developer/NVIDIA/CUDA-7.0"

SYSTEM_TYPE = 64            # '32' or '64', depending on your system
CUDA_ARCH = sm_21           # (tested with sm_30 on my comp) Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
NVCC_OPTIONS = --use_fast_math

INCLUDEPATH += $$CUDA_DIR/include

QMAKE_LIBDIR += $$CUDA_DIR/lib/

CUDA_OBJECTS_DIR = ./

CUDA_LIBS = -lcudart

CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')
LIBS += $$CUDA_LIBS # <-- needed this

QMAKE_LFLAGS += -Wl,-rpath,$$CUDA_DIR/lib # <-- added this
NVCCFLAGS = -Xlinker -rpath,$$CUDA_DIR/lib # <-- and this

CONFIG(debug, debug|release) {
    # Debug mode
    cuda_d.input = CUDA_SOURCES
    cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda_d.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda_d
}
else {
    cuda.input = CUDA_SOURCES
    cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $$CUDA_DIR/bin/nvcc $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda
}


# </CUDA>

RESOURCES += \
    icons.qrc

DISTFILES += \
    segmentation-manager/segmentors/cuda-kernels.cu
