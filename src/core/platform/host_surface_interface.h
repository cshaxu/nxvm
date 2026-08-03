#ifndef NTVDM64_CORE_PLATFORM_HOST_SURFACE_INTERFACE_H
#define NTVDM64_CORE_PLATFORM_HOST_SURFACE_INTERFACE_H

#include <stdatomic.h>
#include <stdint.h>

#include "type.h"

typedef enum core_platform_host_surface_kind {
    CORE_PLATFORM_HOST_SURFACE_CONSOLE,
    CORE_PLATFORM_HOST_SURFACE_WINDOW,
    CORE_PLATFORM_HOST_SURFACE_TERMINAL
} core_platform_host_surface_kind;

typedef struct core_platform_host_surface_context {
    core_platform_host_surface_kind kind;
    void *native_handle;
} core_platform_host_surface_context;

typedef struct core_platform_host_surface_lease {
    atomic_uintptr_t owner;
} core_platform_host_surface_lease;

void core_platform_host_surface_context_initialize(
    core_platform_host_surface_context *context,
    core_platform_host_surface_kind kind, void *native_handle);
void core_platform_host_surface_lease_initialize(
    core_platform_host_surface_lease *lease);
nxvm_core_status core_platform_host_surface_lease_acquire(
    core_platform_host_surface_lease *lease, const void *owner);
nxvm_core_status core_platform_host_surface_lease_release(
    core_platform_host_surface_lease *lease, const void *owner);
int core_platform_host_surface_lease_is_owned_by(
    const core_platform_host_surface_lease *lease, const void *owner);

#endif
