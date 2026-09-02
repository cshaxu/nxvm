#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT t386_s28_write(t_ram *memory, type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT t386_s28_read(t_ram *memory, type_unsigned_8 *value)
{
    return core_machine_memory_read_physical(memory, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

static C_INT t386_s28_write_at(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT t386_s28_read_at(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

static C_VOID t386_s28_select_ega_320(t_port *port)
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
    const core_machine_vadp_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE
    };
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
    t_port generic_port;
    t_ram memory;
    t_vadp vadp;
    t_vadp generic_vadp;
    core_machine_display_snapshot snapshot;
    type_unsigned_8 value = 0u;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&generic_vadp, &generic_port);
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_cecg(&vadp, &config) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_sequencer(&vadp, &memory, &sequencer) !=
        TYPE_STATUS_OK || core_machine_vadp_configure_ega_controllers(&vadp,
        &controllers) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_personality(&generic_vadp, &generic_port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != TYPE_STATUS_OK;
    failed |= !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_write(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE);
    t386_s28_select_ega_320(&port);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
    failed |= !t386_s28_write(&memory, 0x80u) || !t386_s28_read(&memory, &value) ||
        value != 0x80u || !core_machine_vadp_capture_snapshot(&vadp, &memory,
        &snapshot) || snapshot.pixels[0] != 15u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x20u);
    failed |= !t386_s28_write(&memory, 0x00u) || !t386_s28_read(&memory, &value) ||
        value != 0x00u || !core_machine_vadp_capture_snapshot(&vadp, &memory,
        &snapshot) || snapshot.pixels[0] != 0u || !snapshot.buffer_changed;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x00u);
    failed |= !t386_s28_read(&memory, &value) || value != 0x80u ||
        !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.pixels[0] != 15u || !snapshot.buffer_changed;
    /* DeskPro POST writes B0000h while its primary CECG route is 3Dx/B8000h.
     * Both addresses must reach the one VADP planar store, never ordinary RAM. */
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x01u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x0eu);
    failed |= !t386_s28_write_at(&memory, 0x000b0000u, 0x11u) ||
        !t386_s28_write_at(&memory, 0x000b8000u, 0x22u) ||
        !t386_s28_read_at(&memory, 0x000b0000u, &value) || value != 0x22u;
    core_machine_vadp_reset(&vadp);
    t386_s28_select_ega_320(&port);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x07u);
    failed |= !t386_s28_read(&memory, &value) || value != 0u ||
        !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.pixels[0] != 0u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T386:S28:CECG-ODD-EVEN-PAGE:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T386:S28:CECG-ODD-EVEN-PAGE:OK\n");
    return 0;
}
