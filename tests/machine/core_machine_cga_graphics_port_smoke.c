#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_cga_graphics_write_byte(t_ram *memory,
    type_unsigned_32 offset, type_unsigned_8 value)
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
    failed |= core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX) ||
        core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_MODE) ||
        core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_COLOR) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_STATUS) ||
        !core_machine_port_has_write(&port, 0x03dbu) ||
        !core_machine_port_has_write(&port, 0x03dcu);

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x00u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x38u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x3fu;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0xeeu);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x3fu;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x10u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xffu);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    core_machine_port_write(&port, 0x03dbu, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;
    core_machine_port_write(&port, 0x03dcu, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) == 0u;
    core_machine_port_write(&port, 0x03dbu, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;

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

    core_machine_port_write(&port, 0x03d9u, 0x10u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.palette_rgb[1] != 0x55ff55u ||
        snapshot.palette_rgb[2] != 0xff5555u ||
        snapshot.palette_rgb[3] != 0xffff55u || !snapshot.buffer_changed;

    core_machine_port_write(&port, 0x03d8u, 0x1au);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_CGA_640X200X2 ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0xffffffu;
    core_machine_port_write(&port, 0x03d9u, 0x1fu);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.palette_rgb[0] != 0x000000u ||
        snapshot.palette_rgb[1] != 0xffffffu || snapshot.buffer_changed;
    core_machine_port_write(&port, 0x03dcu, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) == 0u;
    core_machine_vadp_reset(&vadp);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x02u) != 0u;
    core_machine_port_write(&port, 0x03d8u, 0x0du);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT;
    core_machine_port_write(&port, 0x03d8u, 0x05u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T228:S1:CGA:PORT:OK\n");
    return 0;
}
