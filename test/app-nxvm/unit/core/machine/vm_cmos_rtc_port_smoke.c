#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"
#include "support/rom/session_assets.h"

static void cmos_write(t_port *port, lib_u8 reg, lib_u8 value)
{
    core_machine_port_write(port, 0x0070u, reg);
    core_machine_port_write(port, 0x0071u, value);
}

static lib_u8 cmos_read(t_port *port, lib_u8 reg)
{
    core_machine_port_write(port, 0x0070u, reg);
    return (lib_u8)core_machine_port_read(port, 0x0071u);
}

static void initialize_pic(t_port *port)
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

static void advance_cmos(core_machine *machine, lib_u64 elapsed_ticks)
{
    x86_rtc_advance(machine->board->shared_rtc, elapsed_ticks);
    core_machine_pic_refresh(&machine->board->shared_pic_master,
        &machine->board->shared_pic_slave);
}

static lib_i32 default_at_cmos_seed_is_loaded(void)
{
    lib_u8 seed[VM_MACHINE_CMOS_SEED_BYTES] = {0};
    vm_machine_config config = {0};
    vm_machine_assets assets;
    vm_machine *session = LIB_NULL;
    lib_u16 checksum = 0u;
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
        seed[index] = (lib_u8)(0xa5u ^ index);
    }
    /* A session seed owns the whole board-NVRAM image, not just vendor bytes.
     * Supply a valid AT checksum exactly as an external .cmos asset would. */
    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (lib_u16)(checksum + seed[index]);
    }
    seed[0x2eu] = CORE_MACHINE_MASK_U8(checksum >> 8u);
    seed[0x2fu] = CORE_MACHINE_MASK_U8(checksum);
    vm_test_default_pc_at_assets(&assets,
        (lib_u8[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES]) {0});
    /* The helper's ROM array must outlive composition only; session copies it. */
    assets.cmos_seed = (vm_machine_asset_bytes) { seed, sizeof(seed) };
    config.profile_kind = VM_MACHINE_PROFILE_DEFAULT_PC_AT;
    config.bios_count = 1u;
    failed |= vm_machine_create_from_assets(&config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;
    if (!failed) {
        t_port *port = &session->core_machine->executor_port;

        for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
            failed |= cmos_read(port, (lib_u8)index) != seed[index];
        }
    }
    vm_machine_destroy(session);
    return failed;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    t_port *port;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) return 1;
    port = &session->core_machine->executor_port;
    if (!session->active) failed = 1;
    initialize_pic(port);

    if (cmos_read(port, X86_RTC_REG_D) != X86_RTC_REG_D_VRT) failed |= 0x0001;
    if (cmos_read(port, X86_RTC_SECOND) != 0x00u) failed |= 0x0002;
    advance_cmos(session->core_machine, 50000u);
    if (cmos_read(port, X86_RTC_SECOND) != 0x01u) failed |= 0x0004;

    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    advance_cmos(session->core_machine, 50000u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->board->rtc_irq_source.master,
        session->core_machine->board->rtc_irq_source.slave)) failed |= 0x0008;
    if (core_machine_pic_get_interrupt(session->core_machine->board->rtc_irq_source.master,
        session->core_machine->board->rtc_irq_source.slave) != 0x70u) failed |= 0x0010;
    if ((cmos_read(port, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) failed |= 0x0020;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);
    if (core_machine_pic_scan_interrupt(session->core_machine->board->rtc_irq_source.master,
        session->core_machine->board->rtc_irq_source.slave)) failed |= 0x0040;

    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_PIE);
    advance_cmos(session->core_machine, 50u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->board->rtc_irq_source.master,
        session->core_machine->board->rtc_irq_source.slave) ||
        core_machine_pic_get_interrupt(session->core_machine->board->rtc_irq_source.master,
            session->core_machine->board->rtc_irq_source.slave) != 0x70u) failed |= 0x0080;
    if ((cmos_read(port, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF)) failed |= 0x0100;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);

    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_SET);
    cmos_write(port, X86_RTC_SECOND, 0x11u);
    advance_cmos(session->core_machine, 100000u);
    if (cmos_read(port, X86_RTC_SECOND) != 0x11u) failed |= 0x0200;
    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H);

    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_DM);
    cmos_write(port, X86_RTC_HOUR, 0x81u);
    if (cmos_read(port, X86_RTC_HOUR) != 0x81u) failed |= 0x0400;

    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_SET);
    cmos_write(port, X86_RTC_SECOND, 0x58u);
    cmos_write(port, X86_RTC_MINUTE, 0x00u);
    cmos_write(port, X86_RTC_HOUR, 0x00u);
    cmos_write(port, X86_RTC_SECOND_ALARM, 0x59u);
    cmos_write(port, X86_RTC_MINUTE_ALARM, 0x00u);
    cmos_write(port, X86_RTC_HOUR_ALARM, 0x00u);
    cmos_write(port, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_AIE);
    advance_cmos(session->core_machine, 50000u);
    if (!core_machine_pic_scan_interrupt(session->core_machine->board->rtc_irq_source.master,
        session->core_machine->board->rtc_irq_source.slave) ||
        core_machine_pic_get_interrupt(session->core_machine->board->rtc_irq_source.master,
            session->core_machine->board->rtc_irq_source.slave) != 0x70u) failed |= 0x0800;
    if ((cmos_read(port, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF)) failed |= 0x1000;
    core_machine_port_write(port, 0x00a0u, 0x20u);
    core_machine_port_write(port, 0x0020u, 0x20u);

    cmos_write(port, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    x86_rtc_reset(session->core_machine->board->shared_rtc);
    if (cmos_read(port, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au) failed |= 0x2000;
    if (cmos_read(port, X86_RTC_SECOND) != 0x59u) failed |= 0x4000;

    failed |= default_at_cmos_seed_is_loaded();
    if (failed) {
        printf("RTC probe failed=%04x: second=%u hour=%u B=%02x\n", failed,
            x86_rtc_read_register(session->core_machine->board->shared_rtc, X86_RTC_SECOND),
            x86_rtc_read_register(session->core_machine->board->shared_rtc, X86_RTC_HOUR),
            x86_rtc_read_register(session->core_machine->board->shared_rtc, X86_RTC_REG_B));
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T232:S1:CMOS-RTC-PORT:OK");
    return 0;
}
