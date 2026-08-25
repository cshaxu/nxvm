#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_VOID vadp_write_crtc(t_port *port, type_unsigned_8 index, type_unsigned_8 value)
{
    core_machine_port_write(port, 0x03d4u, index);
    core_machine_port_write(port, 0x03d5u, value);
}

static type_unsigned_8 vadp_read_crtc(t_port *port, type_unsigned_8 index)
{
    core_machine_port_write(port, 0x03d4u, index);
    return core_machine_port_read(port, 0x03d5u);
}

static C_INT vadp_capture(t_vadp *vadp, t_ram *memory,
    core_machine_display_snapshot *snapshot)
{
    STD_MEMSET(snapshot, 0, sizeof(*snapshot));
    return core_machine_vadp_capture_text_snapshot(vadp, memory, snapshot);
}

C_INT main(C_VOID)
{
    core_machine_vadp_text_timing timing = { 3u, 2u, 1u };
    core_machine_display_snapshot snapshot;
    t_port port;
    t_ram memory;
    t_vadp vadp;
    type_unsigned_8 value;
    type_unsigned_8 initial_status;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    failed |= core_machine_vadp_configure_text_timing(&vadp, &timing) !=
        TYPE_STATUS_OK;
    core_machine_vadp_reset(&vadp);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MODE, 0x0du);

    initial_status = core_machine_port_read(&port, 0x03dau);
    failed |= initial_status != 0x00u ||
        core_machine_port_read(&port, 0x03dau) != initial_status;

    vadp_write_crtc(&port, 0x0au, 0xffu);
    vadp_write_crtc(&port, 0x0bu, 0xffu);
    vadp_write_crtc(&port, 0x0cu, 0xffu);
    vadp_write_crtc(&port, 0x0eu, 0xffu);
    failed |= vadp_read_crtc(&port, 0x0au) != 0u ||
        vadp_read_crtc(&port, 0x0bu) != 0u ||
        vadp_read_crtc(&port, 0x0cu) != 0u ||
        vadp_read_crtc(&port, 0x0eu) != 0x3fu;

    value = 'P';
    failed |= core_machine_memory_write_physical(&memory, 0x000b9000u,
        (type_virtual_address)&value, sizeof(value)) != TYPE_STATUS_OK;
    vadp_write_crtc(&port, 0x0cu, 0x08u);
    vadp_write_crtc(&port, 0x0du, 0x00u);
    vadp_write_crtc(&port, 0x0eu, 0x08u);
    vadp_write_crtc(&port, 0x0fu, 0x51u);
    vadp_write_crtc(&port, 0x0au, 0x06u);
    vadp_write_crtc(&port, 0x0bu, 0x06u);
    failed |= !vadp_capture(&vadp, &memory, &snapshot) ||
        snapshot.characters[0] != 'P' || !snapshot.cursor_visible ||
        snapshot.cursor_x != 1u || snapshot.cursor_y != 1u ||
        snapshot.cursor_top != 6u || snapshot.cursor_bottom != 6u;

    vadp_write_crtc(&port, 0x0au, 0x26u);
    failed |= !vadp_capture(&vadp, &memory, &snapshot) || snapshot.cursor_visible ||
        snapshot.cursor_top != 6u || snapshot.cursor_bottom != 6u;

    vadp_write_crtc(&port, 0x0au, 0x06u);
    vadp_write_crtc(&port, 0x0eu, 0x0fu);
    vadp_write_crtc(&port, 0x0fu, 0xd0u);
    failed |= !vadp_capture(&vadp, &memory, &snapshot) || snapshot.cursor_visible ||
        snapshot.cursor_x != 0u || snapshot.cursor_y != 0u;

    value = 'W';
    failed |= core_machine_memory_write_physical(&memory, 0x000bbffeu,
        (type_virtual_address)&value, sizeof(value)) != TYPE_STATUS_OK;
    value = 'R';
    failed |= core_machine_memory_write_physical(&memory, CORE_MACHINE_VADP_TEXT_BASE,
        (type_virtual_address)&value, sizeof(value)) != TYPE_STATUS_OK;
    vadp_write_crtc(&port, 0x0cu, 0x1fu);
    vadp_write_crtc(&port, 0x0du, 0xffu);
    failed |= !vadp_capture(&vadp, &memory, &snapshot) ||
        snapshot.characters[0] != 'W' || snapshot.characters[1] != 'R';

    core_machine_vadp_advance(&vadp, &memory, 2u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x00u;
    core_machine_vadp_advance(&vadp, &memory, 1u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x01u ||
        core_machine_port_read(&port, 0x03dau) != 0x01u;
    core_machine_vadp_advance(&vadp, &memory, 2u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x09u;
    core_machine_vadp_advance(&vadp, &memory, 1u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x00u;

    vadp_write_crtc(&port, 0x00u, 0x04u);
    vadp_write_crtc(&port, 0x01u, 0x03u);
    vadp_write_crtc(&port, 0x02u, 0x03u);
    vadp_write_crtc(&port, 0x03u, 0x11u);
    vadp_write_crtc(&port, 0x04u, 0xffu);
    vadp_write_crtc(&port, 0x05u, 0xffu);
    vadp_write_crtc(&port, 0x06u, 0xffu);
    vadp_write_crtc(&port, 0x07u, 0xffu);
    vadp_write_crtc(&port, 0x08u, 0xffu);
    vadp_write_crtc(&port, 0x09u, 0xffu);
    failed |= vadp_read_crtc(&port, 0x00u) != 0u ||
        vadp_read_crtc(&port, 0x01u) != 0u ||
        vadp_read_crtc(&port, 0x02u) != 0u ||
        vadp_read_crtc(&port, 0x03u) != 0u ||
        vadp_read_crtc(&port, 0x04u) != 0u ||
        vadp_read_crtc(&port, 0x05u) != 0u ||
        vadp_read_crtc(&port, 0x06u) != 0u ||
        vadp_read_crtc(&port, 0x07u) != 0u ||
        vadp_read_crtc(&port, 0x08u) != 0u ||
        vadp_read_crtc(&port, 0x09u) != 0u;
    vadp_write_crtc(&port, 0x04u, 0x02u);
    vadp_write_crtc(&port, 0x05u, 0x01u);
    vadp_write_crtc(&port, 0x06u, 0x02u);
    vadp_write_crtc(&port, 0x07u, 0x02u);
    vadp_write_crtc(&port, 0x09u, 0x03u);
    vadp_write_crtc(&port, 0x0cu, 0u);
    vadp_write_crtc(&port, 0x0du, 0u);
    vadp_write_crtc(&port, 0x0eu, 0u);
    vadp_write_crtc(&port, 0x0fu, 0u);
    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x01u ||
        !vadp_capture(&vadp, &memory, &snapshot) || snapshot.columns != 3u ||
        snapshot.rows != 2u;
    core_machine_vadp_advance(&vadp, &memory, 65u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x00u;
    core_machine_vadp_advance(&vadp, &memory, 3u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x01u;
    core_machine_vadp_advance(&vadp, &memory, 37u);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x09u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T266:S3:VADP-TEXT-STATUS:OK\n");
    STD_PRINTF("M5:T375:S8:MODEL339-CGA-CLOCK-RECONCILIATION:OK\n");
    STD_PRINTF("M5:T375:S11:CGA-LOGICAL-RASTER:OK\n");
    return 0;
}
