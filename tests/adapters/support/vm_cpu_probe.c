#include "type.h"

#include "adapters/support/vm_cpu_probe.h"





#include "core/machine/cpu.h"

#include "core/machine/cpu.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/memory.h"

#include "vm/composition/session/execution.h"

#include "vm/composition/session/control.h"

#include "core/machine/cpu_instructions.h"

#include "vm/composition/session/session.h"

struct test_vm_cpu_probe {
    C_INT active;
    vm_session machine;
};

static C_INT vm_session_cpu_probe_capture_state(const test_vm_cpu_probe *probe,
    vm_session_cpu_probe_state *state)
{
    const t_cpu *cpu = probe == STD_NULL ? STD_NULL :
        core_machine_configuration_cpu_borrow(probe->machine.core_machine);

    if (cpu == STD_NULL) {
        return 0;
    }
    state->cs = cpu->data.cs.selector;
    state->ip = cpu->data.ip;
    state->linear_pc = cpu->data.cs.base + cpu->data.eip;
    state->eax = cpu->data.eax;
    state->ebx = cpu->data.ebx;
    state->ecx = cpu->data.ecx;
    state->edx = cpu->data.edx;
    state->eflags = cpu->data.eflags;
    return 1;
}

static C_INT vm_session_cpu_probe_reset(test_vm_cpu_probe *probe)
{
    uint32_t eip = 0u;

    vm_session_control_reset(probe->machine.control);
    t_cpu *cpu = core_machine_configuration_cpu_borrow(probe->machine.core_machine);
    core_machine_cpu_execution_context *execution =
        core_machine_configuration_cpu_execution_borrow(probe->machine.core_machine);

    if (core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.es, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ss, 0u)) {
        return 0;
    }
    STD_MEMCPY(&cpu->data.eip, &eip, sizeof(eip));
    return 1;
}

C_INT vm_session_cpu_probe_create(test_vm_cpu_probe **out_probe)
{
    test_vm_cpu_probe *probe;

    if (out_probe == STD_NULL) {
        return 0;
    }
    *out_probe = STD_NULL;
    probe = (test_vm_cpu_probe *)STD_CALLOC(1u, sizeof(*probe));
    if (probe == STD_NULL) return 0;
    vm_session_storage_initialize(&probe->machine);
    vm_session_control_initialize(probe->machine.control, &probe->machine);
    probe->active = 1;
    if (!vm_session_cpu_probe_reset(probe)) {
        vm_session_cpu_probe_destroy(probe);
        return 0;
    }
    *out_probe = probe;
    return 1;
}

C_INT vm_session_cpu_probe_step(
    test_vm_cpu_probe *probe,
    const uint8_t *bytes,
    STD_SIZE_T byte_count,
    vm_session_cpu_probe_capture *out_capture)
{
    if (probe == STD_NULL || !probe->active || bytes == STD_NULL || out_capture == STD_NULL ||
        byte_count == 0u || byte_count > TEST_VM_CPU_PROBE_MAX_BYTES ||
        !vm_session_cpu_probe_reset(probe)) {
        return 0;
    }

    STD_MEMSET(out_capture, 0, sizeof(*out_capture));
    STD_MEMCPY(out_capture->bytes, bytes, byte_count);
    out_capture->byte_count = byte_count;
    core_machine_memory_write_real_to(
        core_machine_configuration_memory_borrow(probe->machine.core_machine), 0u, 0u, bytes,
        byte_count);
    if (!vm_session_cpu_probe_capture_state(probe, &out_capture->before)) {
        return 0;
    }
    {
        core_machine_run_budget budget = {1u, 0u};
        core_machine_run_result result;

        if (core_machine_run(probe->machine.core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.executed != 1u) {
            return 0;
        }
    }
    if (!vm_session_cpu_probe_capture_state(probe, &out_capture->after)) {
        return 0;
    }
    out_capture->exception_mask =
        core_machine_configuration_cpu_instructions_borrow(probe->machine.core_machine)->data.except;
    out_capture->exception_code =
        core_machine_configuration_cpu_instructions_borrow(probe->machine.core_machine)->data.excode;
    return 1;
}

C_VOID vm_session_cpu_probe_destroy(test_vm_cpu_probe *probe)
{
    if (probe != STD_NULL && probe->active) {
        vm_session_control_finalize(probe->machine.control, &probe->machine);
        vm_session_storage_finalize(&probe->machine);
        probe->active = 0;
    }
    STD_FREE(probe);
}
