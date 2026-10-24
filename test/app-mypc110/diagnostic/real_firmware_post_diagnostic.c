#include "app-mypc110/pc110_plan_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/waiting.h"

extern const x86_nxvm_machine_assets vm_app_firmware;

#define MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS 8000000u
#define MYPC110_POST_DIAGNOSTIC_QUANTUM 256u

int main(void)
{
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u};
    x86_nxvm_machine_construction construction = {0};
    x86_nxvm_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_observation observation = {0};
    lib_u64 executed = 0u;
    lib_u32 processor_reset_count = 0u;
    lib_bool advanced = LIB_FALSE;
    lib_u32 wait_count = 0u;
    lib_status status;

    status = mypc110_machine_construction_create(&config, &vm_app_firmware, &construction);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_create(&config, &construction, &machine);
    if (status == LIB_STATUS_OK) {
        x86_nxvm_machine_executor_state_start(&machine->control.state);
    }
    while (status == LIB_STATUS_OK && executed < MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS) {
        status = x86_core_machine_run(machine->x86_core_machine,
            (x86_core_machine_run_budget) {.instructions = MYPC110_POST_DIAGNOSTIC_QUANTUM,
                .ticks = 0u}, &result);
        executed += result.executed;
        if (status != LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) break;
        if (result.reason == X86_CORE_MACHINE_STOP_RESET_REQUESTED) {
            ++processor_reset_count;
            continue;
        }
        if (result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            ++wait_count;
            status = x86_nxvm_machine_waiting_advance(machine, &result, &advanced);
            if (status != LIB_STATUS_OK || !advanced) break;
        } else if (result.reason != X86_CORE_MACHINE_STOP_BUDGET) {
            break;
        }
    }
    if (machine != LIB_NULL) x86_nxvm_machine_control_end_run(&machine->control);
    if (machine != LIB_NULL)
        (void)x86_core_machine_capture_observation(machine->x86_core_machine, &observation);
    lib_c_printf("PC110:POST:status=%d reason=%u pc=%08x detail=%08x executed=%llu resets=%u waits=%u advanced=%u exceptions=%u first=%08x/%08x\n",
        (int)status, (lib_u32)result.reason, result.linear_pc, result.detail,
        (unsigned long long)executed, processor_reset_count, wait_count, advanced,
        observation.diagnostic.delivered_exception_count,
        observation.diagnostic.first_delivered_exception.exception_mask,
        observation.diagnostic.first_delivered_exception.point.linear_pc);
    x86_nxvm_machine_destroy(machine);
    return status == LIB_STATUS_OK &&
        executed >= MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS &&
        processor_reset_count != 0u ? 0 : 1;
}
