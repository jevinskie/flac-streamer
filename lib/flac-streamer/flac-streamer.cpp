#include "flac-streamer/flac-streamer.h"
#include "common-internal.h"
#include "flac-streamer/utils.h"

#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>

namespace FLACStreaming {

using namespace dr_libs::dr_wav;

FLACStreamer::FLACStreamer(const fs::path &wav_path, const fs::path &flac_path,
                           const uint64_t chunk_every_n_ms)
    : m_flac_path{flac_path}, m_chunk_every_n_ms{chunk_every_n_ms} {
    if (!drwav_init_file(&m_wav, wav_path.c_str(), nullptr)) {
        throw std::runtime_error(fmt::format("Failed to open input WAV file: {}", wav_path));
    }
    assert(set_channels(m_wav.channels));
    assert(set_bits_per_sample(m_wav.bitsPerSample));
    assert(set_sample_rate(m_wav.sampleRate));
    assert(set_total_samples_estimate(m_wav.totalPCMFrameCount));
    if (flac_path.string() != "-") {
        m_out_fh = fopen(flac_path.c_str(), "wb");
        if (!m_out_fh) {
            throw std::runtime_error(fmt::format("Failed to open output FLAC file: {}", flac_path));
        }
        m_out_fd = fileno(m_out_fh);
        posix_check(m_out_fd, fmt::format("trying to fileno() FILE* {} for {}", fmt::ptr(m_out_fh),
                                          m_flac_path));
    } else {
        m_out_fd = fileno(stdout);
        posix_check(m_out_fd, fmt::format("trying to fileno() FILE* {} for {}", fmt::ptr(stdout),
                                          m_flac_path));
    }
}

FLACStreamer::~FLACStreamer() {
    drwav_uninit(&m_wav);
    if (m_out_fh) {
        const auto close_res = fclose(m_out_fh);
        posix_check(close_res, fmt::format("trying to fclose() {}", m_flac_path));
        m_out_fh = nullptr;
    }
    m_out_fd = -1;
}

uint64_t FLACStreamer::get_num_samples_per_chunk() {
    return m_wav.sampleRate * ((double)m_chunk_every_n_ms / 1000);
}

double FLACStreamer::get_length() {
    return (double)m_wav.totalPCMFrameCount / m_wav.sampleRate;
}

void FLACStreamer::print_settings() {
    fmt::print("verify: {}\n", get_verify());
    fmt::print("block size: {}\n", get_blocksize());
    fmt::print("streamable: {}\n", get_streamable_subset());
    fmt::print("mid-side: {}\n", get_do_mid_side_stereo());
    fmt::print("loose mid-side: {}\n", get_loose_mid_side_stereo());
    fmt::print("exhaustive model search: {}\n", get_do_exhaustive_model_search());
    fmt::print("quantized linear predictor coefficient precision search: {}\n",
               get_do_qlp_coeff_prec_search());
    fmt::print("max lpc order: {}\n", get_max_lpc_order());
    fmt::print("max residual partition order: {}\n", get_max_residual_partition_order());
    fmt::print("channels: {}\n", get_channels());
    fmt::print("bits per sample: {}\n", get_bits_per_sample());
    fmt::print("sample rate: {}\n", get_sample_rate());
    fmt::print("chunk every N milliseconds: {}\n", m_chunk_every_n_ms);
    fmt::print("num samples per chunk: {}\n", get_num_samples_per_chunk());
    fmt::print("total samples estimate: {}\n", get_total_samples_estimate());
    fmt::print("audio length in seconds: {:.3f}\n", get_length());
    fmt::print("number of threads to encode with: {}\n", get_num_threads());
}

void FLACStreamer::encode() {
    if (const auto init_res = init(); init_res != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        throw std::runtime_error(fmt::format("FLAC::Encoder::Stream::init() failed with: {}",
                                             std::to_underlying(init_res)));
    }
    const auto num_samples  = m_wav.totalPCMFrameCount;
    const auto num_channels = m_wav.channels;
    const auto num_bits     = m_wav.bitsPerSample;
    std::vector<int32_t> buf(num_channels * num_samples);
    const auto num_samples_read = drwav_read_pcm_frames_s32(&m_wav, num_samples, buf.data());
    assert(num_samples_read == num_samples);
    const int32_t sample_max = (1 << (num_bits - 1)) - 1;
    const int32_t sample_min = -(1 << (num_bits - 1));
    for (size_t i = 0; i < buf.size(); ++i) {
        buf[i] = buf[i] * ((double)sample_max / INT32_MAX);
        if (buf[i] > sample_max) {
            fmt::print("clamping max\n");
            buf[i] = sample_max;
        } else if (buf[i] < sample_min) {
            fmt::print("clamping min\n");
            buf[i] = sample_min;
        }
    }

    const auto num_samples_per_chunk                                 = get_num_samples_per_chunk();
    auto samples_remaining                                           = num_samples;
    std::remove_const_t<decltype(num_samples)> num_samples_processed = 0;
    while (samples_remaining > 0) {
        const auto chunk_num_samples = std::min(num_samples_per_chunk, samples_remaining);
        const auto process_ok        = process_interleaved(
            buf.data() + (num_samples_processed * num_channels), chunk_num_samples);
        if (!process_ok) {
            throw std::runtime_error(
                fmt::format("FLAC::Encoder::Stream::process_interleaved() failed with: {}",
                            get_state().as_cstring()));
        }
        samples_remaining -= chunk_num_samples;
        num_samples_processed += chunk_num_samples;
    }
    assert(samples_remaining == 0);
    assert(num_samples_processed == num_samples);
    assert(finish());
}

::FLAC__StreamEncoderWriteStatus FLACStreamer::write_callback(const FLAC__byte buffer[],
                                                              size_t bytes, uint32_t samples,
                                                              uint32_t current_frame) {
    (void)samples;
    (void)current_frame;
    size_t completed = 0;
    while (completed < bytes) {
        const auto bytes_to_write = bytes - completed;
        const auto written        = ::write(m_out_fd, buffer + completed, bytes_to_write);
        if (written < 0) {
            posix_check((int)written, fmt::format("trying to write {:d} bytes to {}",
                                                  bytes_to_write, m_flac_path));
        }
        completed += written;
    }
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

} // namespace FLACStreaming
