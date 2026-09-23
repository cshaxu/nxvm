#ifndef CORE_MACHINE_H
#define CORE_MACHINE_H

#include "core/cartridge.h"
#include "core/controller.h"
#include "core/apu.h"
#include "core/machine_interface.h"
#include "core/ppu.h"

struct core_machine {
    core_cartridge *cartridge;
    core_apu apu;
    core_ppu ppu;
    core_controller controller;
    lib_u8 ram[2048];
    lib_u8 a;
    lib_u8 x;
    lib_u8 y;
    lib_u8 s;
    lib_u8 p;
    lib_u16 pc;
    lib_u8 data_latch;
    lib_u8 initial_ram_byte;
    lib_bool irq_asserted;
    lib_bool external_irq_asserted;
    lib_bool cartridge_irq_asserted;
    lib_bool apu_irq_asserted;
    lib_bool external_nmi_asserted;
    lib_bool ppu_nmi_asserted;
    lib_bool irq_poll_i;
    lib_bool nmi_asserted;
    lib_bool nmi_pending;
    lib_bool nmi_defer_once;
    lib_u8 interrupt_phase;
    lib_u8 dma_page;
    lib_bool dma_pending;
    lib_bool dma_active;
    lib_bool dma_alignment_pending;
    lib_u8 dma_phase;
    lib_u16 dma_index;
    lib_u8 dma_latch;
    lib_u16 dmc_dma_address;
    lib_u8 dmc_dma_phase;
    lib_u8 dmc_dma_latch;
    lib_bool dmc_dma_active;
    lib_u64 cycles;
    lib_u64 slots;
    lib_u64 instructions;
    core_observation trap;
    core_bus_transfer trace[CORE_MACHINE_TRACE_CAPACITY];
    lib_u32 trace_count;
    lib_u16 breakpoints[16];
    lib_u16 breakpoint_count;
    lib_u16 breakpoint_bypass_pc;
    lib_bool breakpoint_bypass_valid;
};

lib_status core_machine_step(core_machine *machine, lib_u32 *out_cycles);
lib_status core_machine_service_interrupt(core_machine *machine,
    lib_bool *out_serviced, lib_u32 *out_cycles);
lib_status core_machine_service_dma(core_machine *machine,
    lib_bool *out_serviced, lib_u32 *out_cycles);
lib_status core_machine_debug_step(core_machine *machine, lib_u32 instruction_limit,
    lib_u32 cycle_limit, core_run_result *out_result);
lib_status core_machine_breakpoint_set(core_machine *machine, lib_u16 address,
    lib_bool enabled);
lib_status core_machine_breakpoint_list(const core_machine *machine,
    lib_u16 *out_addresses, lib_u32 capacity, lib_u32 *out_count);
void core_machine_refresh_cartridge_irq(core_machine *machine);
void core_machine_refresh_apu_irq(core_machine *machine);

#endif
