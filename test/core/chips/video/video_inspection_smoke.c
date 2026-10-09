/* Copyright 2012-2026 Neko. */
#include "video_test.h"

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const lib_u8 sentinel[4] = {0x12u, 0x34u, 0x56u, 0x78u};
    const x86_video_ega_controller_config controllers = {0};
    const lib_u8 memory_modes[] = {0x02u, 0x06u, 0x0eu};
    x86_video *video = LIB_NULL;
    lib_u8 observed[4], actual[4];
    lib_size mode, plane, offset;
    lib_u8 read_mode, map;
    lib_bool failed = LIB_FALSE;

    if (x86_video_create(&video) != LIB_STATUS_OK) return 1;
    if (x86_video_configure_ega_sequencer(video, &sequencer) != LIB_STATUS_OK ||
        x86_video_configure_ega_controllers(video, &controllers) != LIB_STATUS_OK ||
        x86_video_configure_vga(video) != LIB_STATUS_OK) {
        x86_video_destroy(video);
        return 1;
    }
    video->data.graphics[6] = 0x05u;
    video->data.graphics[2] = 0x05u;
    video->data.graphics[7] = 0u;
    for (plane = 0u; plane < 4u; ++plane)
        for (offset = 0u; offset < 4u; ++offset)
            video->data.ega_planar_vram[plane * X86_VIDEO_EGA_PLANE_BYTES + offset] =
                (lib_u8)(0x81u + plane * 17u + offset);

    for (mode = 0u; mode < sizeof(memory_modes); ++mode)
        for (read_mode = 0u; read_mode < 2u; ++read_mode)
            for (map = 0u; map < 4u; ++map) {
                video->data.sequencer[4] = memory_modes[mode];
                video->data.graphics[5] = (lib_u8)(read_mode << 3u);
                video->data.graphics[4] = map;
                lib_memory_copy(video->data.ega_latches, sentinel, sizeof(sentinel));
                failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, observed, sizeof(observed)) != LIB_STATUS_OK;
                failed |= lib_memory_compare(video->data.ega_latches, sentinel,
                    sizeof(sentinel)) != 0;
                failed |= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                    0xa0000u, actual, sizeof(actual)) != LIB_STATUS_OK;
                failed |= lib_memory_compare(actual, observed, sizeof(actual)) != 0;
                failed |= lib_memory_compare(video->data.ega_latches, sentinel,
                    sizeof(sentinel)) == 0;
            }

    failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_PLANAR,
        0xb0000u, observed, 1u) != LIB_STATUS_UNSUPPORTED;
    failed |= x86_video_memory_inspect(video, (x86_video_memory_region)99,
        0xa0000u, observed, 1u) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_video_configure_cga_memory(video) != LIB_STATUS_OK;
    failed |= x86_video_memory_write(video, X86_VIDEO_MEMORY_CGA, 0xb8000u,
        sentinel, sizeof(sentinel)) != LIB_STATUS_OK;
    failed |= x86_video_memory_inspect(video, X86_VIDEO_MEMORY_CGA, 0xb8000u,
        observed, sizeof(observed)) != LIB_STATUS_OK;
    failed |= lib_memory_compare(observed, sentinel, sizeof(sentinel)) != 0;
    x86_video_destroy(video);
    return failed ? 1 : 0;
}
