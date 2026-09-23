#ifndef CORE_MACHINE_CPU_INTERFACE_H
#define CORE_MACHINE_CPU_INTERFACE_H
#include "lib/types/types_interface.h"

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
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u8 halted;
} core_machine_cpu_state;

#define CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY 32u
#define CORE_MACHINE_CPU_DIAGNOSTIC_BYTES 15u

typedef struct core_machine_cpu_execution_point {
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 linear_pc;
    lib_u8 bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    lib_u8 byte_count;
} core_machine_cpu_execution_point;

typedef struct core_machine_cpu_fault_snapshot {
    C_INT valid;
    lib_u32 exception_mask;
    lib_u32 exception_code;
    core_machine_cpu_execution_point point;
    lib_u32 eax;
    lib_u32 ebx;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 cr2;
    lib_u32 esp;
    lib_u16 ss;
    lib_u32 ss_base;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 eflags;
} core_machine_cpu_fault_snapshot;

typedef struct core_machine_cpu_diagnostic {
    core_machine_cpu_fault_snapshot first_fault;
    core_machine_cpu_fault_snapshot first_delivered_exception;
    core_machine_cpu_fault_snapshot last_delivered_exception;
    lib_u32 delivered_exception_count;
    core_machine_cpu_execution_point recent[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    lib_size recent_count;
} core_machine_cpu_diagnostic;

#endif
