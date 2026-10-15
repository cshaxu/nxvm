#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "composition/time_fixture.h"

static void core_machine_dma_rtc_initialize_pic(core_machine *machine)
{
    test_pic_port_write(machine, 0x0020u, 0x11u);
    test_pic_port_write(machine, 0x0021u, 0x08u);
    test_pic_port_write(machine, 0x0021u, 0x04u);
    test_pic_port_write(machine, 0x0021u, 0x01u);
    test_pic_port_write(machine, 0x00a0u, 0x11u);
    test_pic_port_write(machine, 0x00a1u, 0x70u);
    test_pic_port_write(machine, 0x00a1u, 0x02u);
    test_pic_port_write(machine, 0x00a1u, 0x01u);
}

static void core_machine_dma_rtc_cmos_write(core_machine *machine,
    lib_u8 index, lib_u8 value)
{
    test_pic_port_write(machine, 0x0070u, index);
    test_pic_port_write(machine, 0x0071u, value);
}

static lib_u8 core_machine_dma_rtc_cmos_read(core_machine *machine,
    lib_u8 index)
{
    test_pic_port_write(machine, 0x0070u, index);
    return (lib_u8)test_pic_port_read(machine, 0x0071u);
}

static lib_i32 core_machine_at_refresh_leaves_dma0_available(void)
{
    const core_machine_planar_parity_config port_b = {
        .port = 0x0061u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1
    };
    const core_machine_dma_wiring wiring = {
        .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL
    };
    const core_machine_dma_channel_provider provider = {0};
    core_machine_dma_request_binding fdc_request = {0};
    core_machine_dma_request_binding spare_request = {0};
    core_machine_config configuration = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_time_observation time = {0};
    x86_core_cpu_state state = {0};
    lib_u8 address = 0xa5u;
    const lib_u8 nop = 0x90u;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x0200u}
    };
    lib_i32 failed;
    lib_i32 stage = 1;

    configuration.transaction_contract.refresh_cycle_source_ticks = 8u;
    failed = core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_planar_parity(board, &port_b) != LIB_STATUS_OK ||
        core_machine_configure_dma(board, &wiring, &fdc_request) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(board->shared_dma, 0u, &provider,
            &spare_request, &spare_request) != LIB_STATUS_OK ||
        (stage = 2, test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u)) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        !core_machine_board_refresh_request(board, &address) || address != 0u ||
        test_core_machine_advance_time(machine, 8u) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0200u, &nop, 1u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0043u, 0x74u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0041u, 32u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0041u, 0u) != LIB_STATUS_OK ||
        (stage = 3, test_core_machine_advance_time(machine, 32u)) != LIB_STATUS_OK ||
        x86_pit_get_output(board->shared_pit, 1u) ||
        core_machine_board_refresh_request(board, &address) ||
        (stage = 4, test_core_machine_advance_time(machine, 1u)) != LIB_STATUS_OK ||
        !core_machine_board_refresh_request(board, &address) || address != 1u ||
        core_machine_dma_has_pending_request(board->shared_dma) ||
        core_machine_bus_write(machine, 0x0043u, 0x70u) != LIB_STATUS_OK ||
        !core_machine_board_refresh_request(board, &address) ||
        (stage = 5, core_machine_run(machine, (core_machine_run_budget){0u, 7u}, &result)) !=
            LIB_STATUS_OK || result.executed != 0u || result.ticks != 7u ||
        !core_machine_board_refresh_request(board, &address) ||
        core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK || state.eip != 0x0200u ||
        (stage = 6, core_machine_capture_time_observation(machine, &time)) != LIB_STATUS_OK ||
        !time.next_deadline_valid || time.next_deadline_tick != time.elapsed_ticks + 1u ||
        (stage = 7, core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result)) !=
            LIB_STATUS_OK || result.executed != 1u || result.ticks != 4u ||
        core_machine_board_refresh_request(board, &address) || board->refresh_row != 2u ||
        (stage = 8, core_machine_reset(machine)) != LIB_STATUS_OK || board->refresh_row != 0u ||
        !core_machine_board_refresh_request(board, &address) || address != 0u ||
        (stage = 9, core_machine_run(machine, (core_machine_run_budget){0u, 7u}, &result)) !=
            LIB_STATUS_OK || result.executed != 0u || result.ticks != 7u ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        (stage = 10, test_core_machine_advance_time(machine, 1u)) != LIB_STATUS_OK ||
        board->refresh_row != 0u || !core_machine_board_refresh_request(board, &address) ||
        test_core_machine_advance_time(machine, 7u) != LIB_STATUS_OK ||
        board->refresh_row != 1u || core_machine_board_refresh_request(board, &address);
    if (failed) lib_c_printf("AT-REFRESH:LATCH-DURATION:FAIL:%d ticks=%u executed=%u row=%u pending=%u deadline=%u/%u\n",
        stage, (lib_u32)result.ticks, (lib_u32)result.executed,
        board != LIB_NULL ? board->refresh_row : 0u,
        board != LIB_NULL ? board->refresh_hold_pending : 0u,
        (lib_u32)time.next_deadline_tick, time.next_deadline_valid);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_dma_refresh_follows_pit_channel_1(void)
{
    const lib_u8 program[] = {0xdbu, 0xe3u, 0xdbu, 0xe3u,
        0xdbu, 0xe3u, 0xdbu, 0xe3u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x0200u}
    };
    core_machine_config configuration = {0};
    core_machine_run_result result;
    core_machine_dma_wiring wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_dma_request_binding fdc_request = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    failed |= core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_dma(board, &wiring, &fdc_request) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00fffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0200u, program,
            sizeof(program)) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0043u, 0x74u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0041u, 2u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0041u, 0u) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){2u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 2u || result.ticks != 2u ||
        x86_pit_get_output(board->shared_pit, 1u) ||
        (test_pic_port_read(machine, 8u) & 0x10u) == 0u ||
        core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 1u || result.ticks != 1u ||
        !x86_pit_get_output(board->shared_pit, 1u) ||
        (test_pic_port_read(machine, 8u) & 0x10u) != 0u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 core_machine_dma_wiring_matches_constructed_controllers(void)
{
    lib_i32 failed = 0;
    for (lib_u8 count = 1u; count <= 2u; ++count) {
        const core_machine_config configuration = {.dma_controller_count = count};
        core_machine_dma_wiring wiring = {
            .fdc_channel = CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND,
            .controller_count = count == 1u ? 2u : 1u,
            .cascade_channel = count == 1u ? CORE_MACHINE_DMA_CASCADE_CHANNEL : 0u
        };
        core_machine_dma_request_binding request = {77u, 7u};
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        if (core_machine_create(&configuration, &machine, &board) != LIB_STATUS_OK) {
            failed = 1;
        } else {
            failed |= core_machine_configure_dma(board, &wiring, &request) !=
                LIB_STATUS_INVALID_ARGUMENT || board->dma_configured ||
                request.core_token != 77u || request.channel != 7u ||
                board->dma_wiring.controller_count != count;
            wiring.controller_count = count;
            wiring.cascade_channel = count == 2u ? CORE_MACHINE_DMA_CASCADE_CHANNEL : 0u;
            failed |= core_machine_configure_dma(board, &wiring, &request) !=
                LIB_STATUS_OK || !board->dma_configured || request.core_token != 0u;
        }
        core_machine_destroy(machine);
    }
    return failed;
}

static lib_i32 core_machine_rtc_clock_deadline_route(lib_u32 source_rate)
{
    const core_machine_config configuration = {
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, source_rate},
        .clock_plan.rtc = {32768u, source_rate, 0u}
    };
    const core_machine_rtc_cmos_config rtc = {
        .index_port = 0x70u, .data_port = 0x71u, .irq = 8u,
        .nmi_mask_bit = 0x80u, .ticks_per_second = 32768u,
        .timing = {8u, 65u, CORE_MACHINE_RTC_TIMING_L3_SOURCE}
    };
    const core_machine_plan_topology topology = {
        .rtc_cmos_present = LIB_TRUE, .rtc_cmos = rtc
    };
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_time_observation time = {0};
    lib_i32 failed = core_machine_plan_create(&configuration, &plan) != LIB_STATUS_OK;

    failed |= !failed && core_machine_plan_set_topology(plan, &topology) != LIB_STATUS_OK;
    failed |= !failed && core_machine_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) goto done;
    core_machine_dma_rtc_initialize_pic(machine);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_REG_A, 0x20u);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_SECOND_ALARM, 1u);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_MINUTE_ALARM, 0xc0u);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_HOUR_ALARM, 0xc0u);
    core_machine_dma_rtc_cmos_write(machine, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_AIE);
    failed |= core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK;
    failed |= !time.next_deadline_valid || time.next_deadline_tick != source_rate;
    failed |= test_core_machine_advance_time(machine, source_rate - 1u) != LIB_STATUS_OK;
    failed |= core_machine_dma_rtc_cmos_read(machine, X86_RTC_SECOND) != 0u ||
        core_machine_pic_irq_source_is_asserted(board->rtc_irq_source);
    failed |= core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK;
    failed |= !time.next_deadline_valid || time.next_deadline_tick != time.elapsed_ticks + 1u;
    failed |= test_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
    core_machine_pic_refresh(board->shared_pic_master, board->shared_pic_slave);
    failed |= core_machine_dma_rtc_cmos_read(machine, X86_RTC_SECOND) != 1u ||
        !core_machine_pic_irq_source_is_asserted(board->rtc_irq_source) ||
        core_machine_pic_get_interrupt(board->shared_pic_master, board->shared_pic_slave) != 0x70u;
    failed |= core_machine_dma_rtc_cmos_read(machine, X86_RTC_REG_C) !=
        (X86_RTC_REG_C_AF | X86_RTC_REG_C_UF | X86_RTC_REG_C_IRQF) ||
        core_machine_pic_irq_source_is_asserted(board->rtc_irq_source);
    failed |= core_machine_bus_write(machine, 0xa0u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK;
    core_machine_pic_refresh(board->shared_pic_master, board->shared_pic_slave);
    failed |= core_machine_pic_scan_interrupt(board->shared_pic_master, board->shared_pic_slave);
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_dma_rtc_cmos_read(machine, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au ||
        core_machine_dma_rtc_cmos_read(machine, X86_RTC_SECOND) != 1u ||
        core_machine_dma_rtc_cmos_read(machine, X86_RTC_REG_C) != 0u;
    core_machine_dma_rtc_initialize_pic(machine);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_REG_A, 0x26u);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_PIE);
    failed |= core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK;
    {
        const lib_u32 period = (source_rate + 1023u) / 1024u;
        failed |= !time.next_deadline_valid ||
            time.next_deadline_tick != time.elapsed_ticks + period;
        failed |= test_core_machine_advance_time(machine, period - 1u) != LIB_STATUS_OK ||
            core_machine_pic_irq_source_is_asserted(board->rtc_irq_source);
        failed |= test_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
        core_machine_pic_refresh(board->shared_pic_master, board->shared_pic_slave);
        failed |= !core_machine_pic_irq_source_is_asserted(board->rtc_irq_source) ||
            core_machine_pic_get_interrupt(board->shared_pic_master, board->shared_pic_slave) != 0x70u;
        failed |= core_machine_dma_rtc_cmos_read(machine, X86_RTC_REG_C) !=
            (X86_RTC_REG_C_PF | X86_RTC_REG_C_IRQF) ||
            core_machine_pic_irq_source_is_asserted(board->rtc_irq_source);
    }
done:
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    if (failed) lib_c_printf("RTC-CLOCK-DEADLINE:FAIL\n");
    return failed;
}

int main(void)
{
    core_machine_config machine_config = {0};
    core_machine_dma_wiring dma_wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_dma_wiring invalid_wiring = dma_wiring;
    core_machine_rtc_cmos_config rtc_config = {0};
    core_machine_dma_request_binding fdc_request = {0};
    core_machine_run_budget budget = {3u, 0u};
    const core_machine_dma_channel_provider duplicate_provider = {0};
    core_machine_dma_request_binding duplicate_request = {0};
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const lib_u8 program[] = { 0x90u, 0xf4u };
    lib_u8 interrupt_vector = 0u;
    lib_i32 nmi_masked = 0;
    lib_i32 interrupt_pending = 0;
    lib_i32 failed = 0;
    lib_i32 stage = 1;

    failed |= core_machine_at_refresh_leaves_dma0_available();
    failed |= core_machine_dma_refresh_follows_pit_channel_1();
    failed |= core_machine_dma_wiring_matches_constructed_controllers();
    failed |= core_machine_rtc_clock_deadline_route(1000000u);
    failed |= core_machine_rtc_clock_deadline_route(8000000u);
    failed |= core_machine_rtc_clock_deadline_route(16000000u);
    failed |= core_machine_configure_dma(LIB_NULL, &dma_wiring, &fdc_request) !=
        LIB_STATUS_INVALID_STATE ||
        core_machine_get_fdc_dma_request_binding(LIB_NULL, &fdc_request) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_configure_rtc_cmos(LIB_NULL, &rtc_config) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_configure_fdc(LIB_NULL, LIB_NULL) != LIB_STATUS_INVALID_STATE ||
        core_machine_configure_hdc(LIB_NULL, LIB_NULL) != LIB_STATUS_INVALID_STATE;
    machine_config.ticks_per_instruction = 1u;
    machine_config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
    rtc_config.index_port = 0x0070u;
    rtc_config.data_port = 0x0071u;
    rtc_config.irq = 8u;
    rtc_config.nmi_mask_bit = 0x80u;
    rtc_config.ticks_per_second = 1u;
    rtc_config.defaults[0].index = CORE_MACHINE_RTC_EQUIPMENT;
    rtc_config.defaults[0].value = 0x5au;
    rtc_config.default_count = 1u;

    invalid_wiring.controller_count = 1u;
    if (core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_dma(board, &invalid_wiring, &fdc_request) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        (invalid_wiring = dma_wiring, invalid_wiring.fdc_channel = 0u,
            core_machine_configure_dma(board, &invalid_wiring, &fdc_request)) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_configure_dma(board, &dma_wiring, &fdc_request) !=
            LIB_STATUS_OK ||
        core_machine_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_OK ||
        fdc_request.core_token == 0u || fdc_request.channel != 2u ||
        core_machine_configure_dma(board, &dma_wiring, &fdc_request) !=
            LIB_STATUS_INVALID_STATE ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00fffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(board->shared_dma, 2u,
            &duplicate_provider, &duplicate_request, &duplicate_request) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_dma_bind_channel(board->shared_dma, 0u,
            &duplicate_provider, &duplicate_request, &duplicate_request) !=
            LIB_STATUS_INVALID_STATE ||
        board->refresh_dma_request.core_token == 0u ||
        board->refresh_dma_request.channel != 0u ||
        core_machine_dma_has_pending_request(board->shared_dma) ||
        core_machine_dma_rtc_cmos_read(machine, CORE_MACHINE_RTC_EQUIPMENT) !=
            0x5au) {
        failed = 1;
        goto done;
    }

    stage = 2;
    test_pic_port_write(machine, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(machine, &nmi_masked) != LIB_STATUS_OK ||
        !nmi_masked) {
        failed = 1;
        goto done;
    }
    test_pic_port_write(machine, 0x0070u, 0u);
    if (core_machine_get_nmi_mask(machine, &nmi_masked) != LIB_STATUS_OK ||
        nmi_masked) {
        failed = 1;
        goto done;
    }

    stage = 3;
    core_machine_dma_rtc_initialize_pic(machine);
    core_machine_dma_rtc_cmos_write(machine, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    if (core_machine_memory_write(machine, 0x00fffff0u, program, sizeof(program)) !=
            LIB_STATUS_OK || core_machine_run(machine, budget, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        result.executed != 2u) {
        failed = 1;
        stage = 3;
    } else if (core_machine_dma_rtc_cmos_read(machine,
            X86_RTC_SECOND) != 0x05u) {
        failed = 1;
        stage = 4;
    } else if (!(interrupt_pending = core_machine_pic_scan_interrupt(
            board->shared_pic_master, board->shared_pic_slave)) ||
        (interrupt_vector = core_machine_pic_get_interrupt(
            board->shared_pic_master, board->shared_pic_slave)) != 0x70u) {
        failed = 1;
        stage = 5;
    } else if (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_dma_rtc_cmos_read(machine, CORE_MACHINE_RTC_EQUIPMENT) !=
        0x5au || core_machine_dma_rtc_cmos_read(machine,
            X86_RTC_SECOND) != 0x05u) {
        failed = 1;
        stage = 6;
    }

done:
    if (failed && machine != LIB_NULL) {
        lib_c_printf("DMA-RTC-AUTHORITY:DETAIL:%d IRQ=%u asserted=%u pending=%d vector=%02x IRR=%02x/%02x\n",
            stage,
            board->rtc_cmos_config.irq,
            core_machine_pic_irq_source_is_asserted(board->rtc_irq_source),
            interrupt_pending, interrupt_vector,
            test_pic_read(board->shared_pic_master, 0x0au), test_pic_read(board->shared_pic_slave, 0x0au));
    }
    core_machine_destroy(machine);
    if (failed) lib_c_printf("DMA-RTC-AUTHORITY:FAIL:%d\n", stage);
    if (!failed) lib_c_printf("DMA-RTC-AUTHORITY:OK\n");
    if (!failed) lib_c_printf("%s\n", "BOARD-CONTROLLER-HANDLES:OK");
    return failed;
}
