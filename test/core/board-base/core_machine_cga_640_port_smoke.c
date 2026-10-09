#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "video_fixture.h"
#include "core/board-base/vadp.h"

lib_i32 main(void)
{
    core_machine *machine;
    t_vadp vadp;
    x86_video_snapshot snapshot;
    lib_u8 pixel = 0xa0u;
    lib_u32 status;
    lib_i32 failed = 0;
    machine = test_video_create(&vadp);
    test_video_port_write(machine, 0x03d8u, 0x1au);
    test_video_port_write(machine, 0x03d9u, 0x0cu);
    failed |= core_machine_memory_write(machine,
        CORE_MACHINE_VADP_VIDEO_BASE, &pixel,
        sizeof(pixel)) != LIB_STATUS_OK;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    pixel = 0x40u;
    failed |= core_machine_memory_write(machine,
        CORE_MACHINE_VADP_VIDEO_BASE + 0x2000u, &pixel,
        sizeof(pixel)) != LIB_STATUS_OK;
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_CGA_640X200X2 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 1u || snapshot.pixels[1] != 0u ||
        snapshot.pixels[2] != 1u || snapshot.pixels[3] != 0u ||
        snapshot.pixels[640u] != 0u || snapshot.pixels[641u] != 1u ||
        snapshot.palette_rgb[0] != 0u || snapshot.palette_rgb[1] != 0xff5555u;
    status = test_video_port_read(machine, 0x03dau);
    failed |= test_video_port_read(machine, 0x03dau) != status;
    test_video_port_write(machine, 0x03d8u, 0x12u);
    test_video_port_write(machine, 0x03d8u, 0x05u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_TEXT;
    core_machine_vadp_finalize(&vadp);
    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("CGA-640:PORT:OK\n");
    return 0;
}
