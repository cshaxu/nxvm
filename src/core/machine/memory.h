#ifndef NXVM_CORE_MEMORY_H
#define NXVM_CORE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

nxvm_core_status nxvm_core_machine_memory_read(
    const nxvm_core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size);

nxvm_core_status nxvm_core_machine_memory_write(
    nxvm_core_machine *machine,
    uint32_t physical,
    const void *data,
    size_t size);

nxvm_core_status nxvm_core_machine_set_a20(
    nxvm_core_machine *machine,
    int enabled);

#ifdef __cplusplus
}
#endif

#endif
