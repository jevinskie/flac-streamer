#pragma once

#include "common.h"
#include "dr_libs.h"

#include <cstdio>

#include <FLAC++/encoder.h>

namespace FLACStreaming {

using namespace dr_libs::dr_wav;

class FLACSTREAMER_EXPORT FLACStreamer : public FLAC::Encoder::Stream {
public:
    FLACStreamer(const fs::path &wav_path, const fs::path &flac_path);
    ~FLACStreamer();
    void encode();

private:
    ::FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes,
                                                    uint32_t samples,
                                                    uint32_t current_frame) override;

private:
    fs::path m_flac_path;
    drwav m_wav;
    int m_out_fd   = -1;
    FILE *m_out_fh = nullptr;
};

} // namespace FLACStreaming
