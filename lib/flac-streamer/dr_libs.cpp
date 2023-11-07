#include <flac-streamer/dr_libs.h>

namespace dr_libs {
namespace dr_flac {
#define DR_DLAC_IMPLEMENTATION
#include "dr_libs/dr_flac.h"
} // namespace dr_flac
namespace dr_flac {
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"
} // namespace dr_flac
namespace dr_wav {
#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"
} // namespace dr_wav
} // namespace dr_libs
