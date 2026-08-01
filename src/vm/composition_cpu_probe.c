#include "vm/composition_cpu_probe.h"

#include <string.h>

#include "core/machine/vcpu.h"
#include "core/product/runtime/execution_context.h"
#include "vm/machine/device.h"
#include "core/machine/vcpuins.h"

static int nxvm_cpu_probe_active;

static int nxvm_cpu_probe_capture_state(
    nxvm_cpu_probe_state *state)
{
    const nxvm_execution_context *context =
        nxvm_execution_context_current();
    const t_cpu *cpu = (const t_cpu *)nxvm_execution_context_cpu(context);

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

static int nxvm_cpu_probe_reset(void)
{
    uint32_t eip = 0u;

    deviceReset();
    if (deviceConnectCpuLoadCS(0u) || deviceConnectCpuLoadDS(0u) ||
        deviceConnectCpuLoadES(0u) || deviceConnectCpuLoadSS(0u)) {
        return 0;
    }
    memcpy(deviceConnectCpuGetRefEIP(), &eip, sizeof(eip));
    return 1;
}

int nxvm_cpu_probe_begin(void)
{
    if (nxvm_cpu_probe_active) {
        return 0;
    }
    deviceInit();
    nxvm_cpu_probe_active = 1;
    if (!nxvm_cpu_probe_reset()) {
        nxvm_cpu_probe_end();
        return 0;
    }
    return 1;
}

int nxvm_cpu_probe_step(
    const uint8_t *bytes,
    size_t byte_count,
    nxvm_cpu_probe_capture *out_capture)
{
    if (!nxvm_cpu_probe_active || bytes == NULL || out_capture == NULL ||
        byte_count == 0u || byte_count > NXVM_BASELINE_CPU_PROBE_MAX_BYTES ||
        !nxvm_cpu_probe_reset()) {
        return 0;
    }

    memset(out_capture, 0, sizeof(*out_capture));
    memcpy(out_capture->bytes, bytes, byte_count);
    out_capture->byte_count = byte_count;
    deviceConnectRamRealWrite(0u, 0u, (void *)bytes, byte_count);
    if (!nxvm_cpu_probe_capture_state(&out_capture->before)) {
        return 0;
    }
    vcpuRefresh();
    if (!nxvm_cpu_probe_capture_state(&out_capture->after)) {
        return 0;
    }
    out_capture->exception_mask = vcpuins.data.except;
    out_capture->exception_code = vcpuins.data.excode;
    return 1;
}

void nxvm_cpu_probe_end(void)
{
    if (nxvm_cpu_probe_active) {
        deviceFinal();
        nxvm_cpu_probe_active = 0;
    }
}
