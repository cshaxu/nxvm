#ifndef NTVDM64_CORE_MACHINE_CPU_INTERFACE_H
#define NTVDM64_CORE_MACHINE_CPU_INTERFACE_H

#include "type.h"

typedef enum core_machine_cpu_profile {
    CORE_MACHINE_CPU_PROFILE_DEFAULT = 0,
    CORE_MACHINE_CPU_PROFILE_8086,
    CORE_MACHINE_CPU_PROFILE_80186,
    CORE_MACHINE_CPU_PROFILE_80286,
    CORE_MACHINE_CPU_PROFILE_80386
} core_machine_cpu_profile;

const C_CHAR *core_machine_cpu_profile_name(core_machine_cpu_profile profile);

typedef struct core_machine_cpu_state {
    uint16_t cs;
    uint32_t cs_base;
    uint32_t eip;
    uint32_t eflags;
    uint8_t halted;
} core_machine_cpu_state;

#define CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY 32u
#define CORE_MACHINE_CPU_DIAGNOSTIC_BYTES 15u

typedef struct core_machine_cpu_execution_point {
    uint16_t cs;
    uint32_t cs_base;
    uint32_t eip;
    uint32_t linear_pc;
    uint8_t bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    uint8_t byte_count;
} core_machine_cpu_execution_point;

typedef struct core_machine_cpu_fault_snapshot {
    C_INT valid;
    uint32_t exception_mask;
    uint32_t exception_code;
    core_machine_cpu_execution_point point;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
} core_machine_cpu_fault_snapshot;

typedef struct core_machine_cpu_diagnostic {
    core_machine_cpu_fault_snapshot first_fault;
    core_machine_cpu_execution_point recent[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    STD_SIZE_T recent_count;
} core_machine_cpu_diagnostic;

#endif
