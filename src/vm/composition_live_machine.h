#ifndef NXVM_COMPOSITION_LIVE_MACHINE_H
#define NXVM_COMPOSITION_LIVE_MACHINE_H

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/dma.h"
#include "core/machine/kbc.h"
#include "core/machine/vadp.h"
#include "core/machine/block_provider.h"
#include "core/machine/keyboard_interface.h"
#include "core/machine/display_interface.h"
#include "vm/machine/cmos.h"
#include "vm/machine/fdd.h"
#include "vm/machine/fdc.h"
#include "vm/machine/hdd.h"
#include "vm/machine/debug.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "vm/profile/default_profile/firmware/context.h"
#include "vm/composition_control.h"
#include "core/product/debug/debug_target.h"

typedef struct vm_composition_live_machine {
    t_cpu cpu_storage;
    t_cpuins cpuins_storage;
    core_machine_cpu_execution_context cpu_execution_storage;
    t_ram ram_storage;
    t_port port_storage;
    t_pic pic_master_storage;
    t_pic pic_slave_storage;
    t_pit pit_storage;
    t_latch dma_latch_storage;
    t_dma dma_primary_storage;
    t_dma dma_secondary_storage;
    t_kbc kbc_storage;
    t_vadp vadp_storage;
    t_cmos cmos_storage;
    t_fdd fdd_storage;
    t_fdc fdc_storage;
    t_hdd hdd_storage;
    t_debug debug_storage;
    t_bios default_bios_storage;
    t_qdx default_qdx_storage;
    vm_profile_default_context default_profile_context_storage;
    core_machine_block_provider_slot block_provider_storage;
    core_machine_keyboard_provider_slot keyboard_provider_storage;
    core_machine_display_provider_slot display_provider_storage;
    core_product_debug_target *debug_target;
    t_cpu *cpu;
    t_cpuins *cpuins;
    core_machine_cpu_execution_context *cpu_execution;
    t_ram *ram;
    t_port *port;
    t_pic *pic_master;
    t_pic *pic_slave;
    t_pit *pit;
    t_latch *dma_latch;
    t_dma *dma_primary;
    t_dma *dma_secondary;
    t_kbc *kbc;
    t_vadp *vadp;
    t_cmos *cmos;
    t_fdd *fdd;
    t_fdc *fdc;
    t_hdd *hdd;
    t_debug *debug;
    t_bios *default_bios;
    t_qdx *default_qdx;
    vm_profile_default_context *default_profile_context;
    core_machine_block_provider_slot *block_provider;
    core_machine_keyboard_provider_slot *keyboard_provider;
    core_machine_display_provider_slot *display_provider;
    vm_composition_control_state *control;
} vm_composition_live_machine;

void vm_composition_live_machine_initialize(vm_composition_live_machine *machine);
void vm_composition_live_machine_finalize(vm_composition_live_machine *machine);

#endif
