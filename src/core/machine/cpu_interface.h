#ifndef NTVDM64_CORE_MACHINE_CPU_INTERFACE_H
#define NTVDM64_CORE_MACHINE_CPU_INTERFACE_H

#include "type.h"

typedef struct core_machine_cpu_state {
    uint16_t cs;
    uint32_t cs_base;
    uint32_t eip;
    uint32_t eflags;
    uint8_t halted;
} core_machine_cpu_state;

#endif
