#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine.h"

void core_machine_board_media_advance(void *owner, lib_u64 source_ticks,
    lib_u64 due_tick)
{
    core_machine *machine = owner;

    if (machine == LIB_NULL || source_ticks == 0u) return;
    if (machine->fdc_configured) {
        core_machine_fdc_advance_at(&machine->fdc, due_tick);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_ADVANCE,
            0u, 0u, 0u);
    }
    if (machine->hdc_configured) {
        core_machine_hdc_advance_at(&machine->hdc, due_tick);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_ADVANCE,
            0u, 0u, 0u);
    }
}

void core_machine_board_rtc_advance(void *owner, lib_u64 source_ticks)
{
    core_machine *machine = owner;
    lib_u64 rtc_ticks;

    if (machine == LIB_NULL || source_ticks == 0u) return;
    rtc_ticks = core_machine_clock_domain_advance(&machine->rtc_clock, source_ticks);
    if (machine->rtc_cmos_configured) {
        x86_rtc_advance(machine->shared_rtc, rtc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RTC_ADVANCE,
        0u, (lib_u32)rtc_ticks, 0u);
}

/* Input and video follow Core's readiness phase. Presentation only consumes
 * copied snapshots outside this guest-time advancement. */
void core_machine_board_peripheral_advance(void *owner, lib_u64 source_ticks)
{
    core_machine *machine = owner;
    lib_u64 kbc_ticks;
    lib_u64 vadp_ticks;

    if (machine == LIB_NULL || source_ticks == 0u) return;
    kbc_ticks = core_machine_clock_domain_advance(&machine->kbc_clock, source_ticks);
    if (machine->keyboard_topology == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI) {
        x86_xt_keyboard_advance(machine->xt_keyboard, source_ticks);
    } else {
        core_machine_kbc_advance(&machine->shared_kbc, kbc_ticks);
    }
    core_machine_pic_advance(&machine->shared_pic_master, &machine->shared_pic_slave,
        source_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_KBC_ADVANCE,
        0u, (lib_u32)kbc_ticks, 0u);
    vadp_ticks = core_machine_clock_domain_advance(&machine->vadp_clock, source_ticks);
    x86_video_advance(machine->shared_vadp.chip, vadp_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_VADP_ADVANCE,
        0u, (lib_u32)vadp_ticks, 0u);
}
