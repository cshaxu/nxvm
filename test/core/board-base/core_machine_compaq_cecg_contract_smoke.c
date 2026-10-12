#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "video_fixture.h"
#include "core/board-base/vadp.h"

static lib_i32 invalid_cecg_is_failure_atomic(void)
{
    core_machine_config machine_config = {0};
    core_machine_display_config display_config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 valid;

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
    valid = core_machine_create(&machine_config, &machine, &board) == LIB_STATUS_OK &&
        core_machine_configure_display(board, &display_config) == LIB_STATUS_INVALID_ARGUMENT;
    if (valid) {
        display_config.ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC;
        valid = core_machine_configure_display(board, &display_config) == LIB_STATUS_OK;
    }
    core_machine_destroy(machine);
    return valid;
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
        .cecg = {0x41u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
            0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE}
    };
    lib_i32 failed = !invalid_cecg_is_failure_atomic();

    for (lib_u8 generic = 0u; generic < 2u; ++generic) {
        core_machine *machine;
        t_vadp adapter;
        machine = test_video_create(&adapter);
        config.ega_personality = generic ? X86_VIDEO_EGA_PERSONALITY_GENERIC :
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        failed |= core_machine_vadp_configure(&adapter, &config) != LIB_STATUS_OK;
        for (lib_size index = 0u; index < sizeof(expected) / sizeof(expected[0]); ++index) {
            if (expected[index].generic != generic) continue;
            failed |= (expected[index].write ?
                test_video_port_is_owned(machine, expected[index].address, LIB_TRUE) :
                test_video_port_is_owned(machine, expected[index].address, LIB_FALSE)) !=
                    expected[index].present;
        }
        if (!generic) {
            static const lib_u8 crtc[][2] = {
                {0u, 3u}, {1u, 1u}, {6u, 6u}, {7u, 0u},
                {16u, 5u}, {17u, 0x17u}, {18u, 3u}, {21u, 4u}
            };
            static const struct { lib_u8 phase; lib_u8 status; } samples[] = {
                {0u, 1u}, {2u, 0u}, {20u, 0u}, {25u, 8u}, {35u, 0u}
            };
            lib_u8 phase = 0u;

            core_machine_vadp_reset(&adapter);
            test_video_port_write(machine, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u);
            for (lib_size index = 0u; index < sizeof(crtc) / sizeof(crtc[0]); ++index) {
                test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, crtc[index][0]);
                test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, crtc[index][1]);
            }
            /* Reset begins at tick 8; advance to this field's next origin. */
            core_machine_vadp_advance(&adapter, 32u);
            test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 17u);
            test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 7u);
            test_video_port_write(machine, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x17u);
            for (lib_size index = 0u; index < sizeof(samples) / sizeof(samples[0]); ++index) {
                x86_video_bus_observation before = {0}, after = {0};

                core_machine_vadp_advance(&adapter, samples[index].phase - phase);
                phase = samples[index].phase;
                test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, 0x41u);
                failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 9u) !=
                    samples[index].status ||
                    core_machine_vadp_observe_bus(&adapter, &before) != LIB_STATUS_OK ||
                    before.vertical_interrupt != (phase >= 20u);
                test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, 0xc1u);
                failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 9u) !=
                    (samples[index].status | 1u) ||
                    core_machine_vadp_observe_bus(&adapter, &after) != LIB_STATUS_OK ||
                    after.vertical_interrupt != before.vertical_interrupt;
                test_video_port_write(machine, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, 0x41u);
                failed |= (test_video_port_read(machine, CORE_MACHINE_VADP_PORT_STATUS) & 9u) !=
                    samples[index].status;
            }
        }
        core_machine_vadp_finalize(&adapter);
        core_machine_destroy(machine);
    }
    return failed ? 1 : 0;
}
