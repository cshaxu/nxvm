#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_VOID core_machine_vadp_write_crtc(t_port *port, type_unsigned_8 index,
    type_unsigned_8 value)
{
    core_machine_port_write(port, 0x03d4u, index);
    core_machine_port_write(port, 0x03d5u, value);
}

C_INT main(C_VOID)
{
    t_port port;
    t_ram memory;
    t_vadp vadp;
    core_machine_display_snapshot snapshot;
    type_unsigned_8 value;
    C_INT saw_vertical_retrace = TYPE_FALSE;
    C_INT saw_display_interference = TYPE_FALSE;
    C_INT saw_buffer_access = TYPE_FALSE;
    core_machine_vadp_text_timing timing = { 3u, 2u, 1u };
    type_unsigned_8 status;
    STD_SIZE_T refresh;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    failed |= core_machine_vadp_configure_text_timing(&vadp, &timing) !=
        TYPE_STATUS_OK;
    core_machine_vadp_reset(&vadp);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MODE, 0x0du);
    failed |= core_machine_port_read(&port, 0x03dau) != 0x00u;
    value = 'A';
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_TEXT_BASE, (type_virtual_address)&value,
        sizeof(value)) != TYPE_STATUS_OK;
    value = 0x1fu;
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_TEXT_BASE + 1u, (type_virtual_address)&value,
        sizeof(value)) != TYPE_STATUS_OK;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_text_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.columns != 80u || snapshot.rows != 25u ||
        snapshot.characters[0] != 'A' || snapshot.attributes[0] != 0x1fu ||
        !snapshot.buffer_changed;

    core_machine_vadp_write_crtc(&port, 0x0eu, 0u);
    core_machine_vadp_write_crtc(&port, 0x0fu, 1u);
    core_machine_vadp_write_crtc(&port, 0x0au, 2u);
    core_machine_vadp_write_crtc(&port, 0x0bu, 6u);
    core_machine_port_write(&port, 0x03d9u, 0x1eu);
    status = core_machine_port_read(&port, 0x03dau);
    failed |= status != 0x00u || core_machine_port_read(&port, 0x03dau) != status;
    for (refresh = 0u; refresh < 2u * 6u; ++refresh) {
        core_machine_vadp_advance(&vadp, &memory, 1u);
        status = core_machine_port_read(&port, 0x03dau);
        if ((status & 0x08u) != 0u) {
            saw_vertical_retrace = TYPE_TRUE;
        } else if ((status & 0x01u) != 0u) {
            saw_buffer_access = TYPE_TRUE;
        } else {
            saw_display_interference = TYPE_TRUE;
        }
    }
    failed |= !saw_vertical_retrace || !saw_display_interference ||
        !saw_buffer_access;
    failed |= !core_machine_vadp_capture_text_snapshot(&vadp, &memory, &snapshot);
    failed |= !snapshot.cursor_changed || snapshot.cursor_x != 1u ||
        snapshot.cursor_y != 0u || snapshot.cursor_top != 2u ||
        snapshot.cursor_bottom != 6u;

    value = 'B';
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_TEXT_BASE + 2u, (type_virtual_address)&value,
        sizeof(value)) != TYPE_STATUS_OK;
    failed |= !core_machine_vadp_capture_text_snapshot(&vadp, &memory, &snapshot);
    failed |= !snapshot.buffer_changed || snapshot.characters[1] != 'B';

    /* T228 admits 320x200x4; 640x200 remains outside the text controller. */
    core_machine_port_write(&port, 0x03d8u, 0x12u);
    failed |= !core_machine_vadp_capture_text_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T193:S2:VADP-TEXT:OK\n");
    return 0;
}
