#ifndef VM_PLATFORM_UX_BINDING_H
#define VM_PLATFORM_UX_BINDING_H

#include "type.h"
#include "lib/ux/presenter.h"

struct vm_platform_run_context;
struct vm_platform_run_handle;

typedef enum vm_platform_ux_action_id {
    VM_PLATFORM_UX_ACTION_PAUSE_TOGGLE = 1u,
    VM_PLATFORM_UX_ACTION_SEND_CTRL_ALT_DEL,
    VM_PLATFORM_UX_ACTION_SEND_ALT_ENTER,
    VM_PLATFORM_UX_ACTION_RELEASE_MOUSE
} vm_platform_ux_action_id;

type_status vm_platform_ux_binding_initialize(
    const struct vm_platform_run_context *context,
    struct vm_platform_run_handle *handle, ux_binding *out_binding);
C_INT vm_platform_ux_event_submit(const struct vm_platform_run_context *context,
    const ux_event *event);

#endif
