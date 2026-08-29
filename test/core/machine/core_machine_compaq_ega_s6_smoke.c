#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT t386_s6_write_byte(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT t386_s6_configure_ega(t_vadp *vadp, t_ram *memory)
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

    return core_machine_vadp_configure_ega_sequencer(vadp, memory, &sequencer) ==
        TYPE_STATUS_OK && core_machine_vadp_configure_ega_controllers(vadp,
        &controllers) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    t_port port;
    t_port generic_port;
    t_ram memory;
    t_vadp vadp;
    t_vadp generic_vadp;
    core_machine_display_snapshot snapshot;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        (core_machine_vadp_ega_personality)2) != TYPE_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != TYPE_STATUS_OK;
    failed |= !core_machine_port_has_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE) ||
        core_machine_port_has_read(&generic_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
        0x00u || core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE) != 0x30u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE) !=
        0x01u;
    failed |= !t386_s6_configure_ega(&vadp, &memory);

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x01u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x4fu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x07u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x02u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x12u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x5du);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x13u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x28u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, 2u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, 0x0fu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    (C_VOID)core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x2fu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x21u);
    failed |= !t386_s6_write_byte(&memory, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
        0x80u);

    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ||
        snapshot.pixels[0] != 15u || snapshot.palette_rgb[15u] != 0x5500aau;

    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
        0x00u || core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE) != 0x30u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE) !=
        0x01u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        STD_PRINTF("M5:T386:S6:COMPAQ-EGA-PERSONALITY:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S6:COMPAQ-EGA-PERSONALITY:FAIL\n");
    return 1;
}
