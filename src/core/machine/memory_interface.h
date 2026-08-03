#ifndef NTVDM64_CORE_MACHINE_MEMORY_INTERFACE_H
#define NTVDM64_CORE_MACHINE_MEMORY_INTERFACE_H


#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

ntvdm64_status core_machine_memory_read(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size);

ntvdm64_status core_machine_memory_write(
    core_machine *machine,
    uint32_t physical,
    const C_VOID *data,
    STD_SIZE_T size);

ntvdm64_status core_machine_set_a20(
    core_machine *machine,
    C_INT enabled);

#ifdef __cplusplus
}
#endif

#endif
