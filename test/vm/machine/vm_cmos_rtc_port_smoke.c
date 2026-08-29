#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "core/machine/machine.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "core/machine/rtc.h"

static C_VOID cmos_write(t_port *port, type_unsigned_8 reg, type_unsigned_8 value)
{
    core_machine_port_write(port, 0x0070u, reg);
    core_machine_port_write(port, 0x0071u, value);
}

static type_unsigned_8 cmos_read(t_port *port, type_unsigned_8 reg)
{
    core_machine_port_write(port, 0x0070u, reg);
    return (type_unsigned_8)core_machine_port_read(port, 0x0071u);
}

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

static C_VOID advance_cmos(core_machine_rtc *cmos, type_unsigned_64 elapsed_ticks)
{
    core_machine_rtc_advance(cmos, elapsed_ticks);
    core_machine_pic_refresh(cmos->irq_source.master,
        cmos->irq_source.slave);
}

C_INT main(C_VOID)
{
    vm_session *session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    t_port *port;
    C_INT failed = 0;

    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    port = session->core_machine->fdc.connect.port;
    if (!session->active || port == STD_NULL) failed = 1;
    initialize_pic(port);

    if (cmos_read(port, CORE_MACHINE_RTC_REG_D) != CORE_MACHINE_RTC_REG_D_VRT) failed |= 0x0001;
    if (cmos_read(port, CORE_MACHINE_RTC_SECOND) != 0x00u) failed |= 0x0002;
    advance_cmos(&session->core_machine->shared_rtc, 50000u);
    if (cmos_read(port, CORE_MACHINE_RTC_SECOND) != 0x01u) failed |= 0x0004;

    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_UIE);
    advance_cmos(&session->core_machine->shared_rtc, 50000u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->shared_rtc.irq_source.master,
        session->core_machine->shared_rtc.irq_source.slave)) failed |= 0x0008;
    if (core_machine_pic_get_interrupt(session->core_machine->shared_rtc.irq_source.master,
        session->core_machine->shared_rtc.irq_source.slave) != 0x70u) failed |= 0x0010;
    if ((cmos_read(port, CORE_MACHINE_RTC_REG_C) &
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_UF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_UF)) failed |= 0x0020;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);
    if (core_machine_pic_scan_interrupt(session->core_machine->shared_rtc.irq_source.master,
        session->core_machine->shared_rtc.irq_source.slave)) failed |= 0x0040;

    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_PIE);
    advance_cmos(&session->core_machine->shared_rtc, 50u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->shared_rtc.irq_source.master,
        session->core_machine->shared_rtc.irq_source.slave) ||
        core_machine_pic_get_interrupt(session->core_machine->shared_rtc.irq_source.master,
            session->core_machine->shared_rtc.irq_source.slave) != 0x70u) failed |= 0x0080;
    if ((cmos_read(port, CORE_MACHINE_RTC_REG_C) &
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_PF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_PF)) failed |= 0x0100;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);

    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_SET);
    cmos_write(port, CORE_MACHINE_RTC_SECOND, 0x11u);
    advance_cmos(&session->core_machine->shared_rtc, 100000u);
    if (cmos_read(port, CORE_MACHINE_RTC_SECOND) != 0x11u) failed |= 0x0200;
    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H);

    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_DM);
    cmos_write(port, CORE_MACHINE_RTC_HOUR, 0x81u);
    if (cmos_read(port, CORE_MACHINE_RTC_HOUR) != 0x81u) failed |= 0x0400;

    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_SET);
    cmos_write(port, CORE_MACHINE_RTC_SECOND, 0x58u);
    cmos_write(port, CORE_MACHINE_RTC_MINUTE, 0x00u);
    cmos_write(port, CORE_MACHINE_RTC_HOUR, 0x00u);
    cmos_write(port, CORE_MACHINE_RTC_SECOND_ALARM, 0x59u);
    cmos_write(port, CORE_MACHINE_RTC_MINUTE_ALARM, 0x00u);
    cmos_write(port, CORE_MACHINE_RTC_HOUR_ALARM, 0x00u);
    cmos_write(port, CORE_MACHINE_RTC_REG_B, CORE_MACHINE_RTC_REG_B_24H | CORE_MACHINE_RTC_REG_B_AIE);
    advance_cmos(&session->core_machine->shared_rtc, 50000u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->shared_rtc.irq_source.master,
        session->core_machine->shared_rtc.irq_source.slave) ||
        core_machine_pic_get_interrupt(session->core_machine->shared_rtc.irq_source.master,
            session->core_machine->shared_rtc.irq_source.slave) != 0x70u) failed |= 0x0800;
    if ((cmos_read(port, CORE_MACHINE_RTC_REG_C) &
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_AF)) !=
        (CORE_MACHINE_RTC_REG_C_IRQF | CORE_MACHINE_RTC_REG_C_AF)) failed |= 0x1000;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);

    cmos_write(port, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    core_machine_rtc_reset(&session->core_machine->shared_rtc);
    if (cmos_read(port, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au) failed |= 0x2000;
    if (cmos_read(port, CORE_MACHINE_RTC_SECOND) != 0x59u) failed |= 0x4000;

    if (failed) {
        STD_PRINTF("RTC probe failed=%04x: second=%u hour=%u C=%02x B=%02x IRR=%02x/%02x ISR=%02x/%02x\n", failed,
            session->core_machine->shared_rtc.calendar.second,
            session->core_machine->shared_rtc.calendar.hour,
            session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_REG_C],
            session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_REG_B],
            session->core_machine->shared_rtc.irq_source.master->data.irr,
            session->core_machine->shared_rtc.irq_source.slave->data.irr,
            session->core_machine->shared_rtc.irq_source.master->data.isr,
            session->core_machine->shared_rtc.irq_source.slave->data.isr);
    }
    vm_session_finalize(session);
    STD_FREE(session);
    if (failed) return 1;
    puts("M5:T232:S1:CMOS-RTC-PORT:OK");
    return 0;
}
