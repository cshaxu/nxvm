#include "lib/types/types_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "board_board_fixture.h"
#include "lib/types/file.h"

typedef struct board_timing_reentry_probe {
    x86_nxvm_board_state *board;
    const x86_nxvm_board_timing_declaration *declarations;
} board_timing_reentry_probe;

static lib_status board_timing_reentry(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    const board_timing_reentry_probe *probe = opaque;
    (void)firmware;
    return x86_nxvm_board_install_timing_declarations(probe->board,
        probe->declarations, X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) ==
            LIB_STATUS_INVALID_STATE ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}

static lib_i32 board_timing_declarations(void)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    x86_nxvm_board_timing_declaration declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT];
    x86_nxvm_board_timing_declaration observed;
    const x86_nxvm_board_timing_declaration invalid_entries[] = {
        {X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {(x86_nxvm_board_timing_capability)-1, X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {(x86_nxvm_board_timing_capability)X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT, X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC, (x86_nxvm_board_timing_disposition)-1, X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC, (x86_nxvm_board_timing_disposition)(X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED + 1), X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM},
        {X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC, X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, (x86_nxvm_board_timing_seam)-1},
        {X86_NXVM_BOARD_TIMING_CAPABILITY_CPU_EXEC, X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, (x86_nxvm_board_timing_seam)(X86_NXVM_BOARD_TIMING_SEAM_OBSERVATION + 1)}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    const x86_core_machine_firmware_provider firmware = {
        board_timing_reentry, board_timing_reentry, LIB_NULL
    };
    board_timing_reentry_probe reentry = {LIB_NULL, declarations};
    lib_i32 failed = 1;
    lib_size index;

    /* Table representation is tested independently of the plan's source
     * policy. Reverse order proves publication indexes capability, not input. */
    for (index = 0u; index < X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT; ++index)
        declarations[index] = (x86_nxvm_board_timing_declaration){
            (x86_nxvm_board_timing_capability)(X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT - 1u - index),
            X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK, X86_NXVM_BOARD_TIMING_SEAM_CPU_PROGRAM};
    if (x86_nxvm_board_validate_timing_declarations(LIB_NULL,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_validate_timing_declarations(declarations, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_validate_timing_declarations(declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT - 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_validate_timing_declarations(declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT + 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_install_timing_declarations(LIB_NULL, declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK)
        goto done;
    if (x86_nxvm_board_install_timing_declarations(board, LIB_NULL,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT) goto done;
    for (index = 0u; index < sizeof(invalid_entries) / sizeof(invalid_entries[0]); ++index) {
        x86_nxvm_board_timing_declaration saved = declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT - 1u];
        x86_nxvm_board_timing_declaration *invalid = &declarations[X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT - 1u];

        *invalid = invalid_entries[index];
        if (x86_nxvm_board_install_timing_declarations(board, declarations,
                X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
            board->timing_declarations_copied) goto done;
        for (lib_size entry = 0u; entry < X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT; ++entry)
            if (board->timing_declarations[entry].capability != 0) goto done;
        *invalid = saved;
    }
    reentry.board = board;
    if (x86_core_machine_bind_firmware_provider(machine, &firmware, &reentry) !=
            LIB_STATUS_OK) goto done;
    if (x86_nxvm_board_install_timing_declarations(board, declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_OK ||
        x86_nxvm_board_install_timing_declarations(board, declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    declarations[0].disposition = X86_NXVM_BOARD_TIMING_DISPOSITION_L3_REQUIRED;
    if (x86_nxvm_board_get_timing_declaration(board,
            X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_nxvm_board_get_timing_declaration(board,
            X86_NXVM_BOARD_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != X86_NXVM_BOARD_TIMING_DISPOSITION_L2_FALLBACK ||
        x86_nxvm_board_install_timing_declarations(board, declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    x86_core_machine_destroy(machine);
    machine = LIB_NULL;
    if (x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_nxvm_board_install_timing_declarations(board, declarations,
            X86_NXVM_BOARD_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        board->timing_declarations_copied) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 board_default_and_memory_policy(void)
{
    const x86_nxvm_board_config config = {0};
    x86_core_machine_executor_config executor = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_size bytes = 0u;
    x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
    lib_i32 failed = 1;

    if (x86_nxvm_board_prepare_executor(&config, &executor) != LIB_STATUS_OK ||
        executor.memory_bytes != X86_NXVM_BOARD_DEFAULT_MEMORY_BYTES ||
        executor.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_get_memory_bytes(machine, &bytes) != LIB_STATUS_OK ||
        bytes != X86_NXVM_BOARD_DEFAULT_MEMORY_BYTES ||
        x86_core_machine_get_cpu_profile(machine, &profile) != LIB_STATUS_OK ||
        profile != X86_CORE_CPU_PROFILE_80386 ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_reconfigure_memory(machine,
            X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES - 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_reconfigure_memory(machine,
            X86_NXVM_BOARD_MAXIMUM_MEMORY_BYTES + 1u) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_get_memory_bytes(machine, &bytes) != LIB_STATUS_OK ||
        bytes != X86_NXVM_BOARD_DEFAULT_MEMORY_BYTES) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 board_address_decode_policy(void)
{
    for (lib_u32 policy = 0u; policy < 2u; ++policy) {
        x86_nxvm_board_config config = {
            .memory_bytes = 4u * 1024u * 1024u,
            .cpu_profile = X86_CORE_CPU_PROFILE_80386,
            .a20_wrap_policy = (x86_nxvm_board_a20_wrap_policy)policy
        };
        x86_core_machine_executor_config executor;
        x86_core_machine *machine = LIB_NULL;
        x86_nxvm_board_state *board = LIB_NULL;
        const lib_u8 first = 0x11u, second = 0x22u, third = 0x33u;
        lib_u8 value = 0u;
        lib_bool enabled = LIB_TRUE;
        lib_i32 failed = 1;

        if (x86_nxvm_board_prepare_executor(&config, &executor) != LIB_STATUS_OK ||
            executor.address_decode.first != (policy ? 0x100000u : 0u) ||
            executor.address_decode.last != (policy ? 0x1fffffu : LIB_UINT32_MAX) ||
            executor.address_decode.reset_clear_mask != 0x100000u ||
            x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_nxvm_board_observe_a20(board, &enabled) != LIB_STATUS_OK || enabled)
            goto done;
        if (x86_core_machine_memory_write(machine, 0x200000u, &first, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x300000u, &second, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0x200000u, &value, 1u) != LIB_STATUS_OK ||
            value != (policy ? first : second) ||
            x86_core_machine_memory_write(machine, 0x100000u, &third, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0u, &value, 1u) != LIB_STATUS_OK ||
            value != third ||
            x86_core_machine_bus_write(machine, 0x92u, 2u) != LIB_STATUS_OK ||
            x86_nxvm_board_observe_a20(board, &enabled) != LIB_STATUS_OK || !enabled ||
            x86_core_machine_memory_write(machine, 0x100000u, &second, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, 0u, &value, 1u) != LIB_STATUS_OK ||
            value != third || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_nxvm_board_observe_a20(board, &enabled) != LIB_STATUS_OK || enabled)
            goto done;
        config.a20_wrap_policy = (x86_nxvm_board_a20_wrap_policy)2;
        if (x86_nxvm_board_prepare_executor(&config, &executor) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            executor.address_decode.first != (policy ? 0x100000u : 0u) ||
            executor.address_decode.last != (policy ? 0x1fffffu : LIB_UINT32_MAX) ||
            executor.address_decode.reset_clear_mask != 0x100000u ||
            x86_nxvm_board_observe_a20(LIB_NULL, &enabled) != LIB_STATUS_INVALID_ARGUMENT ||
            x86_nxvm_board_observe_a20(board, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT)
            goto done;
        failed = 0;
done:
        x86_core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static lib_i32 scheduler_board_timing_qualification(void)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_core_machine_attachment_deadline_observation observation;
    lib_u64 pit_ticks;
    lib_u64 source_ticks;
    lib_i32 failed = 1;

    if (x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x43u, 0x34u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 5u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK ||
        x86_pit_ticks_until_output(board->shared_pit, 0u,
            &pit_ticks) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_source_ticks_until(&board->pit_clock,
            pit_ticks, &source_ticks) != LIB_STATUS_OK) goto done;
    /* The board owns its qualification state; Core supplies no PC marker. */
    x86_nxvm_board_deadline_observe(board, 0u, &observation);
    if (observation.source_ticks != 0u || observation.immediate_due) goto done;
    board->timing_declarations_copied = LIB_TRUE;
    x86_nxvm_board_deadline_observe(board, 0u, &observation);
    if (observation.source_ticks != source_ticks || observation.immediate_due) goto done;
    /* Both channels share one clock; the earlier output must win. */
    x86_pit_set_gate(board->shared_pit, 1u, LIB_TRUE);
    if (x86_core_machine_bus_write(machine, 0x43u, 0x74u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x41u, 2u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x41u, 0u) != LIB_STATUS_OK) goto done;
    x86_pit_advance(board->shared_pit, 1u);
    {
        lib_u64 later_ticks;
        if (x86_pit_ticks_until_output(board->shared_pit, 0u, &later_ticks) != LIB_STATUS_OK ||
            x86_pit_ticks_until_output(board->shared_pit, 1u, &pit_ticks) != LIB_STATUS_OK ||
            pit_ticks >= later_ticks ||
            x86_core_machine_clock_domain_source_ticks_until(&board->pit_clock,
                pit_ticks, &source_ticks) != LIB_STATUS_OK) goto done;
    }
    x86_nxvm_board_deadline_observe(board, 0u, &observation);
    if (observation.source_ticks != source_ticks || observation.immediate_due) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 scheduler_kbc_deadline_routing(void)
{
    static const lib_u8 native_bytes[] = {0x1cu, 0x32u};
    const x86_nxvm_board_config config = {
        .memory_bytes = X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES,
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .time_axis = {X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_core_machine_attachment_deadline_observation board_observation;
    x86_core_machine_time_observation observation;
    lib_u64 device_ticks;
    lib_u64 source_ticks;
    lib_u32 port_value;
    lib_u8 advanced;
    lib_i32 failed = 1;

    if (x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    x86_nxvm_board_kbc_set_serial_delivery_timing(board->shared_kbc, 3u);
    if (
        x86_nxvm_board_kbc_submit_native_bytes(board->shared_kbc, native_bytes,
            sizeof(native_bytes)) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0064u, 0x20u) != LIB_STATUS_OK ||
        x86_nxvm_board_kbc_ticks_until_event(board->shared_kbc, &device_ticks) !=
            LIB_STATUS_OK || device_ticks != 3u ||
        x86_core_machine_clock_domain_source_ticks_until(&board->kbc_clock,
            device_ticks, &source_ticks) != LIB_STATUS_OK) goto done;
    x86_nxvm_board_deadline_observe(board, 0u,
        &board_observation);
    if (board_observation.immediate_due ||
        board_observation.source_ticks != source_ticks ||
        x86_core_machine_capture_time_observation(machine, &observation) !=
            LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != observation.elapsed_ticks + source_ticks ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
        !advanced || x86_core_machine_bus_read(machine, 0x0060u, &port_value) !=
            LIB_STATUS_OK || port_value != 0x1eu) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 scheduler_pic_relative_deadline(void)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES,
        .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    x86_core_machine_attachment_deadline_observation observation;
    x86_nxvm_board_pic_irq_timing timing = {0};
    lib_u64 ticks = 0u;
    lib_i32 failed = 1;
    lib_u32 stage = 1u;
    timing.unmask_delivery_ticks[1] = 5u;
    if (x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_bind(&source, board->shared_pic_master,
            board->shared_pic_slave, 1u) != LIB_STATUS_OK) goto done;
    x86_nxvm_board_pic_set_irq_timing(board->shared_pic_master, board->shared_pic_slave, &timing);
    stage = 2u;
    if (x86_nxvm_board_pic_write_register(board->shared_pic_master, 0u, 0x11u) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_pic_write_register(board->shared_pic_master, 1u, 8u) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_pic_write_register(board->shared_pic_master, 1u, 4u) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_pic_write_register(board->shared_pic_master, 1u, 1u) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_pic_write_register(board->shared_pic_master, 1u, 0xffu) !=
        LIB_STATUS_OK) goto done;
    x86_nxvm_board_pic_irq_source_assert(source);
    stage = 3u;
    if (x86_nxvm_board_pic_write_register(board->shared_pic_master, 1u, 0xfdu) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_pic_ticks_until_event(board->shared_pic_master,
            board->shared_pic_slave, &ticks) != LIB_STATUS_OK || ticks != 5u) goto done;
    x86_nxvm_board_deadline_observe(board, LIB_UINT64_MAX - 2u, &observation);
    stage = 4u;
    if (observation.immediate_due || observation.source_ticks != 5u) goto done;
    x86_nxvm_board_pic_advance(board->shared_pic_master, board->shared_pic_slave, 4u);
    x86_nxvm_board_deadline_observe(board, LIB_UINT64_MAX - 1u, &observation);
    stage = 5u;
    if (observation.immediate_due || observation.source_ticks != 1u) goto done;
    failed = 0;
done:
    if (failed) lib_c_fprintf(lib_c_stderr, "Board PIC deadline stage=%u ticks=%u\n",
        stage, (lib_u32)ticks);
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    return board_timing_declarations() || board_default_and_memory_policy() || board_address_decode_policy() || scheduler_board_timing_qualification() || scheduler_kbc_deadline_routing() ||
        scheduler_pic_relative_deadline();
}
