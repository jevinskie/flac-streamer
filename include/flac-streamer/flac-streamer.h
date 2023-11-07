#pragma once

#include "common.h"

#include "dr_libs/dr_wav.h"
// #include <FLAC++/encoder.h>

class FLACSTREAMER_EXPORT FLACStreamer {
public:
    FLACStreamer();
    ~FLACStreamer();

private:
private:
};

#if 0
class FLACSTREAMER_EXPORT FLACStreamer : public FLAC::Encoder::Stream {
private:
    drwav wav;

public:
    FLACStreamer(const fs::path& wav_path) {
        if (!drwav_init_file(&wav, wavFilename.c_str(), nullptr)) {
            throw std::runtime_error("Failed to open WAV file");
        }
        set_verify(true);
        set_compression_level(5); // default level
    }

    ~WaveToFlacEncoder() {
        drwav_uninit(&wav);
    }

    bool initialize(int compression_level) {
        set_compression_level(compression_level);
        // Additional FLAC encoder settings for low latency can be configured here.

        // Initialize the FLAC encoder.
        return init() == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
    }

protected:
    ::FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame) override {
        std::fwrite(buffer, sizeof(FLAC__byte), bytes, stdout);
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }
};
#endif
