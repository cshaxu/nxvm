#include "lib/types/types_interface.h"
#include "core/board-at/kbc_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "lib/types/file.h"

static lib_i32 scheduler_board_timing_qualification(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
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
    /* Provider input, not private Core marker, authorizes the board clock. */
    core_machine_board_deadline_observe(board, 0u, LIB_FALSE, &observation);
    if (observation.source_ticks != 0u || observation.immediate_due) goto done;
    core_machine_board_deadline_observe(board, 0u, LIB_TRUE, &observation);
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
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
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
    core_machine_board_deadline_observe(board, 0u, LIB_TRUE,
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
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
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
    core_machine_board_deadline_observe(board, LIB_UINT64_MAX - 2u, LIB_FALSE, &observation);
    stage = 4u;
    if (observation.immediate_due || observation.source_ticks != 5u) goto done;
    core_machine_pic_advance(board->shared_pic_master, board->shared_pic_slave, 4u);
    core_machine_board_deadline_observe(board, LIB_UINT64_MAX - 1u, LIB_FALSE, &observation);
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
    return scheduler_board_timing_qualification() || scheduler_kbc_deadline_routing() ||
        scheduler_pic_relative_deadline();
}
