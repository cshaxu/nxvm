#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core_machine_board_fixture.h"

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

static lib_i32 core_machine_dma_refresh_follows_pit_channel_1(void)
{
    const lib_u8 program[] = {0xdbu, 0xe3u, 0xdbu, 0xe3u,
        0xdbu, 0xe3u, 0xdbu, 0xe3u};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0200u}
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
        core_machine_run(machine, (core_machine_run_budget){3u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 3u || result.ticks != 3u ||
        x86_pit_get_output(board->shared_pit, 1u) ||
        (test_pic_port_read(machine, 8u) & 0x10u) == 0u ||
        core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) !=
            LIB_STATUS_OK || result.executed != 1u || result.ticks != 1u ||
        !x86_pit_get_output(board->shared_pit, 1u) ||
        (test_pic_port_read(machine, 8u) & 0x10u) != 0u;
    core_machine_destroy(machine);
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

    failed |= core_machine_dma_refresh_follows_pit_channel_1();
    failed |= core_machine_configure_dma(LIB_NULL, &dma_wiring, &fdc_request) !=
        LIB_STATUS_INVALID_STATE ||
        core_machine_get_fdc_dma_request_binding(LIB_NULL, &fdc_request) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_configure_rtc_cmos(LIB_NULL, &rtc_config) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_configure_fdc(LIB_NULL, LIB_NULL) != LIB_STATUS_INVALID_STATE ||
        core_machine_configure_hdc(LIB_NULL, LIB_NULL) != LIB_STATUS_INVALID_STATE;
    machine_config.ticks_per_instruction = 1u;
    machine_config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
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
