#include <windows.h>

#include <stdio.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/dma.h"
#include "core/machine/keyboard_controller.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/port.h"
#include "core/machine/video_adapter.h"
#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"
#include "vm/machine/vcmos.h"
#include "vm/machine/vdebug.h"
#include "vm/machine/vfdc.h"
#include "vm/machine/vfdd.h"
#include "vm/machine/vhdd.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "vm/profile/default_profile/firmware/vbios.h"

static int has_single_live_authority(void)
{
    const vm_composition_live_machine *machine =
        vm_composition_live_machine_current();

    return machine != NULL &&
        machine->cpu == &vcpu && machine->cpuins == &vcpuins &&
        machine->ram == &vram && machine->port == &vport &&
        machine->pic_master == &vpic1 && machine->pic_slave == &vpic2 &&
        machine->pit == &vpit && machine->dma_latch == &vlatch &&
        machine->dma_primary == &vdma1 && machine->dma_secondary == &vdma2 &&
        machine->keyboard_controller == &vkbc &&
        machine->video_adapter == &vvadp && machine->cmos == &vcmos &&
        machine->fdd == &vfdd && machine->fdc == &vfdc &&
        machine->hdd == &vhdd && machine->debug == &vdebug &&
        machine->default_bios == &vbios &&
        machine->default_qdx->table == qdxTable;
}

static DWORD WINAPI run_full_pc(void *opaque)
{
    (void)opaque;
    vm_composition_control_start();
    return 0u;
}

int main(int argc, char **argv)
{
    HANDLE thread;
    DWORD result;

    if (argc != 2) return 1;
    vm_composition_control_initialize();
    if (!has_single_live_authority() || vm_machine_fdd_insert(argv[1]) != 0) {
        vm_composition_control_finalize();
        return 1;
    }
    vm_composition_control_reset();
    thread = CreateThread(NULL, 0u, run_full_pc, NULL, 0u, NULL);
    if (thread == NULL) {
        vm_composition_control_finalize();
        return 1;
    }
    Sleep(10u);
    if (!vm_composition_control_is_running() || !has_single_live_authority()) {
        vm_composition_control_stop();
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
        vm_composition_control_finalize();
        return 1;
    }
    vm_composition_control_reset();
    Sleep(10u);
    vm_composition_control_stop();
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    if (result != WAIT_OBJECT_0 || !has_single_live_authority()) {
        vm_composition_control_finalize();
        return 1;
    }
    vm_composition_control_finalize();
    if (vm_composition_live_machine_current() != NULL ||
        core_machine_cpu_current() != NULL ||
        vm_machine_debug_current() != NULL ||
        vm_profile_default_bios_current() != NULL ||
        vm_profile_default_qdx_current() != NULL) return 1;
    puts("M5:T44:S1:FULL-AUTHORITY-CLOSURE:OK");
    return 0;
}
