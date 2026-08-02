#include "vm/composition_live_machine.h"

static vm_composition_live_machine vmCompositionLiveMachine;
static int vmCompositionLiveMachineBound;

void vm_composition_live_machine_bind(void)
{
    vmCompositionLiveMachine.cpu = &vmCompositionLiveMachine.cpu_storage;
    vmCompositionLiveMachine.cpuins =
        &vmCompositionLiveMachine.cpuins_storage;
    vmCompositionLiveMachine.ram = &vmCompositionLiveMachine.ram_storage;
    vmCompositionLiveMachine.port = &vmCompositionLiveMachine.port_storage;
    vmCompositionLiveMachine.pic_master =
        &vmCompositionLiveMachine.pic_master_storage;
    vmCompositionLiveMachine.pic_slave =
        &vmCompositionLiveMachine.pic_slave_storage;
    vmCompositionLiveMachine.pit = &vmCompositionLiveMachine.pit_storage;
    vmCompositionLiveMachine.dma_latch = &vmCompositionLiveMachine.dma_latch_storage;
    vmCompositionLiveMachine.dma_primary = &vmCompositionLiveMachine.dma_primary_storage;
    vmCompositionLiveMachine.dma_secondary = &vmCompositionLiveMachine.dma_secondary_storage;
    vmCompositionLiveMachine.keyboard_controller = &vmCompositionLiveMachine.keyboard_controller_storage;
    vmCompositionLiveMachine.video_adapter = &vmCompositionLiveMachine.video_adapter_storage;
    vmCompositionLiveMachine.cmos = &vmCompositionLiveMachine.cmos_storage;
    vmCompositionLiveMachine.fdd = &vmCompositionLiveMachine.fdd_storage;
    vmCompositionLiveMachine.fdc = &vmCompositionLiveMachine.fdc_storage;
    vmCompositionLiveMachine.hdd = &vmCompositionLiveMachine.hdd_storage;
    vmCompositionLiveMachine.default_bios =
        &vmCompositionLiveMachine.default_bios_storage;
    core_machine_cpu_bind_live(vmCompositionLiveMachine.cpu);
    core_machine_cpu_instructions_bind_live(vmCompositionLiveMachine.cpuins);
    core_machine_memory_bind_live(vmCompositionLiveMachine.ram);
    core_machine_port_bind_live(vmCompositionLiveMachine.port);
    core_machine_pic_bind_live(vmCompositionLiveMachine.pic_master,
        vmCompositionLiveMachine.pic_slave);
    core_machine_pit_bind_live(vmCompositionLiveMachine.pit);
    core_machine_dma_bind_live(vmCompositionLiveMachine.dma_latch,
        vmCompositionLiveMachine.dma_primary, vmCompositionLiveMachine.dma_secondary);
    core_machine_keyboard_controller_bind_live(vmCompositionLiveMachine.keyboard_controller);
    core_machine_video_adapter_bind_live(vmCompositionLiveMachine.video_adapter);
    vm_machine_cmos_bind_live(vmCompositionLiveMachine.cmos);
    vm_machine_fdd_bind_live(vmCompositionLiveMachine.fdd);
    vm_machine_fdc_bind_live(vmCompositionLiveMachine.fdc);
    vm_machine_hdd_bind_live(vmCompositionLiveMachine.hdd);
    vm_profile_default_bios_bind_live(vmCompositionLiveMachine.default_bios);
    vmCompositionLiveMachineBound = 1;
}

void vm_composition_live_machine_clear(void)
{
    core_machine_memory_unbind_live();
    core_machine_port_unbind_live();
    core_machine_pic_unbind_live();
    core_machine_pit_unbind_live();
    core_machine_dma_unbind_live();
    core_machine_keyboard_controller_unbind_live();
    core_machine_video_adapter_unbind_live();
    vm_machine_cmos_unbind_live();
    vm_machine_fdd_unbind_live();
    vm_machine_fdc_unbind_live();
    vm_machine_hdd_unbind_live();
    vm_profile_default_bios_unbind_live();
    core_machine_cpu_instructions_unbind_live();
    core_machine_cpu_unbind_live();
    vmCompositionLiveMachine.cpu = NULL;
    vmCompositionLiveMachine.cpuins = NULL;
    vmCompositionLiveMachine.ram = NULL;
    vmCompositionLiveMachine.port = NULL;
    vmCompositionLiveMachine.pic_master = NULL;
    vmCompositionLiveMachine.pic_slave = NULL;
    vmCompositionLiveMachine.pit = NULL;
    vmCompositionLiveMachine.dma_latch = NULL;
    vmCompositionLiveMachine.dma_primary = NULL;
    vmCompositionLiveMachine.dma_secondary = NULL;
    vmCompositionLiveMachine.keyboard_controller = NULL;
    vmCompositionLiveMachine.video_adapter = NULL;
    vmCompositionLiveMachine.cmos = NULL;
    vmCompositionLiveMachine.fdd = NULL;
    vmCompositionLiveMachine.fdc = NULL;
    vmCompositionLiveMachine.hdd = NULL;
    vmCompositionLiveMachine.default_bios = NULL;
    vmCompositionLiveMachineBound = 0;
}

const vm_composition_live_machine *vm_composition_live_machine_current(void)
{
    return vmCompositionLiveMachineBound ? &vmCompositionLiveMachine : NULL;
}
