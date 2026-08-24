#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_T287_TRACE_BUDGET 2000000u

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_machine_run_budget budget = {1u, 0u};
    const vm_session_config config = {
        .fdd_image = argc == 3 ? argv[1] : STD_NULL,
        .hdd_image = argc == 3 ? argv[2] : STD_NULL,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    type_unsigned_16 int13[2] = {0};
    type_unsigned_32 instruction;
    type_unsigned_8 active_ah = 0u;
    type_unsigned_8 active_dl = 0u;
    type_unsigned_32 read_count = 0u;
    C_INT geometry_ok = 0;
    C_INT active = 0;
    C_INT passed = 0;

    if (argc != 3 || vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || core_machine_memory_read(session->core_machine,
            0x004cu, int13, sizeof(int13)) != TYPE_STATUS_OK) {
        goto done;
    }
    for (instruction = 0u; instruction < VM_T287_TRACE_BUDGET; ++instruction) {
        t_cpu *cpu = &session->core_machine->executor_cpu;

        if (!active && cpu->data.cs.selector == int13[1] &&
            (type_unsigned_16)cpu->data.eip == int13[0]) {
            active_ah = (type_unsigned_8)(cpu->data.eax >> 8u);
            active_dl = (type_unsigned_8)cpu->data.edx;
            active = active_dl >= 0x80u;
        }
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            break;
        }
        cpu = &session->core_machine->executor_cpu;
        if (active && cpu->data.cs.selector != int13[1]) {
            STD_PRINTF("M5:T287:S18:INT13 ah=%02X dl=%02X cf=%u ax=%04X "
                "cx=%04X dx=%04X\n", active_ah, active_dl,
                cpu->data.eflags & 1u, (type_unsigned_16)cpu->data.eax,
                (type_unsigned_16)cpu->data.ecx, (type_unsigned_16)cpu->data.edx);
            if (active_ah == 0x08u && (cpu->data.eflags & 1u) == 0u &&
                ((type_unsigned_16)cpu->data.ecx & 0x003fu) == 63u &&
                ((type_unsigned_16)(cpu->data.edx >> 8u)) == 15u &&
                (type_unsigned_8)cpu->data.edx == 1u) {
                geometry_ok = 1;
            }
            if (active_ah == 0x02u && (cpu->data.eflags & 1u) == 0u) {
                ++read_count;
            }
            active = 0;
            if (geometry_ok && read_count >= 2u) {
                passed = 1;
                break;
            }
        }
    }
    if (passed) {
        STD_PRINTF("M5:T287:S18:INT13:OK reads=%u\n", read_count);
    }

done:
    vm_session_destroy(session);
    return passed ? 0 : 1;
}
