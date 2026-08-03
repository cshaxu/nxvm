#ifndef NTVDM64_CORE_MACHINE_EXECUTION_PROVIDER_H
#define NTVDM64_CORE_MACHINE_EXECUTION_PROVIDER_H

typedef struct core_machine_execution_provider {
    void (*refresh)(void *context);
} core_machine_execution_provider;

#endif
