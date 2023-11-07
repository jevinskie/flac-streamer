#include "flac-streamer/utils.h"
#include "common-internal.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace FLACStreaming {

void posix_check(int retval, const std::string &msg) {
    const auto orig_errno = errno;
    if (FLACSTREAMER_UNLIKELY(retval < 0)) {
        throw std::runtime_error(
            fmt::format("POSIX error: '{:s}' retval: {:d} errno: {:d} description: '{:s}'", msg,
                        retval, orig_errno, strerror(orig_errno)));
    }
}

} // namespace FLACStreaming
