#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

C_INT main(C_VOID)
{
    t_port port;
    t_ram memory;
    t_vadp vadp;
    core_machine_display_snapshot snapshot;
    type_unsigned_8 pixel = 0xa0u;
    type_unsigned_32 status;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_port_write(&port, 0x03d8u, 0x1au);
    core_machine_port_write(&port, 0x03d9u, 0x0cu);
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_VIDEO_BASE, (type_virtual_address)&pixel,
        sizeof(pixel)) != TYPE_STATUS_OK;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    pixel = 0x40u;
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_VIDEO_BASE + 0x2000u, (type_virtual_address)&pixel,
        sizeof(pixel)) != TYPE_STATUS_OK;
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 1u || snapshot.pixels[1] != 0u ||
        snapshot.pixels[2] != 1u || snapshot.pixels[3] != 0u ||
        snapshot.pixels[640u] != 0u || snapshot.pixels[641u] != 1u ||
        snapshot.palette_rgb[0] != 0u || snapshot.palette_rgb[1] != 0xffffffu;
    status = core_machine_port_read(&port, 0x03dau);
    failed |= core_machine_port_read(&port, 0x03dau) != status;
    core_machine_port_write(&port, 0x03d8u, 0x12u);
    core_machine_port_write(&port, 0x03d8u, 0x05u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT;
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T254:S2:CGA-640:PORT:OK\n");
    return 0;
}
