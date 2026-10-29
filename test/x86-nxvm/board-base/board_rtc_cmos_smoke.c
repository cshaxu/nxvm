#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"
#include "board_board_fixture.h"

typedef struct rtc_cmos_fixture {
    x86_core_machine *machine;
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    x86_rtc *rtc;
    x86_nxvm_board_pic_irq_source *irq_source;
} rtc_cmos_fixture;

static lib_status rtc_cmos_initialize_pic(x86_core_machine *port)
{
    const struct { lib_u16 port; lib_u8 value; } program[] = {
        {0x0020u, 0x11u}, {0x0021u, 0x08u}, {0x0021u, 0x04u}, {0x0021u, 0x01u},
        {0x00a0u, 0x11u}, {0x00a1u, 0x70u}, {0x00a1u, 0x02u}, {0x00a1u, 0x01u}
    };
    for (lib_size i = 0u; i < sizeof(program) / sizeof(program[0]); ++i) {
        lib_status status = x86_core_machine_bus_write(port, program[i].port,
            program[i].value);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static void rtc_cmos_output(void *context, lib_bool asserted)
{
    x86_nxvm_board_pic_irq_source *source = context;
    if (asserted) x86_nxvm_board_pic_irq_source_assert(source);
    else x86_nxvm_board_pic_irq_source_deassert(source);
}

static void rtc_cmos_finalize(rtc_cmos_fixture *fixture)
{
    x86_rtc_destroy(fixture->rtc);
    x86_nxvm_board_pic_finalize(fixture->master, fixture->slave);
    x86_core_machine_destroy(fixture->machine);
}

static lib_status rtc_cmos_initialize(rtc_cmos_fixture *fixture)
{
    const x86_core_machine_executor_config executor_config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    x86_rtc_config config = {4u, 0u, 0u, LIB_FALSE};
    lib_status status;

    lib_memory_set(fixture, 0, sizeof(*fixture));
    status = x86_core_machine_neutral_create(&executor_config, &fixture->machine);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_pic_initialize(&fixture->master, &fixture->slave,
            fixture->machine, X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_freeze_execution_providers(fixture->machine);
    if (status == LIB_STATUS_OK) status = x86_core_machine_reset(fixture->machine);
    if (status == LIB_STATUS_OK)
        status = rtc_cmos_initialize_pic(fixture->machine);
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_board_pic_irq_source_bind(&fixture->irq_source,
            fixture->master, fixture->slave, 8u);
    }
    if (status == LIB_STATUS_OK)
        status = x86_rtc_create(&config, rtc_cmos_output,
            fixture->irq_source, &fixture->rtc);
    if (status != LIB_STATUS_OK) rtc_cmos_finalize(fixture);
    return status;
}

static lib_i32 rtc_cmos_test_events_and_irq8(void)
{
    rtc_cmos_fixture fixture;
    lib_u8 flags;
    lib_i32 failed = 0;

    if (rtc_cmos_initialize(&fixture) != LIB_STATUS_OK) return 1;
    x86_rtc_write_register(fixture.rtc, X86_RTC_SECOND_ALARM, 0xc0u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_MINUTE_ALARM, 0xc0u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_HOUR_ALARM, 0xc0u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H);
    x86_rtc_advance(fixture.rtc, 4u);
    flags = x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C);
    failed |= (flags & (X86_RTC_REG_C_PF | X86_RTC_REG_C_AF |
        X86_RTC_REG_C_UF)) != (X86_RTC_REG_C_PF |
        X86_RTC_REG_C_AF | X86_RTC_REG_C_UF) ||
        (flags & X86_RTC_REG_C_IRQF) != 0u || x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_PIE |
        X86_RTC_REG_B_AIE | X86_RTC_REG_B_UIE);
    x86_nxvm_board_pic_refresh(fixture.master, fixture.slave);
    failed |= !x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source) ||
        x86_nxvm_board_pic_get_interrupt(fixture.master, fixture.slave) != 0x70u;
    flags = x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C);
    failed |= (flags & (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF |
        X86_RTC_REG_C_AF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF |
        X86_RTC_REG_C_AF | X86_RTC_REG_C_UF) ||
        x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source);
    failed |= x86_core_machine_bus_write(fixture.machine, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_write(fixture.machine, 0x0020u, 0x20u) != LIB_STATUS_OK;
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H);
    flags = x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C);
    failed |= (flags & (X86_RTC_REG_C_PF | X86_RTC_REG_C_AF |
        X86_RTC_REG_C_UF)) == 0u || (flags & X86_RTC_REG_C_IRQF) != 0u ||
        x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    failed |= !x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_destroy(fixture.rtc);
    fixture.rtc = LIB_NULL;
    failed |= x86_nxvm_board_pic_irq_source_is_asserted(fixture.irq_source);
    rtc_cmos_finalize(&fixture);
    return failed;
}

static lib_i32 rtc_cmos_test_cmos_adapter(lib_bool extended)
{
    x86_nxvm_board_config config = { 0 };
    x86_nxvm_board_rtc_cmos_config rtc_config = { 0 };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_u8 cmos[X86_RTC_REGISTER_COUNT - (X86_RTC_REG_D + 1u)] = {0};
    lib_u8 rtc[X86_RTC_REG_D + 1u] = {0};
    const lib_size cmos_count = (extended ? X86_RTC_REGISTER_COUNT :
        X86_RTC_DEFAULT_REGISTER_COUNT) - (X86_RTC_REG_D + 1u);
    lib_i32 masked = 0;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 1u;
    rtc_config.index_port = 0x0070u;
    rtc_config.data_port = 0x0071u;
    rtc_config.irq = 8u;
    rtc_config.nmi_mask_bit = 0x80u;
    rtc_config.ticks_per_second = 4u;
    rtc_config.defaults[0].index = X86_NXVM_BOARD_RTC_EQUIPMENT;
    rtc_config.defaults[0].value = 0x5au;
    rtc_config.default_count = 1u;
    rtc_config.timing.provenance = X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE;
    if (x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_INVALID_ARGUMENT) {
        failed = 1;
    }
    rtc_config.timing = (x86_nxvm_board_rtc_timing_plan) {1u, 1u,
        X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE};
    rtc_config.extended_registers = 2u;
    if (!failed && x86_nxvm_board_configure_rtc_cmos(board, &rtc_config) !=
            LIB_STATUS_INVALID_ARGUMENT) failed = 1;
    rtc_config.extended_registers = extended;
    if (extended) {
        rtc_config.nmi_mask_bit = 0x40u;
        if (!failed && x86_nxvm_board_configure_rtc_cmos(board, &rtc_config) !=
                LIB_STATUS_INVALID_ARGUMENT) failed = 1;
        rtc_config.nmi_mask_bit = 0x80u;
    }
    rtc_config.defaults[1] = (x86_nxvm_board_rtc_default_byte) {0x7fu, 0xa5u};
    rtc_config.default_count = 2u;
    if (!extended) {
        if (!failed && x86_nxvm_board_configure_rtc_cmos(board, &rtc_config) !=
                LIB_STATUS_INVALID_ARGUMENT) failed = 1;
        rtc_config.default_count = 1u;
    }
    if (failed ||
        x86_nxvm_board_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_OK ||
        test_x86_nxvm_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0070u, 0x94u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x0071u, &value) != LIB_STATUS_OK ||
        value != 0x5au || x86_core_machine_get_nmi_mask(machine, &masked) !=
            LIB_STATUS_OK || !masked ||
        x86_core_machine_bus_write(machine, 0x0070u, X86_NXVM_BOARD_RTC_EQUIPMENT) !=
            LIB_STATUS_OK || x86_core_machine_get_nmi_mask(machine, &masked) !=
            LIB_STATUS_OK || masked) {
        failed = 1;
    }
    if (!failed && (x86_core_machine_bus_write(machine, 0x70u, 0x3bu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x71u, 0x55u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x70u, 0x7bu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x71u, 0xaau) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x70u, 0x3bu) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x71u, &value) != LIB_STATUS_OK ||
        value != (extended ? 0x55u : 0xaau) ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x70u, 0xfbu) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x71u, &value) != LIB_STATUS_OK ||
        value != 0xaau || x86_core_machine_get_nmi_mask(machine, &masked) !=
            LIB_STATUS_OK || !masked)) failed = 1;
    if (!failed && extended && (x86_core_machine_bus_write(machine, 0x70u, 0x7fu) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(machine, 0x71u, &value) !=
        LIB_STATUS_OK || value != 0xa5u)) failed = 1;
    if (!failed && (x86_nxvm_board_capture_cmos(board, cmos, cmos_count - 1u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_capture_cmos(board, cmos, cmos_count) != LIB_STATUS_OK ||
        cmos[0x3bu - (X86_RTC_REG_D + 1u)] != (extended ? 0x55u : 0xaau) ||
        (extended && cmos[0x7fu - (X86_RTC_REG_D + 1u)] != 0xa5u))) failed = 1;
    if (!failed) {
        const lib_size offset = 0x3bu - (X86_RTC_REG_D + 1u);
        const lib_u8 original = cmos[offset];
        cmos[offset] = 0x33u;
        if (x86_nxvm_board_restore_cmos(board, cmos, cmos_count - 1u) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            x86_core_machine_bus_write(machine, 0x70u, 0x3bu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x71u, &value) != LIB_STATUS_OK ||
            value != original ||
            x86_nxvm_board_restore_cmos(board, cmos, cmos_count) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x71u, &value) != LIB_STATUS_OK ||
            value != 0x33u) failed = 1;
        cmos[offset] = original;
        if (x86_nxvm_board_restore_cmos(board, cmos, cmos_count) != LIB_STATUS_OK)
            failed = 1;
    }
    if (!failed) {
        rtc[X86_RTC_REG_B] = X86_RTC_REG_B_DM; /* Binary, 12-hour mode. */
        rtc[X86_RTC_HOUR] = 0x83u;
        rtc[X86_RTC_DAY_WEEK] = 2u;
        rtc[X86_RTC_DAY_MONTH] = 28u;
        rtc[X86_RTC_MONTH] = 10u;
        rtc[X86_RTC_YEAR] = 24u;
        rtc[X86_RTC_REG_A] = 0x26u;
        if (x86_nxvm_board_restore_rtc(board, rtc, sizeof(rtc) - 1u) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            x86_nxvm_board_restore_rtc(board, rtc, sizeof(rtc)) != LIB_STATUS_OK ||
            x86_nxvm_board_capture_rtc(board, rtc, sizeof(rtc)) != LIB_STATUS_OK ||
            rtc[X86_RTC_HOUR] != 0x83u || rtc[X86_RTC_YEAR] != 24u ||
            rtc[X86_RTC_REG_B] != X86_RTC_REG_B_DM ||
            rtc[X86_RTC_REG_C] != 0u || rtc[X86_RTC_REG_D] != X86_RTC_REG_D_VRT)
            failed = 1;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= rtc_cmos_test_events_and_irq8();
    failed |= rtc_cmos_test_cmos_adapter(LIB_FALSE);
    failed |= rtc_cmos_test_cmos_adapter(LIB_TRUE);
    if (failed != 0) return 1;
    lib_c_printf("RTC-CMOS:OK\n");
    return 0;
}
