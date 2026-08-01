#ifndef NXVM_CORE_CPU_H
#define NXVM_CORE_CPU_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

typedef struct nxvm_core_cpu_state {
    uint16_t cs;
    uint32_t cs_base;
    uint32_t eip;
    uint32_t eflags;
    uint8_t halted;
} nxvm_core_cpu_state;

nxvm_core_status nxvm_core_machine_get_cpu_state(
    const nxvm_core_machine *machine,
    nxvm_core_cpu_state *out_state);

#ifdef __cplusplus
}
#endif

#endif
