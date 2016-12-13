file( GLOB SRC_LIST *.cpp *.h )
file( GLOB DIF_LIST differentiators/* )
file( GLOB PRE_LIST preprocessors/* )
file( GLOB SEG_LIST segmentors/proportional-region-growing.* segmentors/segmenter.h )

ADD_LIBRARY( SegmentationManager STATIC
    ${SRC_LIST}
    ${DIF_LIST}
    ${PRE_LIST}
    ${SEG_LIST}
)
