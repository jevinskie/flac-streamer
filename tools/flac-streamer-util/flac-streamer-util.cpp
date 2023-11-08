#include "flac-streamer/flac-streamer.h"

#undef NDEBUG
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <locale>
#include <thread>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace fs = std::filesystem;

using namespace FLACStreaming;

int main(int argc, const char *argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));
    argparse::ArgumentParser parser(getprogname());
    parser.add_argument("-i", "--in-file").required().help("input wave file path");
    parser.add_argument("-o", "--out-file")
        .required()
        .help("output FLAC file path (use - for stdout)");
    parser.add_argument("-c", "--compression-level")
        .scan<'i', int>()
        .default_value(5)
        .help("FLAC compression level");
    parser.add_argument("-n", "--no-verify")
        .default_value(false)
        .implicit_value(true)
        .help("don't verify something");
    parser.add_argument("-S", "--non-streamable")
        .default_value(false)
        .implicit_value(true)
        .help("don't make output streamable");
    parser.add_argument("-M", "--non-mid-side")
        .default_value(false)
        .implicit_value(true)
        .help("don't use mid-side encoding");
    parser.add_argument("-l", "--loose-mid-side")
        .default_value(false)
        .implicit_value(true)
        .help("use loose mid-side encoding instead of exhaustive search");
    parser.add_argument("-e", "--exhaustive-search")
        .default_value(false)
        .implicit_value(true)
        .help("do exhaustive search of all models");
    parser.add_argument("-q", "--qlp-coeff-prec-search")
        .default_value(false)
        .implicit_value(true)
        .help("do search neighboring quantized linear predictor coefficient search");
    parser.add_argument("-b", "--block-size").scan<'i', int>().default_value(0).help("block size");
    parser.add_argument("-m", "--chunk-every-n-ms")
        .scan<'i', int>()
        .default_value(10)
        .help("stream encode by chunking every N milliseconds");
    parser.add_argument("-t", "--threads")
        .scan<'i', int>()
        .default_value((int)std::thread::hardware_concurrency())
        .help("number of threads to encode with");
    parser.add_argument("-C", "--defer-to-compression-level")
        .default_value(false)
        .implicit_value(true)
        .help("don't apply settings that override compression level defaults");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -1;
    }

    const auto comp_level = parser.get<int>("--compression-level");
    if (comp_level < 0 || comp_level > 8) {
        fmt::print(stderr, "Compression level must be between 0 and 8, not {}\n", comp_level);
        return -1;
    }

    const auto block_size                 = parser.get<int>("--block-size");
    const auto chunk_every_n_ms           = parser.get<int>("--chunk-every-n-ms");
    const auto verify                     = !parser.get<bool>("--no-verify");
    const auto streamable                 = !parser.get<bool>("--non-streamable");
    const auto mid_side                   = !parser.get<bool>("--non-mid-side");
    const auto loose_mid_side             = parser.get<bool>("--loose-mid-side");
    const auto exhaustive_model_search    = parser.get<bool>("--exhaustive-search");
    const auto qlp_coeff_prec_search      = parser.get<bool>("--qlp-coeff-prec-search");
    const auto defer_to_compression_level = parser.get<bool>("--defer-to-compression-level");
    const auto num_threads                = parser.get<int>("--threads");

    fmt::print("comp level: {} block size: {} chunk every N milliseconds: {} verify: {} "
               "streamable: {} mid-side: {} loose mid-side: {} exhaustive model search: {} qlp "
               "coefficient precision neighbor search: {} defer to compression level: {} num "
               "threads: {}\n",
               comp_level, block_size, chunk_every_n_ms, verify, streamable, mid_side,
               loose_mid_side, exhaustive_model_search, qlp_coeff_prec_search,
               defer_to_compression_level, num_threads);

    assert(chunk_every_n_ms > 0);
    assert(num_threads > 0);

    const auto in_path  = fs::path{parser.get("--in-file")};
    const auto out_path = fs::path{parser.get("--out-file")};

    auto streamer = FLACStreamer(in_path, out_path, chunk_every_n_ms);
    assert(streamer.set_compression_level(comp_level));
    if (!defer_to_compression_level) {
        assert(streamer.set_blocksize(block_size));
        assert(streamer.set_do_mid_side_stereo(mid_side));
        assert(streamer.set_loose_mid_side_stereo(loose_mid_side));
    } else {
        fmt::print("ignoring block size: {} mid-side: {} loose mid-side: {}\n", block_size,
                   mid_side, loose_mid_side);
    }
    assert(streamer.set_do_exhaustive_model_search(exhaustive_model_search));
    assert(streamer.set_do_qlp_coeff_prec_search(qlp_coeff_prec_search));
    assert(streamer.set_streamable_subset(streamable));
    assert(streamer.set_verify(verify));
    assert(!streamer.set_num_threads(num_threads));
    streamer.print_settings();
    const auto start = std::chrono::high_resolution_clock::now();
    streamer.encode();
    const auto end                  = std::chrono::high_resolution_clock::now();
    const auto encode_time_ns       = std::chrono::round<std::chrono::nanoseconds>(end - start);
    const auto encode_time_seconds  = encode_time_ns.count() / 1000000000.0;
    const auto audio_length_seconds = streamer.get_length();
    fmt::print("Encoded {:.3f} seconds of audio in {:.6f} seconds. Speedup: {:.3f}\n",
               audio_length_seconds, encode_time_seconds,
               audio_length_seconds / encode_time_seconds);
    const auto in_size  = fs::file_size(in_path);
    const auto out_size = fs::file_size(out_path);
    fmt::print(
        "Input file size: {:L} bytes Output file size: {:L} bytes. Compression ratio: {:.4f}\n",
        in_size, out_size, (double)out_size / in_size);

    return 0;
}
