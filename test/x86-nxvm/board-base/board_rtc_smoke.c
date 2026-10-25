#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"

static lib_bool initialize_pic(x86_core_machine *port)
{
    return x86_core_machine_bus_write(port, 0x0020u, 0x11u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x08u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x04u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x0021u, 0x01u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a0u, 0x11u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x70u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x02u) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(port, 0x00a1u, 0x01u) == LIB_STATUS_OK;
}

static void rtc_output(void *context, lib_bool asserted)
{
    x86_nxvm_board_pic_irq_source *source = context;
    if (asserted) x86_nxvm_board_pic_irq_source_assert(source);
    else x86_nxvm_board_pic_irq_source_deassert(source);
}

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine *port;
    x86_nxvm_board_pic_bus *master = LIB_NULL;
    x86_nxvm_board_pic_bus *slave = LIB_NULL;
    x86_rtc *rtc = LIB_NULL;
    x86_nxvm_board_pic_irq_source *irq_source = LIB_NULL;
    x86_rtc_config config = {50000u, 0u, 0u, LIB_FALSE};
    lib_i32 failed = 0;

    const x86_core_machine_executor_config executor_config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    if (x86_core_machine_neutral_create(&executor_config, &machine) != LIB_STATUS_OK)
        return 1;
    port = machine;
    if (x86_nxvm_board_pic_initialize(&master, &slave, machine,
            X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) {
        x86_nxvm_board_pic_finalize(master, slave);
        x86_core_machine_destroy(machine);
        return 1;
    }
    if (!initialize_pic(port) ||
        x86_nxvm_board_pic_irq_source_bind(&irq_source, master, slave, 8u) !=
            LIB_STATUS_OK ||
        x86_rtc_create(&config, rtc_output, irq_source, &rtc) != LIB_STATUS_OK) {
        x86_nxvm_board_pic_finalize(master, slave);
        x86_core_machine_destroy(machine);
        return 1;
    }
    x86_rtc_write_register(rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    x86_rtc_advance(rtc, 50000u);
    x86_nxvm_board_pic_refresh(master, slave);
    if (!x86_nxvm_board_pic_scan_interrupt(master, slave) ||
        x86_nxvm_board_pic_get_interrupt(master, slave) != 0x70u) {
        failed = 1;
    }
    if ((x86_rtc_read_register(rtc, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) {
        failed = 1;
    }
    x86_nxvm_board_pic_refresh(master, slave);
    if (x86_nxvm_board_pic_scan_interrupt(master, slave)) failed = 1;
    x86_rtc_write_register(rtc, X86_NXVM_BOARD_RTC_EQUIPMENT, 0x5au);
    x86_rtc_reset(rtc);
    if (x86_rtc_read_register(rtc, X86_NXVM_BOARD_RTC_EQUIPMENT) != 0x5au) failed = 1;
    x86_rtc_destroy(rtc);
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("%s\n", "CORE-RTC:OK");
    return 0;
}
