add_executable(flac-streamer-util flac-streamer-util.cpp)

target_link_libraries(flac-streamer-util flac-streamer argparse fmt)
target_compile_options(flac-streamer-util PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS flac-streamer-util
    RUNTIME DESTINATION bin
)
