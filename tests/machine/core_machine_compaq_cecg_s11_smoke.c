#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT t386_s11_query_route(t_ram *memory, core_machine_memory_route expected)
{
    core_machine_memory_route route;

    return core_machine_memory_query_physical(memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) == TYPE_STATUS_OK && route == expected;
}

C_INT main(C_VOID)
{
    const core_machine_vadp_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE
    };
    t_port port;
    t_port generic_port;
    t_ram memory;
    t_ram generic_memory;
    t_vadp vadp;
    t_vadp generic_vadp;
    core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE
    };
    core_machine_vadp_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
          0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
          0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_memory_initialize(&memory);
    core_machine_memory_initialize(&generic_memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&generic_vadp, &generic_port);
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_cecg(&vadp, &config) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_sequencer(&vadp, &memory, &sequencer) !=
            TYPE_STATUS_OK || core_machine_vadp_configure_ega_controllers(&vadp,
            &controllers) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_personality(&generic_vadp,
        &generic_port, CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_sequencer(&generic_vadp, &generic_memory,
            &sequencer) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_controllers(&generic_vadp, &controllers) !=
            TYPE_STATUS_OK;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    core_machine_port_write(&generic_port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&generic_port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    failed |= !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_write(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !t386_s11_query_route(&memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    core_machine_port_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x02u);
    failed |= !t386_s11_query_route(&memory, CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM) ||
        !t386_s11_query_route(&generic_memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    core_machine_vadp_reset(&vadp);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    failed |= !t386_s11_query_route(&memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&generic_memory);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        STD_PRINTF("M5:T386:S11:CECG-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S11:CECG-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}
