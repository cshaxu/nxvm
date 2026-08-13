#ifndef TEST_VM_SESSION_CPU_PROBE_H
#define TEST_VM_SESSION_CPU_PROBE_H

#include "type.h"



#ifdef __cplusplus
extern "C" {
#endif

#define TEST_VM_CPU_PROBE_MAX_BYTES 15u

typedef struct vm_session_cpu_probe_state {
    type_unsigned_16 cs;
    type_unsigned_16 ip;
    type_unsigned_32 linear_pc;
    type_unsigned_32 eax;
    type_unsigned_32 ebx;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 eflags;
} vm_session_cpu_probe_state;

typedef struct vm_session_cpu_probe_capture {
    type_unsigned_8 bytes[TEST_VM_CPU_PROBE_MAX_BYTES];
    STD_SIZE_T byte_count;
    vm_session_cpu_probe_state before;
    vm_session_cpu_probe_state after;
    type_unsigned_32 exception_mask;
    type_unsigned_32 exception_code;
} vm_session_cpu_probe_capture;

typedef struct test_vm_cpu_probe test_vm_cpu_probe;

C_INT vm_session_cpu_probe_create(test_vm_cpu_probe **out_probe);
C_INT vm_session_cpu_probe_step(
    test_vm_cpu_probe *probe,
    const type_unsigned_8 *bytes,
    STD_SIZE_T byte_count,
    vm_session_cpu_probe_capture *out_capture);
C_VOID vm_session_cpu_probe_destroy(test_vm_cpu_probe *probe);

#ifdef __cplusplus
}
#endif

#endif
