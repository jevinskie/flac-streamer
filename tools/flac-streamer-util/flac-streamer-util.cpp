#include "flac-streamer/flac-streamer.h"

#undef NDEBUG
#include <cassert>
#include <cstdlib>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

using namespace FLACStreaming;

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(getprogname());
    parser.add_argument("-i", "--in-file").required().help("input wave file path");
    parser.add_argument("-o", "--out-file").help("output FLAC file path (use - for stdout)");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -1;
    }

    auto streamer = FLACStreamer(parser.get("--in-file"), parser.get("--out-file"));
    streamer.encode();

    return 0;
}
