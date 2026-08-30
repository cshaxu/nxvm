#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "core/machine/rtc.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct rtc_cmos_s3_fixture {
    t_port port;
    t_pic master;
    t_pic slave;
    core_machine_rtc rtc;
} rtc_cmos_s3_fixture;

static C_VOID rtc_cmos_s3_initialize_pic(t_port *port)
{
    core_machine_port_write(port, 0x0020u, 0x11u);
    core_machine_port_write(port, 0x0021u, 0x08u);
    core_machine_port_write(port, 0x0021u, 0x04u);
    core_machine_port_write(port, 0x0021u, 0x01u);
    core_machine_port_write(port, 0x00a0u, 0x11u);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    core_machine_port_write(port, 0x00a1u, 0x02u);
    core_machine_port_write(port, 0x00a1u, 0x01u);
}

static C_VOID rtc_cmos_s3_initialize(rtc_cmos_s3_fixture *fixture)
{
    core_machine_rtc_config config = { .irq = 8u, .ticks_per_second = 4u };

    core_machine_port_initialize(&fixture->port);
    core_machine_pic_initialize(&fixture->master, &fixture->slave, &fixture->port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    rtc_cmos_s3_initialize_pic(&fixture->port);
    core_machine_rtc_initialize(&fixture->rtc, &fixture->master, &fixture->slave,
        &config);
}

static C_VOID rtc_cmos_s3_finalize(rtc_cmos_s3_fixture *fixture)
{
    core_machine_rtc_finalize(&fixture->rtc);
    core_machine_pic_finalize(&fixture->master, &fixture->slave);
    core_machine_port_finalize(&fixture->port);
}

static C_VOID rtc_cmos_s3_write(core_machine_rtc *rtc, type_unsigned_8 index,
    type_unsigned_8 value)
{
    core_machine_rtc_select_register(rtc, index);
    core_machine_rtc_write_selected(rtc, value);
}

static type_unsigned_8 rtc_cmos_s3_read(core_machine_rtc *rtc,
    type_unsigned_8 index)
{
    core_machine_rtc_select_register(rtc, index);
    return core_machine_rtc_read_selected(rtc);
}

static C_INT rtc_cmos_s3_test_events_and_irq8(C_VOID)
{
    rtc_cmos_s3_fixture fixture;
    type_unsigned_8 flags;
    C_INT failed = 0;

    rtc_cmos_s3_initialize(&fixture);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_SECOND_ALARM, 0xc0u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_MINUTE_ALARM, 0xc0u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_HOUR_ALARM, 0xc0u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H);
    core_machine_rtc_advance(&fixture.rtc, 4u);
    flags = fixture.rtc.registers[CORE_MACHINE_RTC_REG_C];
    failed |= (flags & (CORE_MACHINE_RTC_REG_C_PF | CORE_MACHINE_RTC_REG_C_AF |
        CORE_MACHINE_RTC_REG_C_UF)) != (CORE_MACHINE_RTC_REG_C_PF |
        CORE_MACHINE_RTC_REG_C_AF | CORE_MACHINE_RTC_REG_C_UF) ||
        (flags & CORE_MACHINE_RTC_REG_C_IRQF) != 0u || fixture.rtc.irq_source.asserted;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_PIE |
        CORE_MACHINE_RTC_REG_B_AIE | CORE_MACHINE_RTC_REG_B_UIE);
    core_machine_pic_refresh(&fixture.master, &fixture.slave);
    failed |= (fixture.rtc.registers[CORE_MACHINE_RTC_REG_C] &
        CORE_MACHINE_RTC_REG_C_IRQF) == 0u || !fixture.rtc.irq_source.asserted ||
        core_machine_pic_get_interrupt(&fixture.master, &fixture.slave) != 0x70u;
    flags = rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_REG_C);
    failed |= (flags & (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_PF |
        CORE_MACHINE_RTC_REG_C_AF | CORE_MACHINE_RTC_REG_C_UF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_PF |
        CORE_MACHINE_RTC_REG_C_AF | CORE_MACHINE_RTC_REG_C_UF) ||
        fixture.rtc.irq_source.asserted;
    core_machine_port_write(&fixture.port, 0x00a0u, 0x20u);
    core_machine_port_write(&fixture.port, 0x0020u, 0x20u);
    core_machine_rtc_advance(&fixture.rtc, 4u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H);
    failed |= (fixture.rtc.registers[CORE_MACHINE_RTC_REG_C] &
        (CORE_MACHINE_RTC_REG_C_PF | CORE_MACHINE_RTC_REG_C_AF |
        CORE_MACHINE_RTC_REG_C_UF)) == 0u || (fixture.rtc.registers[
        CORE_MACHINE_RTC_REG_C] & CORE_MACHINE_RTC_REG_C_IRQF) != 0u ||
        fixture.rtc.irq_source.asserted;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_UIE);
    failed |= !fixture.rtc.irq_source.asserted;
    core_machine_rtc_finalize(&fixture.rtc);
    failed |= fixture.rtc.irq_source.asserted;
    rtc_cmos_s3_finalize(&fixture);
    return failed;
}

static C_INT rtc_cmos_s3_test_calendar_and_reset(C_VOID)
{
    rtc_cmos_s3_fixture fixture;
    C_INT failed = 0;

    rtc_cmos_s3_initialize(&fixture);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_SET);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_HOUR, 0x92u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_SECOND, 0x59u);
    core_machine_rtc_advance(&fixture.rtc, 4u);
    failed |= rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_HOUR) != 0x92u ||
        rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_SECOND) != 0x59u;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B, 0u);
    core_machine_rtc_advance(&fixture.rtc, 4u);
    failed |= rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_HOUR) != 0x92u ||
        rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_MINUTE) != 0x01u ||
        rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_SECOND) != 0x00u;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_SET | CORE_MACHINE_RTC_REG_B_DM);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_HOUR, 0x8cu);
    failed |= rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_HOUR) != 0x8cu;
    core_machine_rtc_write_nvram(&fixture.rtc, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    core_machine_rtc_reset(&fixture.rtc);
    failed |= rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au ||
        rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_MINUTE) != 0x01u ||
        rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_SECOND) != 0x00u ||
        fixture.rtc.irq_source.asserted;
    rtc_cmos_s3_finalize(&fixture);
    return failed;
}

static C_INT rtc_cmos_s3_test_cmos_adapter(C_VOID)
{
    core_machine_config config = { 0 };
    core_machine_rtc_cmos_config rtc_config = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT masked = 0;
    C_INT failed = 0;

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
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_rtc_cmos(machine, &rtc_config) != TYPE_STATUS_INVALID_ARGUMENT) {
        failed = 1;
    }
    rtc_config.timing = (core_machine_rtc_timing_plan) {1u, 1u,
        CORE_MACHINE_RTC_TIMING_L3_SOURCE};
    if (failed ||
        core_machine_configure_rtc_cmos(machine, &rtc_config) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0070u, 0x94u) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0071u, &value) != TYPE_STATUS_OK ||
        value != 0x5au || core_machine_get_nmi_mask(machine, &masked) !=
            TYPE_STATUS_OK || !masked ||
        core_machine_bus_write(machine, 0x0070u, CORE_MACHINE_RTC_EQUIPMENT) !=
            TYPE_STATUS_OK || core_machine_get_nmi_mask(machine, &masked) !=
            TYPE_STATUS_OK || masked) {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT rtc_cmos_s3_test_phase_and_divider(C_VOID)
{
    rtc_cmos_s3_fixture fixture;
    core_machine_rtc_config config = { .irq = 8u, .ticks_per_second = 32768u,
        .timing = {8u, 65u, CORE_MACHINE_RTC_TIMING_L3_SOURCE} };
    C_INT failed = 0;

    core_machine_port_initialize(&fixture.port);
    core_machine_pic_initialize(&fixture.master, &fixture.slave, &fixture.port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    rtc_cmos_s3_initialize_pic(&fixture.port);
    core_machine_rtc_initialize(&fixture.rtc, &fixture.master, &fixture.slave,
        &config);
    core_machine_rtc_advance(&fixture.rtc, 32768u - 73u);
    failed |= (rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_REG_A) &
        CORE_MACHINE_RTC_REG_A_UIP) == 0u;
    core_machine_rtc_reset(&fixture.rtc);
    failed |= (rtc_cmos_s3_read(&fixture.rtc, CORE_MACHINE_RTC_REG_A) &
        CORE_MACHINE_RTC_REG_A_UIP) != 0u;
    core_machine_rtc_advance(&fixture.rtc, 32768u);
    failed |=
        fixture.rtc.calendar.second != 1u ||
        (fixture.rtc.registers[CORE_MACHINE_RTC_REG_C] & CORE_MACHINE_RTC_REG_C_UF) == 0u;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_A, 0x06u);
    core_machine_rtc_advance(&fixture.rtc, 32768u);
    failed |= fixture.rtc.calendar.second != 1u;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_A, 0x26u);
    core_machine_rtc_advance(&fixture.rtc, 16383u);
    failed |= fixture.rtc.calendar.second != 1u;
    core_machine_rtc_advance(&fixture.rtc, 1u);
    failed |= fixture.rtc.calendar.second != 2u;
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_SQWE);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_A, 0x2fu);
    core_machine_rtc_advance(&fixture.rtc, 16384u);
    failed |= !core_machine_rtc_get_square_wave(&fixture.rtc);
    core_machine_rtc_reset(&fixture.rtc);
    failed |= core_machine_rtc_get_square_wave(&fixture.rtc) ||
        (fixture.rtc.registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_SQWE) != 0u;
    rtc_cmos_s3_write(&fixture.rtc, 0x32u, 0x5au);
    failed |= rtc_cmos_s3_read(&fixture.rtc, 0x32u) != 0x5au;
    rtc_cmos_s3_finalize(&fixture);
    return failed;
}

static C_INT rtc_cmos_s3_test_alarm_deadline(C_VOID)
{
    rtc_cmos_s3_fixture fixture;
    type_unsigned_64 ticks = 0u;
    C_INT failed = 0;

    rtc_cmos_s3_initialize(&fixture);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_REG_B,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_AIE);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_SECOND_ALARM, 0x02u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_MINUTE_ALARM, 0x00u);
    rtc_cmos_s3_write(&fixture.rtc, CORE_MACHINE_RTC_HOUR_ALARM, 0x00u);
    failed |= core_machine_rtc_ticks_until_irq(&fixture.rtc, &ticks) !=
        TYPE_STATUS_OK || ticks != 8u;
    core_machine_rtc_advance(&fixture.rtc, ticks);
    failed |= (fixture.rtc.registers[CORE_MACHINE_RTC_REG_C] &
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_AF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_AF) ||
        !fixture.rtc.irq_source.asserted;
    rtc_cmos_s3_finalize(&fixture);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= rtc_cmos_s3_test_events_and_irq8();
    failed |= rtc_cmos_s3_test_calendar_and_reset();
    failed |= rtc_cmos_s3_test_cmos_adapter();
    failed |= rtc_cmos_s3_test_phase_and_divider();
    failed |= rtc_cmos_s3_test_alarm_deadline();
    if (failed != 0) return 1;
    STD_PRINTF("M5:T350:S3:RTC-CMOS:OK\n");
    return 0;
}
