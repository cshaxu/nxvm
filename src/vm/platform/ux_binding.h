#ifndef VM_PLATFORM_UX_BINDING_H
#define VM_PLATFORM_UX_BINDING_H

#include "type.h"
#include "lib/ux-base/event.h"

struct vm_platform_run_context;
struct vm_platform_run_handle;

C_INT vm_platform_ux_event_submit(const struct vm_platform_run_context *context,
    const ux_input_event *event);

#endif
