#pragma once

#define FLACSTREAMER_EXPORT __attribute__((visibility("default")))
#define FLACSTREAMER_INLINE __attribute__((always_inline))
// #define FLACSTREAMER_INLINE
#define FLACSTREAMER_NOINLINE __attribute__((noinline))
#define FLACSTREAMER_LIKELY(cond) __builtin_expect((cond), 1)
#define FLACSTREAMER_UNLIKELY(cond) __builtin_expect((cond), 0)
#define FLACSTREAMER_BREAK() __builtin_debugtrap()
#define FLACSTREAMER_ALIGNED(n) __attribute__((aligned(n)))
#define FLACSTREAMER_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), n)
#define FLACSTREAMER_UNREACHABLE() __builtin_unreachable()
