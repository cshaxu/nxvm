#ifndef NXVM_COMPOSITION_CPU_PROBE_H
#define NXVM_COMPOSITION_CPU_PROBE_H

#include "type.h"



#ifdef __cplusplus
extern "C" {
#endif

#define NXVM_BASELINE_CPU_PROBE_MAX_BYTES 15u

typedef struct vm_composition_cpu_probe_state {
    uint16_t cs;
    uint16_t ip;
    uint32_t linear_pc;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t eflags;
} vm_composition_cpu_probe_state;

typedef struct vm_composition_cpu_probe_capture {
    uint8_t bytes[NXVM_BASELINE_CPU_PROBE_MAX_BYTES];
    STD_SIZE_T byte_count;
    vm_composition_cpu_probe_state before;
    vm_composition_cpu_probe_state after;
    uint32_t exception_mask;
    uint32_t exception_code;
} vm_composition_cpu_probe_capture;

typedef struct nxvm_cpu_probe nxvm_cpu_probe;

C_INT vm_composition_cpu_probe_create(nxvm_cpu_probe **out_probe);
C_INT vm_composition_cpu_probe_step(
    nxvm_cpu_probe *probe,
    const uint8_t *bytes,
    STD_SIZE_T byte_count,
    vm_composition_cpu_probe_capture *out_capture);
C_VOID vm_composition_cpu_probe_destroy(nxvm_cpu_probe *probe);

#ifdef __cplusplus
}
#endif

#endif
