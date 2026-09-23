#ifndef CORE_MACHINE_D4_MEMORY_H
#define CORE_MACHINE_D4_MEMORY_H
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/memory_interface.h"

typedef struct core_machine core_machine;
typedef struct core_machine_d4_memory_config core_machine_d4_memory_config;

typedef struct core_machine_d4_memory {
    lib_u8 control;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 reset_ram_setup;
    lib_u16 ram_setup;
    lib_u8 parity_fault_mask;
    type_bool configured;
} core_machine_d4_memory;

C_INT core_machine_d4_memory_config_is_valid(
    const core_machine_d4_memory_config *config);
type_status core_machine_d4_memory_configure(core_machine *machine,
    const core_machine_d4_memory_config *config);
C_VOID core_machine_d4_memory_reset(core_machine *machine);

#endif
