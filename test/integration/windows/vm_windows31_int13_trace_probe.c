#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"
#include "test/integration/support/session_yaml.h"

#define VM_T287_TRACE_BUDGET 2000000u

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_machine_run_budget budget = {1u, 0u};
    integration_yaml_session yaml_session;
    core_machine_run_result result;
    vm_session *session = STD_NULL;
    type_unsigned_16 int13[2] = {0};
    type_unsigned_32 int13_linear = 0u;
    type_unsigned_32 instruction;
    type_unsigned_8 active_ah = 0u;
    type_unsigned_8 active_dl = 0u;
    type_unsigned_8 opcode;
    type_unsigned_32 read_count = 0u;
    type_unsigned_32 hdd_calls = 0u;
    type_unsigned_32 hdd_returns = 0u;
    C_INT geometry_ok = 0;
    C_INT int13_ready = 0;
    C_INT active = 0;
    C_INT passed = 0;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    for (instruction = 0u; instruction < VM_T287_TRACE_BUDGET; ++instruction) {
        t_cpu *cpu = &session->core_machine->executor_cpu;
        C_INT returning = 0;

        if (!int13_ready && core_machine_memory_read(session->core_machine,
                0x004cu, int13, sizeof(int13)) == TYPE_STATUS_OK &&
            int13[0] != 0u && int13[1] != 0u) {
            int13_linear = (type_unsigned_32)int13[1] * 16u + int13[0];
            int13_ready = 1;
        }
        if (int13_ready && !active && core_machine_linear_pc(session->core_machine) ==
            int13_linear) {
            active_ah = (type_unsigned_8)(cpu->data.eax >> 8u);
            active_dl = (type_unsigned_8)cpu->data.edx;
            active = active_dl >= 0x80u;
            if (active) ++hdd_calls;
        }
        if (active) {
            if (core_machine_memory_read(session->core_machine,
                    cpu->data.cs.base + cpu->data.eip, &opcode,
                    sizeof(opcode)) != TYPE_STATUS_OK) {
                break;
            }
            returning = opcode == 0xcfu;
        }
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            break;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            if (vm_session_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
                !advanced) break;
        }
        cpu = &session->core_machine->executor_cpu;
        if (active && returning) {
            ++hdd_returns;
            STD_PRINTF("M5:T287:S18:INT13 ah=%02X dl=%02X cf=%u ax=%04X "
                "cx=%04X dx=%04X\n", active_ah, active_dl,
                cpu->data.eflags & 1u, (type_unsigned_16)cpu->data.eax,
                (type_unsigned_16)cpu->data.ecx, (type_unsigned_16)cpu->data.edx);
            if (active_ah == 0x08u && (cpu->data.eflags & 1u) == 0u &&
                ((type_unsigned_16)cpu->data.ecx & 0x003fu) != 0u &&
                (type_unsigned_8)(cpu->data.edx >> 8u) != 0u) {
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
    } else {
        STD_FPRINTF(STD_STDERR,
            "M5:T287:S18:INT13:FAIL ready=%d calls=%u returns=%u geometry=%d reads=%u\n",
            int13_ready, hdd_calls, hdd_returns, geometry_ok, read_count);
        STD_FPRINTF(STD_STDERR, "M5:T287:S18:INT13:VECTOR=%04X:%04X linear=%05X pc=%05X\n",
            int13[1], int13[0], int13_linear,
            core_machine_linear_pc(session->core_machine));
    }

    integration_yaml_session_close(&yaml_session);
    return passed ? 0 : 1;
}
