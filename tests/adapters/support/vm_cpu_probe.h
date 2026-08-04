#ifndef TEST_VM_SESSION_CPU_PROBE_H
#define TEST_VM_SESSION_CPU_PROBE_H

#include "type.h"



#ifdef __cplusplus
extern "C" {
#endif

#define TEST_VM_CPU_PROBE_MAX_BYTES 15u

typedef struct vm_session_cpu_probe_state {
    uint16_t cs;
    uint16_t ip;
    uint32_t linear_pc;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t eflags;
} vm_session_cpu_probe_state;

typedef struct vm_session_cpu_probe_capture {
    uint8_t bytes[TEST_VM_CPU_PROBE_MAX_BYTES];
    STD_SIZE_T byte_count;
    vm_session_cpu_probe_state before;
    vm_session_cpu_probe_state after;
    uint32_t exception_mask;
    uint32_t exception_code;
} vm_session_cpu_probe_capture;

typedef struct test_vm_cpu_probe test_vm_cpu_probe;

C_INT vm_session_cpu_probe_create(test_vm_cpu_probe **out_probe);
C_INT vm_session_cpu_probe_step(
    test_vm_cpu_probe *probe,
    const uint8_t *bytes,
    STD_SIZE_T byte_count,
    vm_session_cpu_probe_capture *out_capture);
C_VOID vm_session_cpu_probe_destroy(test_vm_cpu_probe *probe);

#ifdef __cplusplus
}
#endif

#endif
