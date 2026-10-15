#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/x86/memory_interface.h"
#include "core/board-base/vadp.h"
#include "core/board-base/machine_board_state.h"

static lib_bool display_write_crtc(core_machine *machine, lib_u8 index, lib_u8 value)
{
    return core_machine_bus_write(machine, CORE_MACHINE_VADP_PORT_CRTC_INDEX, index) ==
        LIB_STATUS_OK && core_machine_bus_write(machine,
            CORE_MACHINE_VADP_PORT_CRTC_DATA, value) == LIB_STATUS_OK;
}

static lib_bool display_cga_clock_routing(void)
{
    const core_machine_config machine_config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .clock_plan = {.vadp = {315u, 1408u, 0u}}
    };
    const core_machine_display_config display_config = {
        .text_timing = {48u, 8u, 8u}, .cga_vram_present = LIB_TRUE,
        .ports = {.crtc_first = CORE_MACHINE_VADP_PORT_CRTC_INDEX,
            .crtc_last = CORE_MACHINE_VADP_PORT_STATUS}
    };
    static const lib_u8 modes[] = {0x0du, 0x0cu, 0x0au, 0x1au};
    static const lib_u8 clocks[] = {1u, 2u, 2u, 2u};
    static const lib_u8 registers[][2] = {
        {0u, 4u}, {1u, 3u}, {4u, 2u}, {5u, 1u},
        {6u, 2u}, {7u, 2u}, {9u, 3u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u64 ticks = 0u;
    lib_u32 status = LIB_UINT32_MAX;
    lib_size mode = 0u;
    lib_u8 stage = 1u;
    lib_bool failed = LIB_TRUE;

    if (core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_display(board, &display_config) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    for (; mode < sizeof(modes); ++mode) {
        const lib_u32 scale = clocks[mode];

        stage = 2u;
        for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index)
            if (!display_write_crtc(machine, registers[index][0], registers[index][1]))
                goto done;
        if (core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, CORE_MACHINE_VADP_PORT_MODE,
                modes[mode]) != LIB_STATUS_OK) goto done;
        /* Use the selected AT conversion, not a direct chip advance. */
        stage = 3u;
        if (core_machine_clock_domain_source_ticks_until(&board->vadp_clock,
                65u * scale, &ticks) != LIB_STATUS_OK) goto done;
        core_machine_board_after_internal_advance(board, ticks, 0u);
        stage = 4u;
        if (core_machine_bus_read(machine, CORE_MACHINE_VADP_PORT_STATUS, &status) !=
                LIB_STATUS_OK || status != 0u ||
            core_machine_clock_domain_source_ticks_until(&board->vadp_clock,
                3u * scale, &ticks) != LIB_STATUS_OK || ticks == 0u) goto done;
        core_machine_board_after_internal_advance(board, ticks - 1u, 0u);
        stage = 5u;
        if (core_machine_bus_read(machine, CORE_MACHINE_VADP_PORT_STATUS, &status) !=
                LIB_STATUS_OK || status != 0u) goto done;
        core_machine_board_after_internal_advance(board, 1u, 0u);
        stage = 6u;
        if (core_machine_bus_read(machine, CORE_MACHINE_VADP_PORT_STATUS, &status) !=
                LIB_STATUS_OK || status != 1u) goto done;
    }
    failed = LIB_FALSE;
done:
    if (failed) lib_c_fprintf(lib_c_stderr, "CGA clock mode=%u stage=%u status=%u ticks=%u\n",
        (lib_u32)mode, stage, status, (lib_u32)ticks);
    core_machine_destroy(machine);
    return !failed;
}

static lib_bool display_blink_without_irq(core_machine_display_config config)
{
    const core_machine_config machine_config = {
        .memory_bytes = (16u * 1024u * 1024u),
        .clock_plan = {.vadp = {1u, 2u, 0u}}
    };
    static const lib_u8 registers[][2] = {
        {0u, 6u}, {1u, 0u}, {6u, 6u}, {7u, 0x10u}, {9u, 1u},
        {10u, 31u}, {11u, 0u}, {0x11u, 0u}, {0x12u, 1u},
        {0x13u, 1u}, {0x15u, 3u}, {0x17u, 0xe3u}, {0x18u, 0xffu}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    x86_video_snapshot *snapshot = lib_allocate_zero(1u, sizeof(*snapshot));
    core_machine_attachment_deadline_observation deadline = {0};
    x86_video_bus_observation observation = {0};
    lib_u64 device_due = 0u;
    lib_u64 source_due = 0u;
    lib_u8 stage = 0u;
    lib_bool valid = snapshot != LIB_NULL;

    config.vertical_irq = 0u;
    config.ega_sequencer.memory_mode = 7u;
    config.ega_sequencer.map_mask = 4u;
    config.ega_controllers = (x86_video_ega_controller_config){
        .graphics = {[6] = 4u, [8] = 0xffu},
        .attribute = {[1] = 1u, [16] = 14u, [18] = 15u}
    };
    if (!valid || core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_display(board, &config) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        valid = LIB_FALSE;
        goto done;
    }
    valid &= board->shared_vadp->irq_source == LIB_NULL &&
        core_machine_bus_write(machine, 0x3c2u, 3u) == LIB_STATUS_OK;
    stage = 1u;
    for (lib_size index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index)
        valid &= display_write_crtc(machine, registers[index][0], registers[index][1]);
    valid &= core_machine_memory_write(machine, 0xa0820u, &(lib_u8){0x80u}, 1u) ==
            LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x3c4u, 2u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x3c5u, 1u) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0xa0000u, &(lib_u8){'A'}, 1u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x3c5u, 2u) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0xa0000u, &(lib_u8){0x92u}, 1u) == LIB_STATUS_OK &&
        core_machine_capture_display_snapshot(board, snapshot) == LIB_STATUS_OK &&
        snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixels[0] == 2u;
    if (!valid) goto done;
    {
        lib_u8 byte = 0u;

        valid &= display_write_crtc(machine, 0x17u, 0xf3u) &&
            core_machine_capture_display_snapshot(board, snapshot) == LIB_STATUS_OK &&
            snapshot->kind == X86_VIDEO_KIND_TEXT_PIXELS && snapshot->pixels[0] == 0u &&
            core_machine_memory_write(machine, 0xa0100u, &(lib_u8){0x5au}, 1u) == LIB_STATUS_OK &&
            core_machine_bus_write(machine, 0x3ceu, 4u) == LIB_STATUS_OK &&
            core_machine_bus_write(machine, 0x3cfu, 1u) == LIB_STATUS_OK &&
            core_machine_memory_read(machine, 0xa0100u, &byte, 1u) == LIB_STATUS_OK &&
            byte == 0x5au && display_write_crtc(machine, 0x17u, 0xe3u) &&
            core_machine_capture_display_snapshot(board, snapshot) == LIB_STATUS_OK &&
            snapshot->pixels[0] == 2u;
        if (!valid) goto done;
    }
    stage = 2u;
    valid &= x86_video_ticks_until_event(board->shared_vadp->chip, &device_due) == LIB_STATUS_OK &&
        core_machine_clock_domain_source_ticks_until(&board->vadp_clock,
            device_due, &source_due) == LIB_STATUS_OK && source_due > 0u && source_due <= 2048u;
    board->timing_declarations_copied = LIB_TRUE;
    core_machine_board_deadline_observe(board, 0u, &deadline);
    valid &= deadline.source_ticks == source_due && !deadline.immediate_due;
    if (!valid) goto done;
    stage = 3u;
    /* Port setup itself advances guest time; use the real clock remainder. */
    core_machine_board_after_internal_advance(board, source_due - 1u, 0u);
    valid &= core_machine_capture_display_snapshot(board, snapshot) == LIB_STATUS_OK &&
        !snapshot->buffer_changed && snapshot->pixels[0] == 2u;
    core_machine_board_deadline_observe(board, source_due - 1u, &deadline);
    valid &= deadline.source_ticks == 1u && !deadline.immediate_due;
    if (!valid) goto done;
    stage = 4u;
    core_machine_board_after_internal_advance(board, 1u, 0u);
    valid &= core_machine_capture_display_snapshot(board, snapshot) == LIB_STATUS_OK &&
        snapshot->buffer_changed && snapshot->pixels[0] == 1u &&
        x86_video_observe_bus(board->shared_vadp->chip, &observation) == LIB_STATUS_OK &&
        !observation.vertical_interrupt;
done:
    if (!valid) lib_c_fprintf(lib_c_stderr,
        "DISPLAY:BLINK-NO-IRQ:FAIL stage=%u due=%u kind=%u pixel=%u irq=%u\n",
        stage, (lib_u32)deadline.source_ticks, snapshot == LIB_NULL ? 0u : snapshot->kind,
        snapshot == LIB_NULL ? 0u : snapshot->pixels[0], observation.vertical_interrupt);
    lib_release(snapshot);
    core_machine_destroy(machine);
    return valid;
}

int main(void)
{
    core_machine_config machine_config = {0};
    core_machine_display_config display_config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u8 pixel = 0x5au;
    lib_i32 failed = !display_cga_clock_routing();

    machine_config.memory_bytes = (16u * 1024u * 1024u);
    machine_config.clock_plan.vadp = (core_machine_clock_ratio){1u, 2u, 0u};
    display_config.text_timing.active_display_ticks = 48u;
    display_config.text_timing.horizontal_blank_ticks = 8u;
    display_config.text_timing.vertical_retrace_ticks = 8u;
    display_config.ega_present = LIB_TRUE;
    display_config.vertical_irq = 9u;
    display_config.ega_sequencer.aperture_base = CORE_MACHINE_VADP_EGA_APERTURE_BASE;
    display_config.ega_sequencer.aperture_bytes = CORE_MACHINE_VADP_EGA_APERTURE_BYTES;
    display_config.ega_sequencer.reset = 0x03u;
    display_config.ega_sequencer.clocking_mode = 0x01u;
    display_config.ega_sequencer.map_mask = 0x0fu;
    display_config.ega_sequencer.memory_mode = 0x02u;
    display_config.ega_sequencer.planar_ega = LIB_TRUE;
    display_config.ports.attribute_first = CORE_MACHINE_VADP_PORT_ATTRIBUTE;
    display_config.ports.attribute_last = CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ;
    display_config.ports.sequencer_first = CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX;
    display_config.ports.sequencer_last = CORE_MACHINE_VADP_PORT_SEQUENCER_DATA;
    display_config.ports.graphics_first = CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX;
    display_config.ports.graphics_last = CORE_MACHINE_VADP_PORT_GRAPHICS_DATA;
    display_config.ports.crtc_first = CORE_MACHINE_VADP_PORT_CRTC_INDEX;
    display_config.ports.crtc_last = CORE_MACHINE_VADP_PORT_STATUS;

    if (core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK) return 1;
    {
        core_machine_display_config invalid = display_config;
        core_machine_pic_irq_source *source;

        invalid.text_timing.active_display_ticks = 0u;
        failed |= core_machine_configure_display(board, &invalid) != LIB_STATUS_INVALID_ARGUMENT ||
            board->display_configured || board->shared_vadp->configured;
        source = board->shared_vadp->irq_source;
        failed |= source == LIB_NULL || core_machine_pic_irq_source_is_asserted(source);
        failed |= core_machine_configure_display(board, &invalid) != LIB_STATUS_INVALID_ARGUMENT ||
            board->shared_vadp->irq_source != source ||
            core_machine_pic_irq_source_is_asserted(source);
    }
    if (core_machine_configure_display(LIB_NULL, &display_config) !=
            LIB_STATUS_INVALID_STATE ||
        !core_machine_configuration_is_open(machine) ||
        core_machine_configuration_is_open(LIB_NULL) ||
        core_machine_configure_display(board, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_configure_display(board, &display_config) != LIB_STATUS_OK ||
        core_machine_configure_display(board, &display_config) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_configuration_is_open(machine) ||
        core_machine_configure_display(board, &display_config) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
            &pixel, sizeof(pixel)) != LIB_STATUS_OK) {
        failed = 1;
    }
    if (!failed) {
        core_machine_attachment_deadline_observation deadline;
        x86_pic_register_state pic;
        lib_u64 due = 0u;
        lib_u32 status = 0u;

        failed |= core_machine_bus_write(machine,
            CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u) != LIB_STATUS_OK;
        failed |= !display_write_crtc(machine, 0u, 6u) ||
            !display_write_crtc(machine, 1u, 3u) || !display_write_crtc(machine, 6u, 8u) ||
            !display_write_crtc(machine, 7u, 0u) || !display_write_crtc(machine, 0x12u, 3u) ||
            !display_write_crtc(machine, 0x15u, 5u) || !display_write_crtc(machine, 0x11u, 0x19u);
        failed |= x86_video_ticks_until_event(board->shared_vadp->chip, &due) != LIB_STATUS_OK ||
            due != 32u || board->shared_vadp->irq_source == LIB_NULL;
        board->timing_declarations_copied = LIB_TRUE;
        core_machine_board_deadline_observe(board, 0u, &deadline);
        failed |= deadline.source_ticks != 64u || deadline.immediate_due;
        core_machine_board_after_internal_advance(board, 63u, 0u);
        failed |= core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        core_machine_board_after_internal_advance(board, 1u, 0u);
        failed |= !core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source) ||
            core_machine_pic_capture_registers(board->shared_pic_slave, &pic) != LIB_STATUS_OK ||
            (pic.irr & 2u) == 0u ||
            core_machine_bus_read(machine, CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0, &status) !=
                LIB_STATUS_OK || (status & 0x80u) != 0u;
        failed |= !display_write_crtc(machine, 0x11u, 9u) ||
            core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        failed |= !display_write_crtc(machine, 0x11u, 0x19u);
        core_machine_board_after_internal_advance(board, 160u, 0u);
        failed |= !core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        for (lib_u8 divided = 0u; divided < 2u; ++divided) {
            const lib_u64 source_due = divided ? 164u : 74u;

            failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
                core_machine_bus_write(machine,
                    CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u) != LIB_STATUS_OK;
            failed |= core_machine_bus_write(machine, 0x3c4u, 0u) != LIB_STATUS_OK ||
                core_machine_bus_write(machine, 0x3c5u, 2u) != LIB_STATUS_OK ||
                core_machine_bus_write(machine, 0x3c4u, 1u) != LIB_STATUS_OK ||
                core_machine_bus_write(machine, 0x3c5u, divided ? 8u : 0u) != LIB_STATUS_OK ||
                core_machine_bus_write(machine, 0x3c4u, 0u) != LIB_STATUS_OK ||
                core_machine_bus_write(machine, 0x3c5u, 3u) != LIB_STATUS_OK ||
                !display_write_crtc(machine, 0u, 6u) || !display_write_crtc(machine, 6u, 8u) ||
                !display_write_crtc(machine, 7u, 0u) || !display_write_crtc(machine, 0x15u, 5u) ||
                !display_write_crtc(machine, 0x11u, 0x19u);
            board->timing_declarations_copied = LIB_TRUE;
            core_machine_board_deadline_observe(board, 0u, &deadline);
            failed |= deadline.source_ticks != source_due || deadline.immediate_due;
            core_machine_board_after_internal_advance(board, source_due - 1u, 0u);
            failed |= core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
            core_machine_board_after_internal_advance(board, 1u, 0u);
            failed |= !core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        }
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        failed |= core_machine_bus_write(machine,
            CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u) != LIB_STATUS_OK;
        failed |= !display_write_crtc(machine, 0u, 6u) ||
            !display_write_crtc(machine, 6u, 8u) || !display_write_crtc(machine, 7u, 0u) ||
            !display_write_crtc(machine, 0x15u, 5u) ||
            !display_write_crtc(machine, 0x17u, 0xe7u) ||
            !display_write_crtc(machine, 0x11u, 0x19u);
        board->timing_declarations_copied = LIB_TRUE;
        core_machine_board_deadline_observe(board, 0u, &deadline);
        failed |= deadline.source_ticks != 144u || deadline.immediate_due;
        core_machine_board_after_internal_advance(board, 143u, 0u);
        failed |= core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        core_machine_board_after_internal_advance(board, 1u, 0u);
        failed |= !core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        /* Teardown with an asserted borrowed lease must finish before PIC dies. */
        failed |= core_machine_bus_write(machine,
            CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, 1u) != LIB_STATUS_OK;
        failed |= !display_write_crtc(machine, 0u, 6u) ||
            !display_write_crtc(machine, 6u, 8u) || !display_write_crtc(machine, 0x15u, 5u) ||
            !display_write_crtc(machine, 0x11u, 0x19u);
        core_machine_board_after_internal_advance(board, 64u, 0u);
        failed |= !core_machine_pic_irq_source_is_asserted(board->shared_vadp->irq_source);
    }
    core_machine_destroy(machine);
    failed |= !display_blink_without_irq(display_config);
    if (!failed) {
        lib_c_printf("DISPLAY-AUTHORITY:OK\n");
        lib_c_printf("BOARD-DISPLAY-HANDLE:OK\n");
    }
    return failed;
}
