#include "vm/composition_cpu_probe.h"

#include <stdlib.h>
#include <string.h>

#include "core/machine/cpu.h"
#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/product/runtime/execution_context.h"
#include "vm/composition_control.h"
#include "core/machine/cpu_instructions.h"
#include "vm/composition_live_machine.h"

struct nxvm_cpu_probe {
    int active;
    vm_composition_live_machine machine;
};

static int nxvm_cpu_probe_capture_state(const nxvm_cpu_probe *probe,
    nxvm_cpu_probe_state *state)
{
    const t_cpu *cpu = probe == NULL ? NULL : probe->machine.cpu;

    if (cpu == NULL) {
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

static int nxvm_cpu_probe_reset(nxvm_cpu_probe *probe)
{
    uint32_t eip = 0u;

    vm_composition_control_reset(probe->machine.control);
    if (core_machine_cpu_execution_load_segment(probe->machine.cpu_execution,
            &probe->machine.cpu->data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(probe->machine.cpu_execution,
            &probe->machine.cpu->data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(probe->machine.cpu_execution,
            &probe->machine.cpu->data.es, 0u) ||
        core_machine_cpu_execution_load_segment(probe->machine.cpu_execution,
            &probe->machine.cpu->data.ss, 0u)) {
        return 0;
    }
    memcpy(&probe->machine.cpu->data.eip, &eip, sizeof(eip));
    return 1;
}

int nxvm_cpu_probe_create(nxvm_cpu_probe **out_probe)
{
    nxvm_cpu_probe *probe;

    if (out_probe == NULL) {
        return 0;
    }
    *out_probe = NULL;
    probe = (nxvm_cpu_probe *)calloc(1u, sizeof(*probe));
    if (probe == NULL) return 0;
    vm_composition_live_machine_initialize(&probe->machine);
    vm_composition_control_initialize(probe->machine.control, &probe->machine);
    probe->active = 1;
    if (!nxvm_cpu_probe_reset(probe)) {
        nxvm_cpu_probe_destroy(probe);
        return 0;
    }
    *out_probe = probe;
    return 1;
}

int nxvm_cpu_probe_step(
    nxvm_cpu_probe *probe,
    const uint8_t *bytes,
    size_t byte_count,
    nxvm_cpu_probe_capture *out_capture)
{
    if (probe == NULL || !probe->active || bytes == NULL || out_capture == NULL ||
        byte_count == 0u || byte_count > NXVM_BASELINE_CPU_PROBE_MAX_BYTES ||
        !nxvm_cpu_probe_reset(probe)) {
        return 0;
    }

    memset(out_capture, 0, sizeof(*out_capture));
    memcpy(out_capture->bytes, bytes, byte_count);
    out_capture->byte_count = byte_count;
    core_machine_memory_write_real_to(probe->machine.ram, 0u, 0u, bytes,
        byte_count);
    if (!nxvm_cpu_probe_capture_state(probe, &out_capture->before)) {
        return 0;
    }
    {
        core_machine_run_budget budget = {1u, 0u};
        core_machine_run_result result;

        if (core_machine_run(probe->machine.core_machine, budget, &result) !=
                NTVDM64_STATUS_OK || result.executed != 1u) {
            return 0;
        }
    }
    if (!nxvm_cpu_probe_capture_state(probe, &out_capture->after)) {
        return 0;
    }
    out_capture->exception_mask = probe->machine.cpuins->data.except;
    out_capture->exception_code = probe->machine.cpuins->data.excode;
    return 1;
}

void nxvm_cpu_probe_destroy(nxvm_cpu_probe *probe)
{
    if (probe != NULL && probe->active) {
        vm_composition_control_finalize(probe->machine.control, &probe->machine);
        vm_composition_live_machine_finalize(&probe->machine);
        probe->active = 0;
    }
    free(probe);
}
