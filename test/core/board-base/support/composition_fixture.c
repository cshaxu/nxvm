#include "lib/types/types_interface.h"
#include "composition_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "../pic_fixture.h"
#include "../../board-at/support/command_fixture.h"

lib_bool test_board_plan_timing_matches(const core_machine_plan *plan,
    const core_machine_config *expected,
    const core_machine_controller_timing_rules *controller_timing)
{
    const core_machine_config *config = &plan->configuration;
    return config->ticks_per_instruction == expected->ticks_per_instruction &&
        lib_memory_compare(&config->instruction_timing, &expected->instruction_timing,
            sizeof(config->instruction_timing)) == 0 &&
        lib_memory_compare(&config->transaction_contract, &expected->transaction_contract,
            sizeof(config->transaction_contract)) == 0 &&
        lib_memory_compare(&config->clock_plan, &expected->clock_plan,
            sizeof(config->clock_plan)) == 0 &&
        lib_memory_compare(&config->time_axis, &expected->time_axis,
            sizeof(config->time_axis)) == 0 &&
        config->kbc_typematic_initial_ticks == expected->kbc_typematic_initial_ticks &&
        config->kbc_typematic_repeat_ticks == expected->kbc_typematic_repeat_ticks &&
        config->kbc_command_response_ticks == expected->kbc_command_response_ticks &&
        config->kbc_command_response_status_polls == expected->kbc_command_response_status_polls &&
        lib_memory_compare(&plan->controller_timing, controller_timing,
            sizeof(plan->controller_timing)) == 0;
}

lib_bool test_board_instances_are_distinct(const core_machine_board_state *first,
    const core_machine_board_state *second)
{
    return first->shared_rtc != second->shared_rtc &&
        first->fdc != second->fdc && first->hdc != second->hdc;
}

test_board_plan_observation test_board_capture_plan(const core_machine_plan *plan)
{
    return (test_board_plan_observation) {
        .memory_bytes = plan->configuration.memory_bytes,
        .cpu_profile = plan->configuration.cpu_profile,
        .fpu_profile = plan->configuration.fpu_profile,
        .retirement_time_contract = plan->configuration.retirement_time_contract,
        .l1_compatibility_policy = plan->configuration.l1_compatibility_policy,
        .cpu_80386_cr_mov_ignores_mod = plan->configuration.cpu_80386_cr_mov_ignores_mod,
        .time_axis_kind = plan->configuration.time_axis.kind,
        .controller_timing = plan->controller_timing
    };
}

test_board_composition_observation test_board_capture_composition(
    const core_machine_board_state *board)
{
    return (test_board_composition_observation) {
        .fdc = board->fdc_topology.config,
        .hdc = board->hdc_topology.config,
        .rtc_irq = board->rtc_cmos_config.irq,
        .rtc_provenance = board->rtc_cmos_config.timing.provenance,
        .drive_cylinders = board->fdc_topology.drives.cylinder_count[0u],
        .dma_configured = board->dma_configured,
        .dma_wiring = board->dma_wiring,
        .dma_clock = {board->dma_clock.numerator, board->dma_clock.denominator, board->dma_clock.reset_phase},
        .pit_clock = {board->pit_clock.numerator, board->pit_clock.denominator, board->pit_clock.reset_phase},
        .auxiliary_pit_clock = {board->auxiliary_pit_clock.numerator, board->auxiliary_pit_clock.denominator,
            board->auxiliary_pit_clock.reset_phase},
        .rtc_clock = {board->rtc_clock.numerator, board->rtc_clock.denominator, board->rtc_clock.reset_phase},
        .vadp_clock = {board->vadp_clock.numerator, board->vadp_clock.denominator, board->vadp_clock.reset_phase},
        .rtc_ticks_per_second = board->rtc_cmos_config.ticks_per_second,
        .controller_timing = board->controller_timing,
        .kbc_typematic_initial_ticks = board->kbc_typematic_initial_ticks,
        .kbc_typematic_repeat_ticks = board->kbc_typematic_repeat_ticks,
        .kbc_command_response_ticks = board->kbc_command_response_ticks,
        .drives = board->fdc_topology.drives,
        .auxiliary_pit_configured = board->auxiliary_pit_configured,
        .fdc_configured = board->fdc_configured,
        .hdc_configured = board->hdc_configured,
        .rtc_cmos_configured = board->rtc_cmos_configured
    };
}

lib_status test_board_dma_duplicate_bind(core_machine_board_state *board, lib_u8 channel,
    const core_machine_dma_channel_provider *provider, core_machine_dma_request_binding *request)
{
    return core_machine_dma_bind_channel(board->shared_dma, channel,
        provider, request, request);
}

lib_status test_board_dma_bind_channel(core_machine_board_state *board, lib_u8 channel,
    const core_machine_dma_channel_provider *provider, void *context,
    core_machine_dma_request_binding *request)
{
    return core_machine_dma_bind_channel(board->shared_dma, channel,
        provider, context, request);
}

lib_bool test_board_dma_has_pending_request(const core_machine_board_state *board)
{
    return core_machine_dma_has_pending_request(board->shared_dma);
}

void test_board_dma_request_assert(core_machine_board_state *board,
    const core_machine_dma_request_binding *request)
{
    core_machine_dma_request_assert(board->shared_dma, request);
}

void test_board_pit_advance(core_machine_board_state *board, lib_u64 ticks)
{
    x86_pit_advance(board->shared_pit, ticks);
}

lib_bool test_board_pic_source_matches(core_machine_board_state *board,
    test_board_pic_source source, lib_u8 irq)
{
    core_machine_pic_irq_source *line = source == TEST_BOARD_PIT_IRQ0 ?
        board->shared_pit_irq0_source : (source == TEST_BOARD_KEYBOARD_IRQ1 ?
        board->keyboard_irq1_source : board->keyboard_irq12_source);
    return test_pic_source_route(board->shared_pic_master,
        board->shared_pic_slave, line, irq);
}

lib_bool test_board_kbc_command_matches(core_machine_board_state *board,
    core_machine *machine, lib_u8 command, lib_u8 mask, lib_u8 expected)
{
    return kbc_test_command_matches(board->shared_kbc, machine, command, mask, expected);
}

lib_i32 test_board_kbc_read_reply(core_machine_board_state *board, core_machine *machine)
{
    return kbc_test_read_reply(board->shared_kbc, machine);
}

lib_status test_board_kbc_submit_native_byte(core_machine_board_state *board, lib_u8 byte)
{
    return core_machine_kbc_submit_native_byte(board->shared_kbc, byte);
}

lib_status test_board_kbc_ticks_until_event(const core_machine_board_state *board,
    lib_u64 *ticks)
{
    return core_machine_kbc_ticks_until_event(board->shared_kbc, ticks);
}

void test_board_kbc_advance(core_machine_board_state *board, lib_u64 ticks)
{
    core_machine_kbc_advance(board->shared_kbc, ticks);
}

lib_i32 test_board_pic_unmask_delay_matches(core_machine_board_state *board,
    lib_u64 expected_ticks)
{
    core_machine_pic_bus *bus = board->shared_pic_master;
    core_machine_pic_irq_source *source = LIB_NULL;
    lib_u64 ticks = 0u;
    lib_i32 failed;
    if (core_machine_pic_write_register(bus, 0u, 0x13u) != LIB_STATUS_OK ||
        core_machine_pic_write_register(bus, 1u, 8u) != LIB_STATUS_OK ||
        core_machine_pic_write_register(bus, 1u, 1u) != LIB_STATUS_OK ||
        core_machine_pic_write_register(bus, 1u, 2u) != LIB_STATUS_OK ||
        core_machine_pic_irq_source_bind(&source, bus,
            board->shared_pic_slave, 1u) != LIB_STATUS_OK) return 1;
    core_machine_pic_irq_source_assert(source);
    core_machine_pic_irq_source_deassert(source);
    if (core_machine_pic_write_register(bus, 1u, 0u) != LIB_STATUS_OK) return 1;
    failed = core_machine_pic_ticks_until_event(bus,
        board->shared_pic_slave, &ticks) != LIB_STATUS_OK ||
        ticks != expected_ticks ? 0x0012 : 0;
    return failed;
}
