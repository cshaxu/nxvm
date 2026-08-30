#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_ega_planar_write(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT core_machine_ega_planar_read(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

static C_VOID core_machine_ega_graphics_write(t_port *port, type_unsigned_8 index,
    type_unsigned_8 value)
{
    core_machine_port_write(port, 0x03ceu, index);
    core_machine_port_write(port, 0x03cfu, value);
}

static C_VOID core_machine_ega_planar_select_mode_d(t_port *port)
{
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x01u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x27u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x07u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x00u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x12u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0xc7u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x13u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x14u);
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE
    };
    const core_machine_vadp_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    type_unsigned_8 value = 0u;
    type_unsigned_8 status_first = 0u;
    type_unsigned_8 status_second = 0u;
    core_machine_display_snapshot snapshot;
    core_machine_display_kind copied_kind;
    type_unsigned_8 copied_pixel_zero;
    type_unsigned_8 copied_pixel_two;
    type_unsigned_32 copied_palette_fifteen;
    core_machine_memory_route route;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &sequencer) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_controllers(&vadp,
        &controllers) != TYPE_STATUS_OK;
    core_machine_ega_planar_select_mode_d(&port);

    status_first = core_machine_port_read(&port, 0x03dau);
    status_second = core_machine_port_read(&port, 0x03dau);
    failed |= (status_first & 0x30u) != 0x30u || (status_second & 0x30u) != 0u;

    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010000u);
    failed |= core_machine_vadp_ega_aperture_contains(&vadp, 0x000b0000u, 1u);
    failed |= core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010001u);

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x0fu);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x0fu;
    core_machine_port_write(&port, 0x03ceu, 5u);
    core_machine_port_write(&port, 0x03cfu, 0x00u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x00u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x05u;
    core_machine_port_write(&port, 0x03cfu, 0x05u);
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x30u);
    core_machine_port_write(&port, 0x03c0u, 0x01u);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x01u ||
        vadp.data.attribute[16] != 0x01u;

    failed |= !core_machine_ega_planar_write(&memory, 0x000a0000u, 0xa5u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0xa5u;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 15u || snapshot.pixels[1] != 0u ||
        snapshot.pixels[2] != 15u || snapshot.palette_rgb[15] != 0xffffffu;
    copied_kind = snapshot.kind;
    copied_pixel_zero = snapshot.pixels[0];
    copied_pixel_two = snapshot.pixels[2];
    copied_palette_fifteen = snapshot.palette_rgb[15];

    /* Read mode 1 compares the four latches; mode 1 copies them and mode 2
     * expands the four low processor-data bits into the selected planes. */
    core_machine_ega_graphics_write(&port, 1u, 0u);
    core_machine_ega_graphics_write(&port, 3u, 0u);
    core_machine_ega_graphics_write(&port, 8u, 0xffu);
    core_machine_ega_graphics_write(&port, 5u, 0u);
    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x01u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0003u, 0xaau);
    core_machine_port_write(&port, 0x03c5u, 0x02u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0003u, 0x55u);
    core_machine_port_write(&port, 0x03c5u, 0x04u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0003u, 0xf0u);
    core_machine_port_write(&port, 0x03c5u, 0x08u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0003u, 0x0fu);
    core_machine_port_write(&port, 0x03c5u, 0x0fu);
    core_machine_ega_graphics_write(&port, 4u, 0u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0003u, &value) ||
        value != 0xaau;
    core_machine_ega_graphics_write(&port, 2u, 0x01u);
    core_machine_ega_graphics_write(&port, 7u, 0x0eu);
    core_machine_ega_graphics_write(&port, 5u, 0x08u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0003u, &value) ||
        value != 0xaau;
    core_machine_ega_graphics_write(&port, 5u, 0u);
    core_machine_ega_graphics_write(&port, 4u, 0x02u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0003u, &value) ||
        value != 0xf0u;
    core_machine_ega_graphics_write(&port, 4u, 0x04u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0003u, &value) ||
        value != 0u;
    core_machine_ega_graphics_write(&port, 5u, 0x01u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0004u, 0u);
    core_machine_ega_graphics_write(&port, 5u, 0u);
    core_machine_ega_graphics_write(&port, 4u, 0x03u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0004u, &value) ||
        value != 0x0fu;
    core_machine_ega_graphics_write(&port, 5u, 0x02u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0005u, 0x05u);
    core_machine_ega_graphics_write(&port, 5u, 0u);
    core_machine_ega_graphics_write(&port, 4u, 0x00u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0005u, &value) ||
        value != 0xffu;
    core_machine_ega_graphics_write(&port, 4u, 0x01u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0005u, &value) ||
        value != 0u;
    core_machine_ega_graphics_write(&port, 4u, 0x02u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0005u, &value) ||
        value != 0xffu;
    core_machine_ega_graphics_write(&port, 4u, 0x03u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0005u, &value) ||
        value != 0u;
    core_machine_ega_graphics_write(&port, 5u, 0x04u);
    failed |= core_machine_memory_query_physical(&memory, 0x000a0005u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
        !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    core_machine_ega_graphics_write(&port, 5u, 0u);

    core_machine_port_write(&port, 0x03c4u, 0u);
    core_machine_port_write(&port, 0x03c5u, 0x02u);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x02u ||
        !core_machine_ega_planar_write(&memory, 0x000a0000u, 0x00u) ||
        core_machine_memory_query_physical(&memory, 0x000a0000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
        !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    core_machine_port_write(&port, 0x03c5u, 0x01u);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x01u ||
        !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        core_machine_memory_query_physical(&memory, 0x000a0000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
        !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    core_machine_port_write(&port, 0x03c5u, 0x03u);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x03u ||
        !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0xa5u || core_machine_memory_query_physical(&memory,
        0x000a0000u, 1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) !=
        TYPE_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x02u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0001u, 0x80u);
    core_machine_port_write(&port, 0x03ceu, 4u);
    core_machine_port_write(&port, 0x03cfu, 0x01u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0001u, &value) ||
        value != 0x80u;
    core_machine_port_write(&port, 0x03ceu, 0u);
    core_machine_port_write(&port, 0x03cfu, 0x01u);
    core_machine_port_write(&port, 0x03ceu, 1u);
    core_machine_port_write(&port, 0x03cfu, 0x01u);
    core_machine_port_write(&port, 0x03ceu, 3u);
    core_machine_port_write(&port, 0x03cfu, 0x00u);
    core_machine_port_write(&port, 0x03ceu, 8u);
    core_machine_port_write(&port, 0x03cfu, 0xffu);
    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x01u);
    failed |= !core_machine_ega_planar_write(&memory, 0x000a0002u, 0x00u);
    core_machine_port_write(&port, 0x03ceu, 4u);
    core_machine_port_write(&port, 0x03cfu, 0x00u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0002u, &value) ||
        value != 0xffu;

    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x09u);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000b0000u,
        0x00008000u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0u;

    /* Reset clears the transient planar store; a guest mode write re-arms it. */
    core_machine_vadp_reset(&vadp);
    core_machine_ega_planar_select_mode_d(&port);
    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x05u);
    status_first = core_machine_port_read(&port, 0x03dau);
    status_second = core_machine_port_read(&port, 0x03dau);
    failed |= (status_first & 0x30u) != 0x30u || (status_second & 0x30u) != 0u;

    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010000u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0u;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u || !snapshot.buffer_changed;
    failed |= copied_kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 ||
        copied_pixel_zero != 15u || copied_pixel_two != 15u ||
        copied_palette_fifteen != 0xffffffu;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T238:S2:EGA-PLANAR:PORT:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T238:S2:EGA-PLANAR:PORT:OK\n");
    return 0;
}
