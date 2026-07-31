#include "adapters/nxvm_baseline/cpu_probe.h"

#include <string.h>

#include "nxvm-baseline/device/device.h"
#include "nxvm-baseline/device/vcpu.h"
#include "nxvm-baseline/device/vcpuins.h"

static int nxvm_baseline_cpu_probe_active;

static void nxvm_baseline_cpu_probe_capture_state(
    nxvm_baseline_cpu_probe_state *state)
{
    state->cs = *(const uint16_t *)deviceConnectCpuGetRefCS();
    state->ip = *(const uint16_t *)deviceConnectCpuGetRefIP();
    state->linear_pc = deviceConnectCpuGetCsBase() +
        *(const uint32_t *)deviceConnectCpuGetRefEIP();
    state->eax = *(const uint32_t *)deviceConnectCpuGetRefEAX();
    state->ebx = *(const uint32_t *)deviceConnectCpuGetRefEBX();
    state->ecx = *(const uint32_t *)deviceConnectCpuGetRefECX();
    state->edx = *(const uint32_t *)deviceConnectCpuGetRefEDX();
    state->eflags = *(const uint32_t *)deviceConnectCpuGetRefEFLAGS();
}

static int nxvm_baseline_cpu_probe_reset(void)
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

int nxvm_baseline_cpu_probe_begin(void)
{
    if (nxvm_baseline_cpu_probe_active) {
        return 0;
    }
    deviceInit();
    nxvm_baseline_cpu_probe_active = 1;
    if (!nxvm_baseline_cpu_probe_reset()) {
        nxvm_baseline_cpu_probe_end();
        return 0;
    }
    return 1;
}

int nxvm_baseline_cpu_probe_step(
    const uint8_t *bytes,
    size_t byte_count,
    nxvm_baseline_cpu_probe_capture *out_capture)
{
    if (!nxvm_baseline_cpu_probe_active || bytes == NULL || out_capture == NULL ||
        byte_count == 0u || byte_count > NXVM_BASELINE_CPU_PROBE_MAX_BYTES ||
        !nxvm_baseline_cpu_probe_reset()) {
        return 0;
    }

    memset(out_capture, 0, sizeof(*out_capture));
    memcpy(out_capture->bytes, bytes, byte_count);
    out_capture->byte_count = byte_count;
    deviceConnectRamRealWrite(0u, 0u, (void *)bytes, byte_count);
    nxvm_baseline_cpu_probe_capture_state(&out_capture->before);
    vcpuRefresh();
    nxvm_baseline_cpu_probe_capture_state(&out_capture->after);
    out_capture->exception_mask = vcpuins.data.except;
    out_capture->exception_code = vcpuins.data.excode;
    return 1;
}

void nxvm_baseline_cpu_probe_end(void)
{
    if (nxvm_baseline_cpu_probe_active) {
        deviceFinal();
        nxvm_baseline_cpu_probe_active = 0;
    }
}
