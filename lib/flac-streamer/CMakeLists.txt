set(FLAC_STREAMER_SRC
    dr_libs.cpp
    common-internal.h
    flac-streamer.cpp
)

set(FLAC_STREAMER_HDR_STANDALONE
    common.h
    common-attrs.h
)

set(FLAC_STREAMER_HDR)
foreach(HDR ${FLAC_STREAMER_HDR_STANDALONE})
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/flac-streamer/${HDR}")
    list(APPEND FLAC_STREAMER_HDR ${HDR})
endforeach()

foreach(SRC ${FLAC_STREAMER_SRC})
    get_filename_component(HDR_NAME ${SRC} NAME_WLE)
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/flac-streamer/${HDR_NAME}.h")
    if(EXISTS ${HDR})
        list(APPEND FLAC_STREAMER_HDR ${HDR})
    endif()
endforeach()

set(FLAC_STREAMER_PUBLIC_LIBS
    dr_libs
    FLAC++
    icecream
)

set(FLAC_STREAMER_PRIVATE_LIBS
    fmt
)

# FLAC_STREAMER_HDR added for Xcode project generation
add_library(flac-streamer STATIC ${FLAC_STREAMER_SRC} ${FLAC_STREAMER_HDR})
add_library(flac-streamer-shared SHARED ${FLAC_STREAMER_SRC} ${FLAC_STREAMER_HDR})
set_target_properties(flac-streamer flac-streamer-shared PROPERTIES PUBLIC_HEADER "${FLAC_STREAMER_HDR}")

target_link_libraries(flac-streamer
PUBLIC
    ${FLAC_STREAMER_PUBLIC_LIBS}
PRIVATE
    ${FLAC_STREAMER_PRIVATE_LIBS}
)
target_compile_options(flac-streamer PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(flac-streamer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(flac-streamer-shared
PUBLIC
    ${FLAC_STREAMER_PUBLIC_LIBS}
PRIVATE
    ${FLAC_STREAMER_PRIVATE_LIBS}
)
target_compile_options(flac-streamer-shared PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(flac-streamer-shared
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)


install(TARGETS flac-streamer flac-streamer-shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/flac-streamer
)

# target_compile_options(flac-streamer PRIVATE -Xclang -fdump-record-layouts)
