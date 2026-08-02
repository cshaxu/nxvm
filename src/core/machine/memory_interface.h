#ifndef NTVDM64_CORE_MACHINE_MEMORY_INTERFACE_H
#define NTVDM64_CORE_MACHINE_MEMORY_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

nxvm_core_status core_machine_memory_read(
    const core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size);

nxvm_core_status core_machine_memory_write(
    core_machine *machine,
    uint32_t physical,
    const void *data,
    size_t size);

nxvm_core_status core_machine_set_a20(
    core_machine *machine,
    int enabled);

#ifdef __cplusplus
}
#endif

#endif
