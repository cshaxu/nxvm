/* Same-owner register fixture; no PC port decode or board implementation. */
#ifndef X86_VIDEO_TEST_H
#define X86_VIDEO_TEST_H
#include "x86/devices/video/video.h"

/* Caller-owned text/CGA backing bytes, never a copy of device-owned VRAM. */
typedef struct video_test_backing {
    lib_u8 bytes[X86_VIDEO_TEXT_BYTES];
    lib_u32 calls;
    lib_u32 fail_on;
} video_test_backing;

static inline lib_status video_test_backing_read(void *context, lib_u32 address,
    lib_u8 *destination, lib_size bytes)
{
    video_test_backing *fixture = context;

    if (address < X86_VIDEO_TEXT_BASE ||
        (lib_u64)address - X86_VIDEO_TEXT_BASE + bytes > sizeof(fixture->bytes))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (++fixture->calls == fixture->fail_on) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(destination, fixture->bytes + address - X86_VIDEO_TEXT_BASE, bytes);
    return LIB_STATUS_OK;
}

static inline lib_status video_test_backing_write(video_test_backing *fixture,
    lib_u32 address, const lib_u8 *source, lib_size bytes)
{
    if (address < X86_VIDEO_TEXT_BASE ||
        (lib_u64)address - X86_VIDEO_TEXT_BASE + bytes > sizeof(fixture->bytes))
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(fixture->bytes + address - X86_VIDEO_TEXT_BASE, source, bytes);
    return LIB_STATUS_OK;
}

static inline lib_u8 video_test_read(x86_video *video, x86_video_register reg)
{
    lib_u8 value = 0u;

    return x86_video_register_read(video, reg, &value) == LIB_STATUS_OK ?
        value : 0xffu;
}

static inline lib_status video_test_cga_read(void *context, lib_u32 address,
    lib_u8 *destination, lib_size bytes)
{
    return x86_video_memory_read(context, X86_VIDEO_MEMORY_CGA, address,
        destination, bytes);
}

static inline void video_test_write(x86_video *video, x86_video_register reg,
    lib_u8 value)
{
    (void)x86_video_register_write(video, reg, value);
}
#endif
