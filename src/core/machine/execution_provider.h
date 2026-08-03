#ifndef NTVDM64_CORE_MACHINE_EXECUTION_PROVIDER_H
#define NTVDM64_CORE_MACHINE_EXECUTION_PROVIDER_H


#include "type.h"
typedef struct core_machine_execution_provider {
    C_VOID (*reset)(C_VOID *context);
    C_VOID (*refresh)(C_VOID *context);
} core_machine_execution_provider;

#endif
