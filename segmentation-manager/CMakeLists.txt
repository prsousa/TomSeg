FILE( GLOB SRC_LIST *.cpp *.h )
FILE( GLOB DIF_LIST differentiators/* )
FILE( GLOB PRE_LIST preprocessors/* )
FILE( GLOB JSON_LIST json/* )
FILE( GLOB SEG_LIST segmentors/proportional-region-growing.* segmentors/segmenter.h )

ADD_LIBRARY( SegmentationManager STATIC
    ${SRC_LIST}
    ${DIF_LIST}
    ${PRE_LIST}
    ${JSON_LIST}
    ${SEG_LIST}
)
