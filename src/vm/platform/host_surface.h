#ifndef VM_PLATFORM_HOST_SURFACE_H
#define VM_PLATFORM_HOST_SURFACE_H


#include "type.h"

typedef enum vm_platform_host_surface_kind {
    VM_PLATFORM_HOST_SURFACE_CONSOLE,
    VM_PLATFORM_HOST_SURFACE_WINDOW,
    VM_PLATFORM_HOST_SURFACE_TERMINAL
} vm_platform_host_surface_kind;

typedef struct vm_platform_host_surface_context {
    vm_platform_host_surface_kind kind;
    C_VOID *native_handle;
} vm_platform_host_surface_context;

typedef struct vm_platform_host_surface_lease {
    STD_ATOMIC_UINTPTR_T owner;
} vm_platform_host_surface_lease;

C_VOID vm_platform_host_surface_context_initialize(
    vm_platform_host_surface_context *context,
    vm_platform_host_surface_kind kind, C_VOID *native_handle);
C_VOID vm_platform_host_surface_lease_initialize(
    vm_platform_host_surface_lease *lease);
type_status vm_platform_host_surface_lease_acquire(
    vm_platform_host_surface_lease *lease, const C_VOID *owner);
type_status vm_platform_host_surface_lease_release(
    vm_platform_host_surface_lease *lease, const C_VOID *owner);
C_INT vm_platform_host_surface_lease_is_owned_by(
    const vm_platform_host_surface_lease *lease, const C_VOID *owner);

#endif
