/* Same-owner register fixture; no PC port decode or board implementation. */
#ifndef X86_VIDEO_TEST_H
#define X86_VIDEO_TEST_H
#include "core/chips/video/video.h"

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
        address - X86_VIDEO_TEXT_BASE > sizeof(fixture->bytes) ||
        bytes > sizeof(fixture->bytes) - (address - X86_VIDEO_TEXT_BASE))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (++fixture->calls == fixture->fail_on) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(destination, fixture->bytes + address - X86_VIDEO_TEXT_BASE, bytes);
    return LIB_STATUS_OK;
}

static inline lib_status video_test_backing_write(video_test_backing *fixture,
    lib_u32 address, const lib_u8 *source, lib_size bytes)
{
    if (address < X86_VIDEO_TEXT_BASE ||
        address - X86_VIDEO_TEXT_BASE > sizeof(fixture->bytes) ||
        bytes > sizeof(fixture->bytes) - (address - X86_VIDEO_TEXT_BASE))
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

/* Establish real enhanced text registers; construction defaults are not CRTC state. */
static inline void video_test_text_geometry(x86_video *video, lib_u8 columns,
    lib_u8 rows, lib_u8 height)
{
    const lib_u16 scanlines = (lib_u16)rows * height - 1u;
    const lib_u8 registers[][2] = {
        {1u, (lib_u8)(columns - 1u)},
        {7u, (lib_u8)(0x10u | (scanlines / 256u) * 2u)},
        {9u, (lib_u8)(height - 1u)},
        {0x12u, (lib_u8)scanlines},
        {0x13u, (lib_u8)(columns / 2u)},
        {0x17u, 0xe3u}, {0x18u, 0xffu}
    };

    for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index) {
        video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, registers[index][0]);
        video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, registers[index][1]);
    }
}
#endif
