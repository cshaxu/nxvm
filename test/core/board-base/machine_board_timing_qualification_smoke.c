#include "lib/types/types_interface.h"
#include "core/board-at/kbc_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "lib/types/file.h"

typedef struct board_timing_reentry_probe {
    core_machine_board_state *board;
    const core_machine_board_timing_declaration *declarations;
} board_timing_reentry_probe;

static lib_status board_timing_reentry(void *opaque,
    core_machine_firmware_context *firmware)
{
    const board_timing_reentry_probe *probe = opaque;
    (void)firmware;
    return core_machine_board_install_timing_declarations(probe->board,
        probe->declarations, CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) ==
            LIB_STATUS_INVALID_STATE ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

static lib_i32 board_timing_declarations(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_BOARD_MINIMUM_MEMORY_BYTES,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    core_machine_board_timing_declaration declarations[CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT];
    core_machine_board_timing_declaration observed;
    const core_machine_board_timing_declaration invalid_entries[] = {
        {CORE_MACHINE_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {(core_machine_board_timing_capability)-1, CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {(core_machine_board_timing_capability)CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT, CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXEC, (core_machine_board_timing_disposition)-1, CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXEC, (core_machine_board_timing_disposition)(CORE_MACHINE_BOARD_TIMING_DISPOSITION_L3_REQUIRED + 1), CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXEC, CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, (core_machine_board_timing_seam)-1},
        {CORE_MACHINE_BOARD_TIMING_CAPABILITY_CPU_EXEC, CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, (core_machine_board_timing_seam)(CORE_MACHINE_BOARD_TIMING_SEAM_OBSERVATION + 1)}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const core_machine_firmware_provider firmware = {
        board_timing_reentry, board_timing_reentry, LIB_NULL
    };
    board_timing_reentry_probe reentry = {LIB_NULL, declarations};
    lib_i32 failed = 1;
    lib_size index;

    /* Table representation is tested independently of the plan's source
     * policy. Reverse order proves publication indexes capability, not input. */
    for (index = 0u; index < CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT; ++index)
        declarations[index] = (core_machine_board_timing_declaration){
            (core_machine_board_timing_capability)(CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT - 1u - index),
            CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_BOARD_TIMING_SEAM_CPU_PROGRAM};
    if (core_machine_board_validate_timing_declarations(LIB_NULL,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_board_validate_timing_declarations(declarations, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_board_validate_timing_declarations(declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT - 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_board_validate_timing_declarations(declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT + 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_board_install_timing_declarations(LIB_NULL, declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK)
        goto done;
    if (core_machine_board_install_timing_declarations(board, LIB_NULL,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT) goto done;
    for (index = 0u; index < sizeof(invalid_entries) / sizeof(invalid_entries[0]); ++index) {
        core_machine_board_timing_declaration saved = declarations[CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT - 1u];
        core_machine_board_timing_declaration *invalid = &declarations[CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT - 1u];

        *invalid = invalid_entries[index];
        if (core_machine_board_install_timing_declarations(board, declarations,
                CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
            board->timing_declarations_copied) goto done;
        for (lib_size entry = 0u; entry < CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT; ++entry)
            if (board->timing_declarations[entry].capability != 0) goto done;
        *invalid = saved;
    }
    reentry.board = board;
    if (core_machine_bind_firmware_provider(machine, &firmware, &reentry) !=
            LIB_STATUS_OK) goto done;
    if (core_machine_board_install_timing_declarations(board, declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_OK ||
        core_machine_board_install_timing_declarations(board, declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    declarations[0].disposition = CORE_MACHINE_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    if (core_machine_board_get_timing_declaration(board,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_board_get_timing_declaration(board,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK ||
        core_machine_board_install_timing_declarations(board, declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    core_machine_destroy(machine);
    machine = LIB_NULL;
    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_board_install_timing_declarations(board, declarations,
            CORE_MACHINE_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        board->timing_declarations_copied) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 board_default_and_memory_policy(void)
{
    const core_machine_config config = {0};
    core_machine_executor_config executor = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_size bytes = 0u;
    x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
    lib_i32 failed = 1;

    if (core_machine_board_prepare_executor(&config, &executor) != LIB_STATUS_OK ||
        executor.memory_bytes != CORE_MACHINE_BOARD_DEFAULT_MEMORY_BYTES ||
        executor.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_get_memory_bytes(machine, &bytes) != LIB_STATUS_OK ||
        bytes != CORE_MACHINE_BOARD_DEFAULT_MEMORY_BYTES ||
        core_machine_get_cpu_profile(machine, &profile) != LIB_STATUS_OK ||
        profile != X86_CORE_CPU_PROFILE_80386 ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_reconfigure_memory(machine,
            CORE_MACHINE_BOARD_MINIMUM_MEMORY_BYTES - 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_reconfigure_memory(machine,
            CORE_MACHINE_BOARD_MAXIMUM_MEMORY_BYTES + 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_get_memory_bytes(machine, &bytes) != LIB_STATUS_OK ||
        bytes != CORE_MACHINE_BOARD_DEFAULT_MEMORY_BYTES) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 scheduler_board_timing_qualification(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_BOARD_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_attachment_deadline_observation observation;
    lib_u64 pit_ticks;
    lib_u64 source_ticks;
    lib_i32 failed = 1;

    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x43u, 0x34u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, 5u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK ||
        x86_pit_ticks_until_output(board->shared_pit, 0u,
            &pit_ticks) != LIB_STATUS_OK ||
        core_machine_clock_domain_source_ticks_until(&board->pit_clock,
            pit_ticks, &source_ticks) != LIB_STATUS_OK) goto done;
    /* The board owns its qualification state; Core supplies no PC marker. */
    core_machine_board_deadline_observe(board, 0u, &observation);
    if (observation.source_ticks != 0u || observation.immediate_due) goto done;
    board->timing_declarations_copied = LIB_TRUE;
    core_machine_board_deadline_observe(board, 0u, &observation);
    if (observation.source_ticks != source_ticks || observation.immediate_due) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 scheduler_kbc_deadline_routing(void)
{
    static const lib_u8 native_bytes[] = {0x1cu, 0x32u};
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_BOARD_MINIMUM_MEMORY_BYTES,
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .time_axis = {CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_attachment_deadline_observation board_observation;
    core_machine_time_observation observation;
    lib_u64 device_ticks;
    lib_u64 source_ticks;
    lib_u32 port_value;
    lib_u8 advanced;
    lib_i32 failed = 1;

    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    core_machine_kbc_set_serial_delivery_timing(board->shared_kbc, 3u);
    if (
        core_machine_kbc_submit_native_bytes(board->shared_kbc, native_bytes,
            sizeof(native_bytes)) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0064u, 0x20u) != LIB_STATUS_OK ||
        core_machine_kbc_ticks_until_event(board->shared_kbc, &device_ticks) !=
            LIB_STATUS_OK || device_ticks != 3u ||
        core_machine_clock_domain_source_ticks_until(&board->kbc_clock,
            device_ticks, &source_ticks) != LIB_STATUS_OK) goto done;
    core_machine_board_deadline_observe(board, 0u,
        &board_observation);
    if (board_observation.immediate_due ||
        board_observation.source_ticks != source_ticks ||
        core_machine_capture_time_observation(machine, &observation) !=
            LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != observation.elapsed_ticks + source_ticks ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || core_machine_bus_read(machine, 0x0060u, &port_value) !=
            LIB_STATUS_OK || port_value != 0x1eu) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 scheduler_pic_relative_deadline(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_BOARD_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_attachment_deadline_observation observation;
    core_machine_pic_irq_timing timing = {0};
    lib_u64 ticks = 0u;
    lib_i32 failed = 1;
    lib_u32 stage = 1u;
    timing.unmask_delivery_ticks[1] = 5u;
    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_pic_irq_source_bind(&source, board->shared_pic_master,
            board->shared_pic_slave, 1u) != LIB_STATUS_OK) goto done;
    core_machine_pic_set_irq_timing(board->shared_pic_master, board->shared_pic_slave, &timing);
    stage = 2u;
    if (core_machine_pic_write_register(board->shared_pic_master, 0u, 0x11u) !=
            LIB_STATUS_OK ||
        core_machine_pic_write_register(board->shared_pic_master, 1u, 8u) !=
            LIB_STATUS_OK ||
        core_machine_pic_write_register(board->shared_pic_master, 1u, 4u) !=
            LIB_STATUS_OK ||
        core_machine_pic_write_register(board->shared_pic_master, 1u, 1u) !=
            LIB_STATUS_OK ||
        core_machine_pic_write_register(board->shared_pic_master, 1u, 0xffu) !=
        LIB_STATUS_OK) goto done;
    core_machine_pic_irq_source_assert(source);
    stage = 3u;
    if (core_machine_pic_write_register(board->shared_pic_master, 1u, 0xfdu) !=
            LIB_STATUS_OK ||
        core_machine_pic_ticks_until_event(board->shared_pic_master,
            board->shared_pic_slave, &ticks) != LIB_STATUS_OK || ticks != 5u) goto done;
    core_machine_board_deadline_observe(board, LIB_UINT64_MAX - 2u, &observation);
    stage = 4u;
    if (observation.immediate_due || observation.source_ticks != 5u) goto done;
    core_machine_pic_advance(board->shared_pic_master, board->shared_pic_slave, 4u);
    core_machine_board_deadline_observe(board, LIB_UINT64_MAX - 1u, &observation);
    stage = 5u;
    if (observation.immediate_due || observation.source_ticks != 1u) goto done;
    failed = 0;
done:
    if (failed) lib_c_fprintf(lib_c_stderr, "Board PIC deadline stage=%u ticks=%u\n",
        stage, (lib_u32)ticks);
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    return board_timing_declarations() || board_default_and_memory_policy() || scheduler_board_timing_qualification() || scheduler_kbc_deadline_routing() ||
        scheduler_pic_relative_deadline();
}
