#include "vm/composition_live_machine.h"

#include "core/machine/machine.h"

#include <stdlib.h>

void vm_composition_live_machine_initialize(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    {
        core_machine_config config = {0};

        config.profile = CORE_MACHINE_PROFILE_CUSTOM;
        if (core_machine_create(&config, &machine->core_machine) !=
                NXVM_CORE_STATUS_OK ||
            core_machine_enable_legacy_executor(machine->core_machine) !=
                NXVM_CORE_STATUS_OK) {
            core_machine_destroy(machine->core_machine);
            machine->core_machine = NULL;
            return;
        }
    }
    machine->ram = core_machine_legacy_memory_borrow(machine->core_machine);
    machine->port = core_machine_legacy_port_borrow(machine->core_machine);
    machine->cpu = core_machine_legacy_cpu_borrow(machine->core_machine);
    machine->cpuins = core_machine_legacy_cpu_instructions_borrow(
        machine->core_machine);
    machine->cpu_execution = core_machine_legacy_cpu_execution_borrow(
        machine->core_machine);
    machine->pic_master = &machine->pic_master_storage;
    machine->pic_slave = &machine->pic_slave_storage;
    core_machine_cpu_execution_context_bind_pic(machine->cpu_execution,
        machine->pic_master, machine->pic_slave);
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
    machine->default_profile_context = &machine->default_profile_context_storage;
    vm_profile_default_context_initialize(machine->default_profile_context,
        machine->default_bios, machine->default_qdx, machine->ram,
        machine->vadp, NULL, NULL);
    core_machine_cpu_execution_context_bind_extension(machine->cpu_execution,
        machine->default_profile_context);
    machine->default_profile_context->execution = machine->cpu_execution;
    machine->block_provider = &machine->block_provider_storage;
    core_machine_block_provider_slot_initialize(machine->block_provider);
    machine->default_profile_context->block_provider = machine->block_provider;
    machine->keyboard_provider = &machine->keyboard_provider_storage;
    core_machine_keyboard_provider_slot_initialize(machine->keyboard_provider);
    machine->default_profile_context->keyboard_provider = machine->keyboard_provider;
    machine->display_provider = &machine->display_provider_storage;
    core_machine_display_provider_slot_initialize(machine->display_provider);
    machine->default_profile_context->display_provider = machine->display_provider;
    machine->presentation_mailbox = &machine->presentation_mailbox_storage;
    vm_platform_presentation_mailbox_initialize(machine->presentation_mailbox);
    machine->execution_transport = &machine->execution_transport_storage;
    machine->keyboard_transport = &machine->keyboard_transport_storage;
    machine->wait_scope = &machine->wait_scope_storage;
    machine->platform_run_context = &machine->platform_run_context_storage;
    machine->display_generation = 0u;
    machine->control = (vm_composition_control_state *)calloc(1u,
        sizeof(*machine->control));
}

void vm_composition_live_machine_finalize(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    machine->cpu = NULL;
    machine->cpuins = NULL;
    machine->cpu_execution = NULL;
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
    core_machine_cpu_execution_context_bind_extension(machine->cpu_execution, NULL);
    machine->default_profile_context = NULL;
    core_machine_block_provider_slot_finalize(machine->block_provider);
    machine->block_provider = NULL;
    core_machine_keyboard_provider_slot_finalize(machine->keyboard_provider);
    machine->keyboard_provider = NULL;
    core_machine_display_provider_slot_finalize(machine->display_provider);
    machine->display_provider = NULL;
    machine->presentation_mailbox = NULL;
    machine->execution_transport = NULL;
    machine->keyboard_transport = NULL;
    machine->wait_scope = NULL;
    machine->platform_run_context = NULL;
    free(machine->control);
    machine->control = NULL;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = NULL;
}
