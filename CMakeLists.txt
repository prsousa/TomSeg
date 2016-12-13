PROJECT( TomSeg )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
FIND_PACKAGE( CUDA REQUIRED )
FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE( Boost COMPONENTS program_options )

INCLUDE_DIRECTORIES (
  ${Boost_INCLUDE_DIRS}
)

SET( CMAKE_CXX_FLAGS "-std=c++11 -O3" )

# SET( CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_21 )
SET( CUDA_PROPAGATE_HOST_FLAGS OFF )
SET( CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -std=c++11 )

CUDA_ADD_LIBRARY( CudaObjs
    segmentation-manager/segmentors/cuda-kernels.cu
)

FILE( GLOB SRC_LIST main.cpp cliapplication.cpp cliapplication.h )

ADD_SUBDIRECTORY( segmentation-manager )

ADD_EXECUTABLE( ${PROJECT_NAME} ${SRC_LIST} )

ADD_DEFINITIONS( -D CLI )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    SegmentationManager
    CudaObjs
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
)
