#include "flac-streamer/flac-streamer.h"

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <filesystem>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
    fmt::print("flac-streamer-util\n");
    auto streamer = FLACStreamer();
    return 0;
}
