#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static void vadp_write_crtc(x86_video *port, lib_u8 index, lib_u8 value)
{
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, value);
}

static lib_u8 vadp_read_crtc(x86_video *port, lib_u8 index)
{
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    return video_test_read(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA);
}

static lib_i32 vadp_capture(x86_video *vadp, video_test_backing *memory,
    x86_video_snapshot *snapshot)
{
    lib_memory_set(snapshot, 0, sizeof(*snapshot));
    const x86_video_memory_reader reader = { video_test_backing_read, memory };

    return x86_video_capture_text_snapshot_from(vadp, &reader, snapshot);
}

lib_i32 main(void)
{
    x86_video_text_timing timing = { 3u, 2u, 1u };
    x86_video_snapshot snapshot;
    video_test_backing memory = {0};
    x86_video *vadp = LIB_NULL;
    lib_u8 value;
    lib_u8 initial_status;
    lib_i32 failed = 0;

    lib_memory_set(&memory, 0, sizeof(memory));
    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_text_timing(vadp, &timing) !=
        LIB_STATUS_OK;
    x86_video_reset(vadp);
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x0du);

    initial_status = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= initial_status != 0x00u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != initial_status;

    vadp_write_crtc(vadp, 0x0au, 0xffu);
    vadp_write_crtc(vadp, 0x0bu, 0xffu);
    vadp_write_crtc(vadp, 0x0cu, 0xffu);
    vadp_write_crtc(vadp, 0x0eu, 0xffu);
    failed |= vadp_read_crtc(vadp, 0x0au) != 0u ||
        vadp_read_crtc(vadp, 0x0bu) != 0u ||
        vadp_read_crtc(vadp, 0x0cu) != 0u ||
        vadp_read_crtc(vadp, 0x0eu) != 0x3fu;

    value = 'P';
    failed |= video_test_backing_write(&memory, 0x000b9000u,
        &value, sizeof(value)) != LIB_STATUS_OK;
    vadp_write_crtc(vadp, 0x0cu, 0x08u);
    vadp_write_crtc(vadp, 0x0du, 0x00u);
    vadp_write_crtc(vadp, 0x0eu, 0x08u);
    vadp_write_crtc(vadp, 0x0fu, 0x51u);
    vadp_write_crtc(vadp, 0x0au, 0x06u);
    vadp_write_crtc(vadp, 0x0bu, 0x06u);
    failed |= !vadp_capture(vadp, &memory, &snapshot) ||
        snapshot.characters[0] != 'P' || !snapshot.cursor_visible ||
        snapshot.cursor_x != 1u || snapshot.cursor_y != 1u ||
        snapshot.cursor_top != 6u || snapshot.cursor_bottom != 6u;

    vadp_write_crtc(vadp, 0x0au, 0x26u);
    failed |= !vadp_capture(vadp, &memory, &snapshot) || snapshot.cursor_visible ||
        snapshot.cursor_top != 6u || snapshot.cursor_bottom != 6u;

    vadp_write_crtc(vadp, 0x0au, 0x06u);
    vadp_write_crtc(vadp, 0x0eu, 0x0fu);
    vadp_write_crtc(vadp, 0x0fu, 0xd0u);
    failed |= !vadp_capture(vadp, &memory, &snapshot) || snapshot.cursor_visible ||
        snapshot.cursor_x != 0u || snapshot.cursor_y != 0u;

    value = 'W';
    failed |= video_test_backing_write(&memory, 0x000bbffeu,
        &value, sizeof(value)) != LIB_STATUS_OK;
    value = 'R';
    failed |= video_test_backing_write(&memory, X86_VIDEO_TEXT_BASE,
        &value, sizeof(value)) != LIB_STATUS_OK;
    vadp_write_crtc(vadp, 0x0cu, 0x1fu);
    vadp_write_crtc(vadp, 0x0du, 0xffu);
    failed |= !vadp_capture(vadp, &memory, &snapshot) ||
        snapshot.characters[0] != 'W' || snapshot.characters[1] != 'R';

    x86_video_advance(vadp, 2u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x00u;
    x86_video_advance(vadp, 1u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x01u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x01u;
    x86_video_advance(vadp, 2u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x09u;
    x86_video_advance(vadp, 1u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x00u;

    vadp_write_crtc(vadp, 0x00u, 0x04u);
    vadp_write_crtc(vadp, 0x01u, 0x03u);
    vadp_write_crtc(vadp, 0x02u, 0x03u);
    vadp_write_crtc(vadp, 0x03u, 0x11u);
    vadp_write_crtc(vadp, 0x04u, 0xffu);
    vadp_write_crtc(vadp, 0x05u, 0xffu);
    vadp_write_crtc(vadp, 0x06u, 0xffu);
    vadp_write_crtc(vadp, 0x07u, 0xffu);
    vadp_write_crtc(vadp, 0x08u, 0xffu);
    vadp_write_crtc(vadp, 0x09u, 0xffu);
    failed |= vadp_read_crtc(vadp, 0x00u) != 0u ||
        vadp_read_crtc(vadp, 0x01u) != 0u ||
        vadp_read_crtc(vadp, 0x02u) != 0u ||
        vadp_read_crtc(vadp, 0x03u) != 0u ||
        vadp_read_crtc(vadp, 0x04u) != 0u ||
        vadp_read_crtc(vadp, 0x05u) != 0u ||
        vadp_read_crtc(vadp, 0x06u) != 0u ||
        vadp_read_crtc(vadp, 0x07u) != 0u ||
        vadp_read_crtc(vadp, 0x08u) != 0u ||
        vadp_read_crtc(vadp, 0x09u) != 0u;
    vadp_write_crtc(vadp, 0x04u, 0x02u);
    vadp_write_crtc(vadp, 0x05u, 0x01u);
    vadp_write_crtc(vadp, 0x06u, 0x02u);
    vadp_write_crtc(vadp, 0x07u, 0x02u);
    vadp_write_crtc(vadp, 0x09u, 0x03u);
    vadp_write_crtc(vadp, 0x0cu, 0u);
    vadp_write_crtc(vadp, 0x0du, 0u);
    vadp_write_crtc(vadp, 0x0eu, 0u);
    vadp_write_crtc(vadp, 0x0fu, 0u);
    x86_video_reset(vadp);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x01u ||
        !vadp_capture(vadp, &memory, &snapshot) || snapshot.columns != 3u ||
        snapshot.rows != 2u;
    x86_video_advance(vadp, 65u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x00u;
    x86_video_advance(vadp, 3u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x01u;
    x86_video_advance(vadp, 37u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS) != 0x09u;

    x86_video_destroy(vadp);
    if (failed) return 1;
    lib_c_printf("M5:T266:S3:VADP-TEXT-STATUS:OK\n");
    lib_c_printf("M5:T375:S8:MODEL339-CGA-CLOCK-RECONCILIATION:OK\n");
    lib_c_printf("M5:T375:S11:CGA-LOGICAL-RASTER:OK\n");
    return 0;
}
