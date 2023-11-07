#include "flac-streamer/flac-streamer.h"
#include "common-internal.h"
#include "flac-streamer/utils.h"

#undef NDEBUG
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <vector>

namespace FLACStreaming {

using namespace dr_libs::dr_wav;

FLACStreamer::FLACStreamer(const fs::path &wav_path, const fs::path &flac_path)
    : m_flac_path{flac_path} {
    fmt::print("FLACStreamer ctor\n");
    if (!drwav_init_file(&m_wav, wav_path.c_str(), nullptr)) {
        throw std::runtime_error(fmt::format("Failed to open input WAV file: {}", wav_path));
    }
    set_verify(true);
    set_channels(m_wav.channels);
    set_bits_per_sample(m_wav.bitsPerSample);
    set_sample_rate(m_wav.sampleRate);
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
    fmt::print("FLACStreamer dtor\n");
    drwav_uninit(&m_wav);
    if (m_out_fh) {
        const auto close_res = fclose(m_out_fh);
        posix_check(close_res, fmt::format("trying to fclose() {}", m_flac_path));
        m_out_fh = nullptr;
    }
    m_out_fd = -1;
}

void FLACStreamer::encode() {
    if (const auto init_res = init(); init_res != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        throw std::runtime_error(fmt::format("FLAC::Encoder::Stream::init() failed with: {}",
                                             std::to_underlying(init_res)));
    }
    const auto num_samples  = m_wav.totalPCMFrameCount;
    const auto num_channels = m_wav.channels;
    const auto num_bits     = m_wav.bitsPerSample;
    // fmt::print(stderr, "num_samples: {:d} num_channels: {:d}\n", num_samples, num_channels);
    std::vector<::FLAC__int32> buf(num_channels * num_samples);
    const auto num_samples_read = drwav_read_pcm_frames_s32(&m_wav, num_samples, buf.data());
    assert(num_samples_read == num_samples);
    for (size_t i = 0; i < buf.size(); ++i) {
        // buf[i] = (::FLAC__int32)((((double)buf[i]) * num_bits / 32.0) + 0.5);
        buf[i] = (::FLAC__int32)((double)buf[i] * 32767.0 / 2147483647.0);
        if (buf[i] > INT16_MAX) {
            buf[i] = INT16_MAX;
        } else if (buf[i] < INT16_MIN) {
            buf[i] = INT16_MIN;
        }
    }
    // fmt::print(stderr, "buf data: {} size: {}\n", fmt::ptr(buf.data()), buf.size());
    const auto process_ok = process_interleaved(buf.data(), num_samples);
    if (!process_ok) {
        throw std::runtime_error(
            fmt::format("FLAC::Encoder::Stream::process_interleaved() failed with: {}",
                        get_state().as_cstring()));
    }
    finish();
}

::FLAC__StreamEncoderWriteStatus FLACStreamer::write_callback(const FLAC__byte buffer[],
                                                              size_t bytes, uint32_t samples,
                                                              uint32_t current_frame) {
    (void)samples;
    (void)current_frame;
    // fmt::print(stderr, "wcb: bytes: {} samples: {} current_frame: {}\n", bytes, samples,
    //            current_frame);
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
