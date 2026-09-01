#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT t386_s9_invalid_cecg_is_failure_atomic(C_VOID)
{
    core_machine_config machine_config = {0};
    core_machine_display_config display_config = {0};
    core_machine *machine = STD_NULL;
    type_status status;

    machine_config.memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    display_config.text_timing = (core_machine_vadp_text_timing) {48u, 8u, 8u};
    display_config.ega_present = TYPE_TRUE;
    display_config.ega_personality =
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
    display_config.ega_sequencer = (core_machine_vadp_ega_sequencer_config) {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE };
    display_config.ports = (core_machine_display_port_topology) {
        CORE_MACHINE_VADP_PORT_ATTRIBUTE, CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ,
        CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA,
        CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, CORE_MACHINE_VADP_PORT_STATUS };
    status = core_machine_create(&machine_config, &machine);
    if (status == TYPE_STATUS_OK) status = core_machine_configure_display(machine,
        &display_config);
    if (status == TYPE_STATUS_INVALID_ARGUMENT) {
        display_config.ega_personality = CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC;
        status = core_machine_configure_display(machine, &display_config);
    }
    core_machine_destroy(machine);
    return status == TYPE_STATUS_OK;
}
C_INT main(C_VOID)
{
    const core_machine_vadp_cecg_config config = {
        0x50u, 0x00u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE
    };
    t_port port;
    t_port generic_port;
    t_vadp vadp;
    t_vadp generic_vadp;
    C_INT failed = !t386_s9_invalid_cecg_is_failure_atomic();

    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&generic_vadp, &generic_port);
    failed |= core_machine_vadp_configure_ega_personality(&generic_vadp,
        &generic_port, CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_cecg(&vadp, &config) != TYPE_STATUS_OK;
    failed |= !core_machine_port_has_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x50u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0x00u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE) != 0x30u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE) != 0x01u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE,
        0x7fu);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x7fu;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE,
        0xa5u);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0xa5u;
    core_machine_port_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) !=
        0x04u;
    core_machine_port_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) !=
        0x06u;
    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x50u ||
        (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x04u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        STD_PRINTF("M5:T386:S9:CECG-CONTRACT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S9:CECG-CONTRACT:FAIL\n");
    return 1;
}
