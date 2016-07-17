file( GLOB SRC_LIST *.cpp *.h )

ADD_LIBRARY(segmenters STATIC
    ${SRC_LIST}
)
