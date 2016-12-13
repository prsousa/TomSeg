project( TomSeg )
cmake_minimum_required( VERSION 2.8 )
find_package( CUDA REQUIRED )
find_package( OpenCV REQUIRED )
find_package( Boost COMPONENTS program_options )

set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )
# set( CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_21 )

cuda_add_library( CudaObjs
    segmentation-manager/segmentors/cuda-kernels.cu )

file( GLOB SRC_LIST main.cpp cliapplication.cpp cliapplication.h )

add_subdirectory( segmentation-manager )

add_executable( ${PROJECT_NAME} ${SRC_LIST} )

add_definitions(-D CLI)

TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${Boost_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${OpenCV_LIBS} )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} CudaObjs )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} SegmentationManager )
