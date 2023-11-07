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
    parser.add_argument("-n", "--no-verify")
        .default_value(false)
        .implicit_value(true)
        .help("don't verify something");
    parser.add_argument("-S", "--non-streamable")
        .default_value(false)
        .implicit_value(true)
        .help("don't make output streamable");
    parser.add_argument("-c", "--compression-level")
        .scan<'i', uint32_t>()
        .default_value(5)
        .help("FLAC compression level");
    parser.add_argument("-b", "--block-size")
        .scan<'i', uint32_t>()
        .default_value(0)
        .help("FLAC compression level");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -1;
    }

    const auto comp_level = parser.get<uint32_t>("--compression-level");
    if (comp_level < 0 || comp_level > 8) {
        fmt::print(stderr, "Compression level must be between 0 and 8, not {}\n", comp_level);
        return -1;
    }

    const auto block_size = parser.get<uint32_t>("--block-size");
    const auto verify     = !parser.get<bool>("--no-verify");
    const auto streamable = !parser.get<bool>("--non-streamable");

    fmt::print("comp_level: {} block_size: {} verify: {} streamable: {}\n", comp_level, block_size,
               verify, streamable);

    auto streamer = FLACStreamer(parser.get("--in-file"), parser.get("--out-file"));
    streamer.set_compression_level(comp_level);
    streamer.set_blocksize(block_size);
    streamer.set_streamable_subset(streamable);
    streamer.set_verify(verify);
    streamer.encode();

    return 0;
}
