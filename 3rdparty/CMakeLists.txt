add_subdirectory(argparse)

add_library(dr_libs INTERFACE)
target_include_directories(dr_libs INTERFACE dr_libs-scoped-include)


option(FMT_DOC OFF)
option(FMT_TEST OFF)
option(FMT_INSTALL OFF)
add_subdirectory(fmt)

add_library(icecream INTERFACE)
target_include_directories(icecream INTERFACE icecream-scoped-include)
