#include "flac-streamer/flac-streamer.h"
#include "common-internal.h"

#include <stdexcept>

namespace FLACStreaming {

// using namespace dr_libs::dr_wav;

FLACStreamer::FLACStreamer(const fs::path &wave_path, const fs::path &flac_path) {
    fmt::print("FLACStreamer ctor\n");
    if (!drwav_init_file(&m_wav, wave_path.c_str(), nullptr)) {
        throw std::runtime_error("Failed to open WAV file");
    }
    set_verify(true);
    set_compression_level(5); // default level
}

FLACStreamer::~FLACStreamer() {
    fmt::print("FLACStreamer dtor\n");
}

::FLAC__StreamEncoderWriteStatus FLACStreamer::write_callback(const FLAC__byte buffer[],
                                                              size_t bytes, uint32_t samples,
                                                              uint32_t current_frame) {
    std::fwrite(buffer, sizeof(FLAC__byte), bytes, stdout);
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

} // namespace FLACStreaming
