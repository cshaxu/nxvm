#include "lib/types/types_interface.h"
#include <stdio.h>

#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/core/machine_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"

static lib_bool initialize_pic(core_machine *port)
{
    return core_machine_bus_write(port, 0x0020u, 0x11u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x08u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x04u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x0021u, 0x01u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a0u, 0x11u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x70u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x02u) == LIB_STATUS_OK &&
        core_machine_bus_write(port, 0x00a1u, 0x01u) == LIB_STATUS_OK;
}

static void rtc_output(void *context, lib_bool asserted)
{
    core_machine_pic_irq_source *source = context;
    if (asserted) core_machine_pic_irq_source_assert(source);
    else core_machine_pic_irq_source_deassert(source);
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine *port;
    core_machine_pic_bus *master = LIB_NULL;
    core_machine_pic_bus *slave = LIB_NULL;
    x86_rtc *rtc = LIB_NULL;
    core_machine_pic_irq_source *irq_source = LIB_NULL;
    x86_rtc_config config = {50000u, 0u, 0u};
    lib_i32 failed = 0;

    const core_machine_executor_config executor_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    if (core_machine_neutral_create(&executor_config, &machine) != LIB_STATUS_OK)
        return 1;
    port = machine;
    if (core_machine_pic_initialize(&master, &slave, machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_pic_finalize(master, slave);
        core_machine_destroy(machine);
        return 1;
    }
    if (!initialize_pic(port) ||
        core_machine_pic_irq_source_bind(&irq_source, master, slave, 8u) !=
            LIB_STATUS_OK ||
        x86_rtc_create(&config, rtc_output, irq_source, &rtc) != LIB_STATUS_OK) {
        core_machine_pic_finalize(master, slave);
        core_machine_destroy(machine);
        return 1;
    }
    x86_rtc_write_register(rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    x86_rtc_advance(rtc, 50000u);
    core_machine_pic_refresh(master, slave);
    if (!core_machine_pic_scan_interrupt(master, slave) ||
        core_machine_pic_get_interrupt(master, slave) != 0x70u) {
        failed = 1;
    }
    if ((x86_rtc_read_register(rtc, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) {
        failed = 1;
    }
    core_machine_pic_refresh(master, slave);
    if (core_machine_pic_scan_interrupt(master, slave)) failed = 1;
    x86_rtc_write_register(rtc, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    x86_rtc_reset(rtc);
    if (x86_rtc_read_register(rtc, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au) failed = 1;
    x86_rtc_destroy(rtc);
    core_machine_pic_finalize(master, slave);
    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T273:S2:CORE-RTC:OK");
    return 0;
}
