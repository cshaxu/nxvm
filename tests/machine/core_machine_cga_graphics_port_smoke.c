#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_cga_graphics_write_byte(t_ram *memory,
    uint32_t offset, uint8_t value)
{
    return core_machine_memory_write_physical(memory,
        CORE_MACHINE_VADP_VIDEO_BASE + offset, (type_virtual_address)&value,
        sizeof(value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    t_port port;
    t_ram memory;
    t_vadp vadp;
    core_machine_display_snapshot snapshot;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);

    core_machine_port_write(&port, 0x03d8u, 0x0au);
    core_machine_port_write(&port, 0x03d9u, 0x00u);
    failed |= !core_machine_cga_graphics_write_byte(&memory, 0u, 0x1bu);
    failed |= !core_machine_cga_graphics_write_byte(&memory, 0x2000u, 0xe4u);
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.kind != CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u;
    failed |= snapshot.pixels[0] != 0u || snapshot.pixels[1] != 1u ||
        snapshot.pixels[2] != 2u || snapshot.pixels[3] != 3u;
    failed |= snapshot.pixels[320u] != 3u || snapshot.pixels[321u] != 2u ||
        snapshot.pixels[322u] != 1u || snapshot.pixels[323u] != 0u;
    failed |= snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0x00aa00u ||
        snapshot.palette_rgb[2] != 0xaa0000u ||
        snapshot.palette_rgb[3] != 0xaa5500u || !snapshot.buffer_changed;

    core_machine_port_write(&port, 0x03d9u, 0x20u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.palette_rgb[1] != 0x00aaaau ||
        snapshot.palette_rgb[2] != 0xaa00aau ||
        snapshot.palette_rgb[3] != 0xaaaaaau || !snapshot.buffer_changed;

    core_machine_port_write(&port, 0x03d8u, 0x1au);
    failed |= core_machine_port_read(&port, 0x03d8u) != 0x1au;
    core_machine_port_write(&port, 0x03d8u, 0x0du);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T228:S1:CGA:PORT:OK\n");
    return 0;
}
