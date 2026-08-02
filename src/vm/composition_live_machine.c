#include "vm/composition_live_machine.h"

void vm_composition_live_machine_initialize(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    machine->cpu = &machine->cpu_storage;
    machine->cpuins = &machine->cpuins_storage;
    machine->ram = &machine->ram_storage;
    machine->port = &machine->port_storage;
    machine->pic_master = &machine->pic_master_storage;
    machine->pic_slave = &machine->pic_slave_storage;
    machine->pit = &machine->pit_storage;
    machine->dma_latch = &machine->dma_latch_storage;
    machine->dma_primary = &machine->dma_primary_storage;
    machine->dma_secondary = &machine->dma_secondary_storage;
    machine->kbc = &machine->kbc_storage;
    machine->vadp = &machine->vadp_storage;
    machine->cmos = &machine->cmos_storage;
    machine->fdd = &machine->fdd_storage;
    machine->fdc = &machine->fdc_storage;
    machine->hdd = &machine->hdd_storage;
    machine->debug = &machine->debug_storage;
    machine->default_bios = &machine->default_bios_storage;
    machine->default_qdx = &machine->default_qdx_storage;
}

void vm_composition_live_machine_bind_legacy(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    core_machine_cpu_bind_live(machine->cpu);
    core_machine_cpu_instructions_bind_live(machine->cpuins);
    core_machine_memory_bind_live(machine->ram);
    core_machine_port_bind_live(machine->port);
    core_machine_pic_bind_live(machine->pic_master, machine->pic_slave);
    core_machine_pit_bind_live(machine->pit);
    core_machine_dma_bind_live(machine->dma_latch, machine->dma_primary,
        machine->dma_secondary);
    core_machine_kbc_bind_live(machine->kbc);
    core_machine_vadp_bind_live(machine->vadp);
    vm_machine_cmos_bind_live(machine->cmos);
    vm_machine_fdd_bind_live(machine->fdd);
    vm_machine_fdc_bind_live(machine->fdc);
    vm_machine_hdd_bind_live(machine->hdd);
    vm_machine_debug_bind_live(machine->debug);
    vm_profile_default_bios_bind_live(machine->default_bios);
    vm_profile_default_qdx_bind_live(machine->default_qdx);
}

void vm_composition_live_machine_finalize(vm_composition_live_machine *machine)
{
    core_machine_memory_unbind_live();
    core_machine_port_unbind_live();
    core_machine_pic_unbind_live();
    core_machine_pit_unbind_live();
    core_machine_dma_unbind_live();
    core_machine_kbc_unbind_live();
    core_machine_vadp_unbind_live();
    vm_machine_cmos_unbind_live();
    vm_machine_fdd_unbind_live();
    vm_machine_fdc_unbind_live();
    vm_machine_hdd_unbind_live();
    vm_machine_debug_unbind_live();
    vm_profile_default_bios_unbind_live();
    vm_profile_default_qdx_unbind_live();
    core_machine_cpu_instructions_unbind_live();
    core_machine_cpu_unbind_live();
    if (machine == NULL) return;
    machine->cpu = NULL;
    machine->cpuins = NULL;
    machine->ram = NULL;
    machine->port = NULL;
    machine->pic_master = NULL;
    machine->pic_slave = NULL;
    machine->pit = NULL;
    machine->dma_latch = NULL;
    machine->dma_primary = NULL;
    machine->dma_secondary = NULL;
    machine->kbc = NULL;
    machine->vadp = NULL;
    machine->cmos = NULL;
    machine->fdd = NULL;
    machine->fdc = NULL;
    machine->hdd = NULL;
    machine->debug = NULL;
    machine->default_bios = NULL;
    machine->default_qdx = NULL;
}
