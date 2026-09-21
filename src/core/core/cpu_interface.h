#ifndef CORE_MACHINE_CPU_INTERFACE_H
#define CORE_MACHINE_CPU_INTERFACE_H

#include "type.h"

/* An external-cycle address is only comparable within its named CPU space.
 * This is shared by profile configuration and the Core CPU lifecycle. */
typedef enum core_machine_cpu_external_cycle_space {
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY = 0,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT
} core_machine_cpu_external_cycle_space;

typedef enum core_machine_cpu_profile {
    CORE_MACHINE_CPU_PROFILE_DEFAULT = 0,
    CORE_MACHINE_CPU_PROFILE_8086,
    CORE_MACHINE_CPU_PROFILE_8088,
    CORE_MACHINE_CPU_PROFILE_80186,
    CORE_MACHINE_CPU_PROFILE_80286,
    CORE_MACHINE_CPU_PROFILE_80386
} core_machine_cpu_profile;

static inline C_INT core_machine_cpu_profile_has_8086_semantics(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_8086 ||
        profile == CORE_MACHINE_CPU_PROFILE_8088;
}

#define CORE_MACHINE_CPU_DEVICE_NAME "Intel 8086+"

const C_CHAR *core_machine_cpu_profile_name(core_machine_cpu_profile profile);

typedef struct core_machine_cpu_state {
    type_unsigned_16 cs;
    type_unsigned_32 cs_base;
    type_unsigned_32 eip;
    type_unsigned_32 eflags;
    type_unsigned_8 halted;
} core_machine_cpu_state;

#define CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY 32u
#define CORE_MACHINE_CPU_DIAGNOSTIC_BYTES 15u

typedef struct core_machine_cpu_execution_point {
    type_unsigned_16 cs;
    type_unsigned_32 cs_base;
    type_unsigned_32 eip;
    type_unsigned_32 linear_pc;
    type_unsigned_8 bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    type_unsigned_8 byte_count;
} core_machine_cpu_execution_point;

typedef struct core_machine_cpu_fault_snapshot {
    C_INT valid;
    type_unsigned_32 exception_mask;
    type_unsigned_32 exception_code;
    core_machine_cpu_execution_point point;
    type_unsigned_32 eax;
    type_unsigned_32 ebx;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 cr2;
    type_unsigned_32 esp;
    type_unsigned_16 ss;
    type_unsigned_32 ss_base;
    type_unsigned_32 ebp;
    type_unsigned_32 esi;
    type_unsigned_32 edi;
    type_unsigned_32 eflags;
} core_machine_cpu_fault_snapshot;

typedef struct core_machine_cpu_diagnostic {
    core_machine_cpu_fault_snapshot first_fault;
    core_machine_cpu_fault_snapshot first_delivered_exception;
    core_machine_cpu_fault_snapshot last_delivered_exception;
    type_unsigned_32 delivered_exception_count;
    core_machine_cpu_execution_point recent[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    STD_SIZE_T recent_count;
} core_machine_cpu_diagnostic;

#endif
