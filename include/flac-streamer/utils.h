#pragma once

#include "common.h"

#include <stdexcept>

namespace FLACStreaming {

void posix_check(int retval, const std::string &msg);

}
