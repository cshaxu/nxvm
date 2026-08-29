#include "type.h"

#include "adapters/support/vm_cpu_probe.h"
#include "../../../../core/support/core_machine_cpu_fixture.h"





#include "core/machine/cpu.h"

#include "core/machine/cpu.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/memory.h"

#include "vm/composition/session/execution.h"

#include "vm/composition/session/control.h"

#include "core/machine/cpu_instructions.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"

struct test_vm_cpu_probe {
    C_INT active;
    vm_session machine;
};

static C_INT vm_session_cpu_probe_capture_state(const test_vm_cpu_probe *probe,
    vm_session_cpu_probe_state *state)
{
    t_cpu cpu;

    if (probe == STD_NULL) {
        return 0;
    }
    cpu = test_core_machine_fixture_capture_cpu_after_run(probe->machine.core_machine);
    state->cs = cpu.data.cs.selector;
    state->ip = cpu.data.ip;
    state->linear_pc = cpu.data.cs.base + cpu.data.eip;
    state->eax = cpu.data.eax;
    state->ebx = cpu.data.ebx;
    state->ecx = cpu.data.ecx;
    state->edx = cpu.data.edx;
    state->eflags = cpu.data.eflags;
    return 1;
}

static C_INT vm_session_cpu_probe_reset(test_vm_cpu_probe *probe)
{
    vm_session_control_reset(&probe->machine.control);
    return test_core_machine_fixture_reset_real_mode(probe->machine.core_machine);
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
    vm_session_control_initialize(&probe->machine.control, &probe->machine);
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
    const type_unsigned_8 *bytes,
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
    if (core_machine_memory_write(probe->machine.core_machine, 0u, bytes,
            byte_count) != TYPE_STATUS_OK) return 0;
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
    return test_core_machine_fixture_capture_instruction_exception(
        probe->machine.core_machine, &out_capture->exception_mask,
        &out_capture->exception_code);
}

C_VOID vm_session_cpu_probe_destroy(test_vm_cpu_probe *probe)
{
    if (probe != STD_NULL && probe->active) {
        vm_session_control_finalize(&probe->machine.control, &probe->machine);
        vm_session_storage_finalize(&probe->machine);
        probe->active = 0;
    }
    STD_FREE(probe);
}
