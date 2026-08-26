#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "core/machine/rtc.h"

static C_VOID initialize_pic(t_port *port)
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

C_INT main(C_VOID)
{
    t_port port;
    t_pic master;
    t_pic slave;
    core_machine_rtc rtc;
    core_machine_rtc_config config = { .irq = 8u, .ticks_per_second = 50000u };
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port,
        CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    initialize_pic(&port);
    core_machine_rtc_initialize(&rtc, &master, &slave, &config);
    core_machine_rtc_select_register(&rtc, CORE_MACHINE_RTC_REG_B);
    core_machine_rtc_write_selected(&rtc,
        CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_UIE);
    core_machine_rtc_advance(&rtc, 50000u);
    core_machine_pic_refresh(&master, &slave);
    if (!core_machine_pic_scan_interrupt(&master, &slave) ||
        core_machine_pic_get_interrupt(&master, &slave) != 0x70u) {
        failed = 1;
    }
    core_machine_rtc_select_register(&rtc, CORE_MACHINE_RTC_REG_C);
    if ((core_machine_rtc_read_selected(&rtc) &
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_UF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_UF)) {
        failed = 1;
    }
    core_machine_pic_refresh(&master, &slave);
    if (core_machine_pic_scan_interrupt(&master, &slave)) failed = 1;
    core_machine_rtc_write_nvram(&rtc, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    core_machine_rtc_reset(&rtc);
    core_machine_rtc_select_register(&rtc, CORE_MACHINE_RTC_EQUIPMENT);
    if (core_machine_rtc_read_selected(&rtc) != 0x5au) failed = 1;
    core_machine_rtc_finalize(&rtc);
    core_machine_pic_finalize(&master, &slave);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    puts("M5:T273:S2:CORE-RTC:OK");
    return 0;
}
