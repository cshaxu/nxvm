#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

#define T314_CRTC_ADJACENT_INDEX 0x12u

_Static_assert(T314_CRTC_ADJACENT_INDEX < CORE_MACHINE_VADP_CRTC_REGISTER_COUNT,
    "T314 adjacent CRTC test index must fit the VADP CRTC register bank");

static void core_machine_ega_crtc_write(t_port *port, lib_u8 index,
    lib_u8 value)
{
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, index);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_DATA, value);
}

static lib_u8 core_machine_ega_crtc_read(t_port *port, lib_u8 index)
{
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, index);
    return core_machine_port_read(port, CORE_MACHINE_VADP_PORT_CRTC_DATA);
}

lib_i32 main(void)
{
    const core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
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
    core_machine_display_snapshot snapshot;
    lib_u8 mode;
    lib_u8 color;
    lib_u8 index;
    static const lib_u8 masks[CORE_MACHINE_VADP_CRTC_REGISTER_COUNT] = {
        0xffu, 0xffu, 0xffu, 0x7fu, 0xffu, 0xffu, 0xffu, 0x3fu,
        0x1fu, 0x1fu, 0x1fu, 0x7fu, 0xffu, 0xffu, 0xffu, 0xffu,
        0xffu, 0x3fu, 0xffu, 0xffu, 0x1fu, 0xffu, 0x1fu, 0xffu,
        0xffu
    };
    lib_i32 failed = 0;

    lib_memory_set(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    failed |= core_machine_memory_initialize_for(&memory, 16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &sequencer) != LIB_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_controllers(&vadp,
        &controllers) != LIB_STATUS_OK;

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, 2u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, 0x0fu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    (void)core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x30u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x01u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MODE, 0x1au);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COLOR, 0x35u);
    mode = core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MODE);
    color = core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COLOR);
    vadp.data.crtc[T314_CRTC_ADJACENT_INDEX] = 0x5du;
    core_machine_ega_crtc_write(&port, 0x01u, 0x4fu);
    core_machine_ega_crtc_write(&port, 0x07u, 0x02u);
    core_machine_ega_crtc_write(&port, 0x13u, 0x28u);

    failed |= core_machine_ega_crtc_read(&port, 0x13u) != 0u ||
        vadp.data.crtc[0x13u] != 0x28u ||
        vadp.data.crtc[T314_CRTC_ADJACENT_INDEX] != 0x5du ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MODE) != mode ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COLOR) != color;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16;

    core_machine_ega_crtc_write(&port, 0x07u, 0x00u);
    core_machine_ega_crtc_write(&port, 0x12u, 0xc7u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_640X200X16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 200u;

    core_machine_ega_crtc_write(&port, 0x01u, 0x27u);
    core_machine_ega_crtc_write(&port, 0x13u, 0x14u);
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16;

    for (index = 0u; index <= CORE_MACHINE_VADP_CRTC_EGA_LAST; ++index) {
        core_machine_ega_crtc_write(&port, index, 0xffu);
        failed |= vadp.data.crtc[index] != masks[index] ||
            core_machine_ega_crtc_read(&port, index) !=
            (index >= 0x0cu && index <= 0x0fu ? masks[index] : 0u);
    }
    core_machine_vadp_reset(&vadp);
    failed |= vadp.data.crtc[CORE_MACHINE_VADP_CRTC_EGA_LAST] != 0u ||
        vadp.data.crtc[0x17u] != 0u || vadp.data.crtc[0x0au] != 6u ||
        vadp.data.crtc[0x0bu] != 7u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) {
        fprintf(stderr, "M5:T314:S2:EGA-CRTC-BOUNDARY:FAIL\n");
        return 1;
    }
    printf("M5:T314:S2:EGA-CRTC-BOUNDARY:OK\n");
    return 0;
}
