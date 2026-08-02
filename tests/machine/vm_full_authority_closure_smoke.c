#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/dma.h"
#include "core/machine/kbc.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"
#include "vm/machine/cmos.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdc.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "vm/profile/default_profile/firmware/bios.h"

static int has_single_live_authority(const vm_composition_live_machine *machine)
{
    return machine != NULL &&
        machine->cpu == &machine->cpu_storage &&
        machine->cpuins == &machine->cpuins_storage &&
        machine->ram == &machine->ram_storage &&
        machine->port == &machine->port_storage &&
        machine->pic_master == &machine->pic_master_storage &&
        machine->pic_slave == &machine->pic_slave_storage &&
        machine->pit == &machine->pit_storage &&
        machine->dma_latch == &machine->dma_latch_storage &&
        machine->dma_primary == &machine->dma_primary_storage &&
        machine->dma_secondary == &machine->dma_secondary_storage &&
        machine->kbc == &machine->kbc_storage &&
        machine->vadp == &machine->vadp_storage &&
        machine->cmos == &machine->cmos_storage &&
        machine->fdd == &machine->fdd_storage &&
        machine->fdc == &machine->fdc_storage &&
        machine->hdd == &machine->hdd_storage &&
        machine->debug == &machine->debug_storage &&
        machine->default_bios == &machine->default_bios_storage &&
        machine->default_qdx == &machine->default_qdx_storage &&
        machine->default_profile_context != NULL &&
        machine->default_profile_context->qdx == machine->default_qdx &&
        core_machine_cpu_execution_context_extension(machine->cpu_execution) ==
            machine->default_profile_context &&
        machine->control != NULL &&
        machine->control->execution_context.device == machine;
}
static DWORD WINAPI run_full_pc(void *opaque)
{
    vm_composition_control_start((vm_composition_control_state *)opaque);
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;
    vm_composition_live_machine *session;

    if (argc != 2) return 1;
    session = (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_live_machine_initialize(session);
    vm_composition_control_initialize(session->control, session);
    if (!has_single_live_authority(session) ||
        vm_machine_fdd_insert_for(session->fdd, argv[1]) != 0) {
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_control_reset(session->control);
    thread = CreateThread(NULL, 0u, run_full_pc, session->control, 0u, NULL);
    if (thread == NULL) {
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    Sleep(10u);
    if (!vm_composition_control_is_running(session->control) || !has_single_live_authority(session)) {
        vm_composition_control_stop(session->control);
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_control_reset(session->control);
    Sleep(10u);
    vm_composition_control_stop(session->control);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    if (result != WAIT_OBJECT_0 || !has_single_live_authority(session)) {
        vm_composition_control_finalize(session->control, session);
        vm_composition_live_machine_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_control_finalize(session->control, session);
    vm_composition_live_machine_finalize(session);
    free(session);
    puts("M5:T44:S1:FULL-AUTHORITY-CLOSURE:OK");
    return 0;
}
