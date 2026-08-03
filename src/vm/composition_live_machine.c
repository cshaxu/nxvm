#include "type.h"

#include "vm/composition_live_machine.h"


#include "core/machine/machine.h"


#include <stdlib.h>

C_VOID vm_composition_live_machine_initialize(vm_composition_live_machine *machine)
{
    if (machine == STD_NULL || machine->core_machine != STD_NULL) return;
    {
        core_machine_config config = {0};

        config.profile = CORE_MACHINE_PROFILE_CUSTOM;
        if (core_machine_create(&config, &machine->core_machine) !=
                NTVDM64_STATUS_OK ||
            core_machine_enable_executor(machine->core_machine) !=
                NTVDM64_STATUS_OK) {
            core_machine_destroy(machine->core_machine);
            machine->core_machine = STD_NULL;
            return;
        }
    }
    machine->ram = core_machine_executor_memory_borrow(machine->core_machine);
    machine->port = core_machine_executor_port_borrow(machine->core_machine);
    machine->cpu = core_machine_executor_cpu_borrow(machine->core_machine);
    machine->cpuins = core_machine_executor_cpu_instructions_borrow(
        machine->core_machine);
    machine->cpu_execution = core_machine_executor_cpu_execution_borrow(
        machine->core_machine);
    if (core_machine_enable_shared_devices(machine->core_machine) !=
        NTVDM64_STATUS_OK) {
        core_machine_destroy(machine->core_machine);
        machine->core_machine = STD_NULL;
        return;
    }
    machine->pic_master = core_machine_shared_pic_master_borrow(
        machine->core_machine);
    machine->pic_slave = core_machine_shared_pic_slave_borrow(
        machine->core_machine);
    core_machine_cpu_execution_context_bind_pic(machine->cpu_execution,
        machine->pic_master, machine->pic_slave);
    machine->pit = core_machine_shared_pit_borrow(machine->core_machine);
    machine->dma_latch = core_machine_shared_dma_latch_borrow(
        machine->core_machine);
    machine->dma_primary = core_machine_shared_dma_primary_borrow(
        machine->core_machine);
    machine->dma_secondary = core_machine_shared_dma_secondary_borrow(
        machine->core_machine);
    machine->kbc = core_machine_shared_kbc_borrow(machine->core_machine);
    machine->vadp = core_machine_shared_vadp_borrow(machine->core_machine);
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
        machine->vadp, STD_NULL, STD_NULL);
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
    machine->debugger_context = &machine->debugger_context_storage;
    core_product_debug_context_initialize(machine->debugger_context);
    machine->console_context = &machine->console_context_storage;
    nxvm_product_console_context_initialize(machine->console_context);
    machine->console_target = &machine->console_target_storage;
    machine->display_generation = 0u;
    machine->control = (vm_composition_control_state *)STD_CALLOC(1u,
        sizeof(*machine->control));
}

C_VOID vm_composition_live_machine_finalize(vm_composition_live_machine *machine)
{
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    core_machine_cpu_execution_context_bind_extension(machine->cpu_execution, STD_NULL);
    machine->cpu = STD_NULL;
    machine->cpuins = STD_NULL;
    machine->cpu_execution = STD_NULL;
    machine->ram = STD_NULL;
    machine->port = STD_NULL;
    machine->pic_master = STD_NULL;
    machine->pic_slave = STD_NULL;
    machine->pit = STD_NULL;
    machine->dma_latch = STD_NULL;
    machine->dma_primary = STD_NULL;
    machine->dma_secondary = STD_NULL;
    machine->kbc = STD_NULL;
    machine->vadp = STD_NULL;
    machine->cmos = STD_NULL;
    machine->fdd = STD_NULL;
    machine->fdc = STD_NULL;
    machine->hdd = STD_NULL;
    machine->debug = STD_NULL;
    machine->default_bios = STD_NULL;
    machine->default_qdx = STD_NULL;
    machine->default_profile_context = STD_NULL;
    core_machine_block_provider_slot_finalize(machine->block_provider);
    machine->block_provider = STD_NULL;
    core_machine_keyboard_provider_slot_finalize(machine->keyboard_provider);
    machine->keyboard_provider = STD_NULL;
    core_machine_display_provider_slot_finalize(machine->display_provider);
    machine->display_provider = STD_NULL;
    machine->presentation_mailbox = STD_NULL;
    machine->execution_transport = STD_NULL;
    machine->keyboard_transport = STD_NULL;
    machine->wait_scope = STD_NULL;
    machine->platform_run_context = STD_NULL;
    machine->debugger_context = STD_NULL;
    machine->console_context = STD_NULL;
    machine->console_target = STD_NULL;
    STD_FREE(machine->control);
    machine->control = STD_NULL;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
}
