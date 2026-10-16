#ifndef X86_CORE_MACHINE_EXECUTION_PROVIDER_H
#define X86_CORE_MACHINE_EXECUTION_PROVIDER_H
#include "lib/types/types_interface.h"


typedef struct x86_core_machine_execution_provider {
    void (*reset)(void *context);
    void (*advance_time)(void *context, lib_u64 elapsed_ticks);
} x86_core_machine_execution_provider;

#endif
