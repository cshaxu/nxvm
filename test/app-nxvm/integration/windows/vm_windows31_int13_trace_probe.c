#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/waiting.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_T287_TRACE_BUDGET 2000000u

lib_i32 main(lib_i32 argc, char **argv)
{
    const core_machine_run_budget budget = {1u, 0u};
    integration_ini_session ini_session;
    core_machine_run_result result;
    vm_machine *session = LIB_NULL;
    lib_u16 int13[2] = {0};
    lib_u32 int13_linear = 0u;
    lib_u32 instruction;
    lib_u8 active_ah = 0u;
    lib_u8 active_dl = 0u;
    lib_u8 opcode;
    lib_u32 read_count = 0u;
    lib_u32 hdd_calls = 0u;
    lib_u32 hdd_returns = 0u;
    lib_i32 geometry_ok = 0;
    lib_i32 int13_ready = 0;
    lib_i32 active = 0;
    lib_i32 passed = 0;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    for (instruction = 0u; instruction < VM_T287_TRACE_BUDGET; ++instruction) {
        t_cpu *cpu = &session->core_machine->executor_cpu;
        lib_i32 returning = 0;

        if (!int13_ready && core_machine_memory_read(session->core_machine,
                0x004cu, int13, sizeof(int13)) == LIB_STATUS_OK &&
            int13[0] != 0u && int13[1] != 0u) {
            int13_linear = (lib_u32)int13[1] * 16u + int13[0];
            int13_ready = 1;
        }
        if (int13_ready && !active && core_machine_linear_pc(session->core_machine) ==
            int13_linear) {
            active_ah = (lib_u8)(cpu->data.eax >> 8u);
            active_dl = (lib_u8)cpu->data.edx;
            active = active_dl >= 0x80u;
            if (active) ++hdd_calls;
        }
        if (active) {
            if (core_machine_memory_read(session->core_machine,
                    cpu->data.cs.base + cpu->data.eip, &opcode,
                    sizeof(opcode)) != LIB_STATUS_OK) {
                break;
            }
            returning = opcode == 0xcfu;
        }
        if (core_machine_run(session->core_machine, budget, &result) !=
                LIB_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            break;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) != LIB_STATUS_OK ||
                !advanced) break;
        }
        cpu = &session->core_machine->executor_cpu;
        if (active && returning) {
            ++hdd_returns;
            printf("M5:T287:S18:INT13 ah=%02X dl=%02X cf=%u ax=%04X "
                "cx=%04X dx=%04X\n", active_ah, active_dl,
                cpu->data.eflags & 1u, (lib_u16)cpu->data.eax,
                (lib_u16)cpu->data.ecx, (lib_u16)cpu->data.edx);
            if (active_ah == 0x08u && (cpu->data.eflags & 1u) == 0u &&
                ((lib_u16)cpu->data.ecx & 0x003fu) != 0u &&
                (lib_u8)(cpu->data.edx >> 8u) != 0u) {
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
        printf("M5:T287:S18:INT13:OK reads=%u\n", read_count);
    } else {
        fprintf(stderr,
            "M5:T287:S18:INT13:FAIL ready=%d calls=%u returns=%u geometry=%d reads=%u\n",
            int13_ready, hdd_calls, hdd_returns, geometry_ok, read_count);
        fprintf(stderr, "M5:T287:S18:INT13:VECTOR=%04X:%04X linear=%05X pc=%05X\n",
            int13[1], int13[0], int13_linear,
            core_machine_linear_pc(session->core_machine));
    }

    integration_ini_session_close(&ini_session);
    return passed ? 0 : 1;
}
