#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "x86/core/machine_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"
#include "core_machine_board_fixture.h"

typedef struct rtc_cmos_s3_fixture {
    core_machine *machine;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
    x86_rtc *rtc;
    core_machine_pic_irq_source *irq_source;
} rtc_cmos_s3_fixture;

static lib_status rtc_cmos_s3_initialize_pic(core_machine *port)
{
    const struct { lib_u16 port; lib_u8 value; } program[] = {
        {0x0020u, 0x11u}, {0x0021u, 0x08u}, {0x0021u, 0x04u}, {0x0021u, 0x01u},
        {0x00a0u, 0x11u}, {0x00a1u, 0x70u}, {0x00a1u, 0x02u}, {0x00a1u, 0x01u}
    };
    for (lib_size i = 0u; i < sizeof(program) / sizeof(program[0]); ++i) {
        lib_status status = core_machine_bus_write(port, program[i].port,
            program[i].value);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static void rtc_cmos_s3_output(void *context, lib_bool asserted)
{
    core_machine_pic_irq_source *source = context;
    if (asserted) core_machine_pic_irq_source_assert(source);
    else core_machine_pic_irq_source_deassert(source);
}

static void rtc_cmos_s3_finalize(rtc_cmos_s3_fixture *fixture)
{
    x86_rtc_destroy(fixture->rtc);
    core_machine_pic_finalize(fixture->master, fixture->slave);
    core_machine_destroy(fixture->machine);
}

static lib_status rtc_cmos_s3_initialize(rtc_cmos_s3_fixture *fixture)
{
    const core_machine_executor_config executor_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    x86_rtc_config config = {4u, 0u, 0u};
    lib_status status;

    lib_memory_set(fixture, 0, sizeof(*fixture));
    status = core_machine_neutral_create(&executor_config, &fixture->machine);
    if (status == LIB_STATUS_OK)
        status = core_machine_pic_initialize(&fixture->master, &fixture->slave,
            fixture->machine, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    if (status == LIB_STATUS_OK)
        status = core_machine_freeze_execution_providers(fixture->machine);
    if (status == LIB_STATUS_OK) status = core_machine_reset(fixture->machine);
    if (status == LIB_STATUS_OK)
        status = rtc_cmos_s3_initialize_pic(fixture->machine);
    if (status == LIB_STATUS_OK) {
        status = core_machine_pic_irq_source_bind(&fixture->irq_source,
            fixture->master, fixture->slave, 8u);
    }
    if (status == LIB_STATUS_OK)
        status = x86_rtc_create(&config, rtc_cmos_s3_output,
            fixture->irq_source, &fixture->rtc);
    if (status != LIB_STATUS_OK) rtc_cmos_s3_finalize(fixture);
    return status;
}

static lib_i32 rtc_cmos_s3_test_events_and_irq8(void)
{
    rtc_cmos_s3_fixture fixture;
    lib_u8 flags;
    lib_i32 failed = 0;

    if (rtc_cmos_s3_initialize(&fixture) != LIB_STATUS_OK) return 1;
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
        (flags & X86_RTC_REG_C_IRQF) != 0u || core_machine_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_PIE |
        X86_RTC_REG_B_AIE | X86_RTC_REG_B_UIE);
    core_machine_pic_refresh(fixture.master, fixture.slave);
    failed |= !core_machine_pic_irq_source_is_asserted(fixture.irq_source) ||
        core_machine_pic_get_interrupt(fixture.master, fixture.slave) != 0x70u;
    flags = x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C);
    failed |= (flags & (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF |
        X86_RTC_REG_C_AF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF |
        X86_RTC_REG_C_AF | X86_RTC_REG_C_UF) ||
        core_machine_pic_irq_source_is_asserted(fixture.irq_source);
    failed |= core_machine_bus_write(fixture.machine, 0x00a0u, 0x20u) != LIB_STATUS_OK;
    failed |= core_machine_bus_write(fixture.machine, 0x0020u, 0x20u) != LIB_STATUS_OK;
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H);
    flags = x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C);
    failed |= (flags & (X86_RTC_REG_C_PF | X86_RTC_REG_C_AF |
        X86_RTC_REG_C_UF)) == 0u || (flags & X86_RTC_REG_C_IRQF) != 0u ||
        core_machine_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_advance(fixture.rtc, 4u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    failed |= !core_machine_pic_irq_source_is_asserted(fixture.irq_source);
    x86_rtc_destroy(fixture.rtc);
    fixture.rtc = LIB_NULL;
    failed |= core_machine_pic_irq_source_is_asserted(fixture.irq_source);
    rtc_cmos_s3_finalize(&fixture);
    return failed;
}

static lib_i32 rtc_cmos_s3_test_cmos_adapter(void)
{
    core_machine_config config = { 0 };
    core_machine_rtc_cmos_config rtc_config = { 0 };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 masked = 0;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 1u;
    rtc_config.index_port = 0x0070u;
    rtc_config.data_port = 0x0071u;
    rtc_config.irq = 8u;
    rtc_config.nmi_mask_bit = 0x80u;
    rtc_config.ticks_per_second = 4u;
    rtc_config.defaults[0].index = CORE_MACHINE_RTC_EQUIPMENT;
    rtc_config.defaults[0].value = 0x5au;
    rtc_config.default_count = 1u;
    rtc_config.timing.provenance = CORE_MACHINE_RTC_TIMING_L3_SOURCE;
    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_INVALID_ARGUMENT) {
        failed = 1;
    }
    rtc_config.timing = (core_machine_rtc_timing_plan) {1u, 1u,
        CORE_MACHINE_RTC_TIMING_L3_SOURCE};
    if (failed ||
        core_machine_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0070u, 0x94u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0071u, &value) != LIB_STATUS_OK ||
        value != 0x5au || core_machine_get_nmi_mask(machine, &masked) !=
            LIB_STATUS_OK || !masked ||
        core_machine_bus_write(machine, 0x0070u, CORE_MACHINE_RTC_EQUIPMENT) !=
            LIB_STATUS_OK || core_machine_get_nmi_mask(machine, &masked) !=
            LIB_STATUS_OK || masked) {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= rtc_cmos_s3_test_events_and_irq8();
    failed |= rtc_cmos_s3_test_cmos_adapter();
    if (failed != 0) return 1;
    printf("M5:T350:S3:RTC-CMOS:OK\n");
    return 0;
}
