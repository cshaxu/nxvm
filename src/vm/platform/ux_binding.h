#ifndef VM_PLATFORM_UX_BINDING_H
#define VM_PLATFORM_UX_BINDING_H

#include "type.h"
#include "lib/ux/presenter.h"

struct vm_platform_run_context;
struct vm_platform_run_handle;

type_status vm_platform_ux_binding_initialize(
    const struct vm_platform_run_context *context,
    struct vm_platform_run_handle *handle, ux_binding *out_binding);

#endif
