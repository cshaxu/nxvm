#ifndef CORE_MACHINE_FIRMWARE_INTERRUPT_INTERFACE_H
#define CORE_MACHINE_FIRMWARE_INTERRUPT_INTERFACE_H

#include "type.h"

typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

#define CORE_MACHINE_FIRMWARE_INTERRUPT_PORTAL_CAPACITY 16u

typedef C_VOID (*core_machine_firmware_interrupt_provider)(C_VOID *context,
    core_machine_cpu_execution_context *execution, uint8_t vector);

typedef struct core_machine_firmware_interrupt_portal {
    uint8_t vector;
    uint32_t origin_linear_start;
    uint32_t origin_linear_bytes;
    core_machine_firmware_interrupt_provider provider;
    C_VOID *context;
} core_machine_firmware_interrupt_portal;

#endif
