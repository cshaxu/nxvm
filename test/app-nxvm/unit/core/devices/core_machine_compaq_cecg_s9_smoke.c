#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_board_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/vadp.h"

static lib_i32 t386_s9_invalid_cecg_is_failure_atomic(void)
{
    core_machine_config machine_config = {0};
    core_machine_display_config display_config = {0};
    core_machine *machine = LIB_NULL;
    lib_status status;

    machine_config.memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    display_config.text_timing = (x86_video_text_timing) {48u, 8u, 8u};
    display_config.ega_present = LIB_TRUE;
    display_config.ega_personality =
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
    display_config.ega_sequencer = (x86_video_ega_sequencer_config) {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE };
    display_config.ports = (core_machine_display_port_topology) {
        CORE_MACHINE_VADP_PORT_ATTRIBUTE, CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ,
        CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA,
        CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA,
        CORE_MACHINE_VADP_PORT_CRTC_INDEX, CORE_MACHINE_VADP_PORT_STATUS };
    status = core_machine_create(&machine_config, &machine, LIB_NULL);
    if (status == LIB_STATUS_OK) status = core_machine_configure_display(machine,
        &display_config);
    if (status == LIB_STATUS_INVALID_ARGUMENT) {
        display_config.ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC;
        status = core_machine_configure_display(machine, &display_config);
    }
    core_machine_destroy(machine);
    return status == LIB_STATUS_OK;
}
lib_i32 main(void)
{
    static const struct {
        lib_bool generic;
        lib_u16 address;
        lib_bool write;
        lib_bool present;
    } expected[] = {
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE, LIB_FALSE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE, LIB_FALSE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE, LIB_FALSE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_FALSE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_TRUE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET, LIB_TRUE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET, LIB_TRUE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_FALSE, LIB_FALSE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL, LIB_TRUE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR, LIB_TRUE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE, LIB_FALSE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, LIB_TRUE, LIB_TRUE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_MONO_STATUS, LIB_FALSE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, LIB_TRUE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE, LIB_FALSE },
        { LIB_FALSE, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, LIB_FALSE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0, LIB_FALSE, LIB_TRUE },
        { LIB_TRUE, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE, LIB_FALSE },
    };
    core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u},
        .ega_present = LIB_TRUE,
        .ega_sequencer = {0xa0000u, 0x10000u, 3u, 0u, 15u, 2u, LIB_TRUE},
        .cecg = {0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
            0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE}
    };
    lib_i32 failed = !t386_s9_invalid_cecg_is_failure_atomic();

    for (lib_u8 generic = 0u; generic < 2u; ++generic) {
        core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_port *port = &machine.executor_port;
        t_ram *memory = &machine.executor_memory;
        t_vadp adapter;

        core_machine_port_initialize(port);
        if (core_machine_memory_initialize_for(memory, 0x100000u, LIB_NULL) !=
                LIB_STATUS_OK) return 1;
        if (core_machine_vadp_initialize(&adapter, &machine) != LIB_STATUS_OK) return 1;
        config.ega_personality = generic ? X86_VIDEO_EGA_PERSONALITY_GENERIC :
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        failed |= core_machine_vadp_configure(&adapter, &config) != LIB_STATUS_OK;
        for (lib_size index = 0u; index < sizeof(expected) / sizeof(expected[0]); ++index) {
            if (expected[index].generic != generic) continue;
            failed |= (expected[index].write ?
                core_machine_port_has_write(port, expected[index].address) :
                core_machine_port_has_read(port, expected[index].address)) !=
                    expected[index].present;
        }
        core_machine_vadp_finalize(&adapter);
        core_machine_memory_finalize(memory);
        core_machine_port_finalize(port);
    }
    return failed ? 1 : 0;
}
