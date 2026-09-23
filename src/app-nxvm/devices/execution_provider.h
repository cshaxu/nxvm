#ifndef CORE_MACHINE_EXECUTION_PROVIDER_H
#define CORE_MACHINE_EXECUTION_PROVIDER_H
#include "lib/types/types_interface.h"


#include "type.h"
typedef struct core_machine_execution_provider {
    C_VOID (*reset)(C_VOID *context);
    C_VOID (*advance_time)(C_VOID *context, lib_u64 elapsed_ticks);
} core_machine_execution_provider;

#endif
