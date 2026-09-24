#ifndef CORE_MACHINE_EXECUTION_PROVIDER_H
#define CORE_MACHINE_EXECUTION_PROVIDER_H
#include "lib/types/types_interface.h"


typedef struct core_machine_execution_provider {
    void (*reset)(void *context);
    void (*advance_time)(void *context, lib_u64 elapsed_ticks);
} core_machine_execution_provider;

#endif
