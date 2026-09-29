#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


#define T314_CRTC_ADJACENT_INDEX 0x12u

_Static_assert(T314_CRTC_ADJACENT_INDEX < X86_VIDEO_CRTC_REGISTER_COUNT,
    "T314 adjacent CRTC test index must fit the VADP CRTC register bank");

static void core_machine_ega_crtc_write(x86_video *port, lib_u8 index,
    lib_u8 value)
{
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, value);
}

static lib_u8 core_machine_ega_crtc_read(x86_video *port, lib_u8 index)
{
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    return video_test_read(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA);
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    x86_video *vadp = LIB_NULL;
    x86_video_snapshot snapshot;
    lib_u8 mode;
    lib_u8 color;
    lib_u8 index;
    static const lib_u8 masks[X86_VIDEO_CRTC_REGISTER_COUNT] = {
        0xffu, 0xffu, 0xffu, 0x7fu, 0xffu, 0xffu, 0xffu, 0x3fu,
        0x1fu, 0x1fu, 0x1fu, 0x7fu, 0xffu, 0xffu, 0xffu, 0xffu,
        0xffu, 0x3fu, 0xffu, 0xffu, 0x1fu, 0xffu, 0x1fu, 0xffu,
        0xffu
    };
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(vadp,
        &sequencer) != LIB_STATUS_OK;
    failed |= x86_video_configure_ega_controllers(vadp,
        &controllers) != LIB_STATUS_OK;

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x1au);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR, 0x35u);
    mode = video_test_read(vadp, X86_VIDEO_REGISTER_MODE);
    color = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR);
    vadp->data.crtc[T314_CRTC_ADJACENT_INDEX] = 0x5du;
    core_machine_ega_crtc_write(vadp, 0x01u, 0x4fu);
    core_machine_ega_crtc_write(vadp, 0x07u, 0x02u);
    core_machine_ega_crtc_write(vadp, 0x13u, 0x28u);

    failed |= core_machine_ega_crtc_read(vadp, 0x13u) != 0u ||
        vadp->data.crtc[0x13u] != 0x28u ||
        vadp->data.crtc[T314_CRTC_ADJACENT_INDEX] != 0x5du ||
        video_test_read(vadp, X86_VIDEO_REGISTER_MODE) != mode ||
        video_test_read(vadp, X86_VIDEO_REGISTER_COLOR) != color;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_EGA_640X350X16;

    core_machine_ega_crtc_write(vadp, 0x07u, 0x00u);
    core_machine_ega_crtc_write(vadp, 0x12u, 0xc7u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_EGA_640X200X16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 200u;

    core_machine_ega_crtc_write(vadp, 0x01u, 0x27u);
    core_machine_ega_crtc_write(vadp, 0x13u, 0x14u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16;

    for (index = 0u; index <= X86_VIDEO_CRTC_EGA_LAST; ++index) {
        core_machine_ega_crtc_write(vadp, index, 0xffu);
        failed |= vadp->data.crtc[index] != masks[index] ||
            core_machine_ega_crtc_read(vadp, index) !=
            (index >= 0x0cu && index <= 0x0fu ? masks[index] : 0u);
    }
    x86_video_reset(vadp);
    failed |= vadp->data.crtc[X86_VIDEO_CRTC_EGA_LAST] != 0u ||
        vadp->data.crtc[0x17u] != 0u || vadp->data.crtc[0x0au] != 6u ||
        vadp->data.crtc[0x0bu] != 7u;

    x86_video_destroy(vadp);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "M5:T314:S2:EGA-CRTC-BOUNDARY:FAIL\n");
        return 1;
    }
    lib_c_printf("M5:T314:S2:EGA-CRTC-BOUNDARY:OK\n");
    return 0;
}
