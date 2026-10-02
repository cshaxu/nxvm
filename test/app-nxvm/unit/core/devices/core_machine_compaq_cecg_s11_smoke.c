#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/devices/machine_interface.h"

static lib_i32 t386_s11_query_route(t_ram *memory, core_machine_memory_route expected)
{
    core_machine_memory_route route;

    return core_machine_memory_query_physical(memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) == LIB_STATUS_OK && route == expected;
}

lib_i32 main(void)
{
    const x86_video_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_port *port = &machine.executor_port;
    core_machine generic_machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_port *generic_port = &generic_machine.executor_port;
    t_ram *memory = &machine.executor_memory;
    t_ram *generic_memory = &generic_machine.executor_memory;
    t_vadp vadp;
    t_vadp generic_vadp;
    x86_video_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
          0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
          0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    lib_i32 failed = 0;
    core_machine_display_config display = {
        .text_timing = {48u, 8u, 8u},
        .ega_present = LIB_TRUE,
        .ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR
    };

    core_machine_port_initialize(port);
    core_machine_port_initialize(generic_port);
    failed |= core_machine_memory_initialize_for(memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    failed |= core_machine_memory_initialize_for(generic_memory,
        16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    failed |= core_machine_vadp_initialize(&vadp, &machine) != LIB_STATUS_OK;
    failed |= core_machine_vadp_initialize(&generic_vadp, &generic_machine) != LIB_STATUS_OK;
    display.cecg = config;
    display.ega_sequencer = sequencer;
    display.ega_controllers = controllers;
    failed |= core_machine_vadp_configure(&vadp, &display) != LIB_STATUS_OK;
    display.ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC;
    failed |= core_machine_vadp_configure(&generic_vadp, &display) != LIB_STATUS_OK;
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    core_machine_port_write(generic_port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(generic_port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    failed |= !core_machine_port_has_write(port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_write(generic_port,
        CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT) ||
        core_machine_port_has_read(generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !t386_s11_query_route(memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    core_machine_port_write(port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, 0x02u);
    failed |= !t386_s11_query_route(memory, CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM) ||
        !t386_s11_query_route(generic_memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    x86_video_reset(vadp.chip);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    failed |= !t386_s11_query_route(memory, CORE_MACHINE_MEMORY_ROUTE_PROVIDER);

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(generic_memory);
    core_machine_memory_finalize(memory);
    core_machine_port_finalize(generic_port);
    core_machine_port_finalize(port);
    if (!failed) {
        printf("M5:T386:S11:CECG-CPU-VIDEO-GATE:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S11:CECG-CPU-VIDEO-GATE:FAIL\n");
    return 1;
}
