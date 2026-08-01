#ifndef NXVM_BASELINE_CPU_PROBE_H
#define NXVM_BASELINE_CPU_PROBE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_BASELINE_CPU_PROBE_MAX_BYTES 15u

typedef struct nxvm_baseline_cpu_probe_state {
    uint16_t cs;
    uint16_t ip;
    uint32_t linear_pc;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t eflags;
} nxvm_baseline_cpu_probe_state;

typedef struct nxvm_baseline_cpu_probe_capture {
    uint8_t bytes[NXVM_BASELINE_CPU_PROBE_MAX_BYTES];
    size_t byte_count;
    nxvm_baseline_cpu_probe_state before;
    nxvm_baseline_cpu_probe_state after;
    uint32_t exception_mask;
    uint32_t exception_code;
} nxvm_baseline_cpu_probe_capture;

int nxvm_baseline_cpu_probe_begin(void);
int nxvm_baseline_cpu_probe_step(
    const uint8_t *bytes,
    size_t byte_count,
    nxvm_baseline_cpu_probe_capture *out_capture);
void nxvm_baseline_cpu_probe_end(void);

#ifdef __cplusplus
}
#endif

#endif
