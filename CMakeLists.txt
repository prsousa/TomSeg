project(TomSeg)
cmake_minimum_required(VERSION 2.8)
find_package( OpenCV REQUIRED )
file( GLOB SRC_LIST *.cpp *.h )

add_subdirectory(segmentors)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT_NAME} segmenters )
