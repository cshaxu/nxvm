/* Product session threads use only neutral lib host and UX contracts. */

#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/platform/platform_internal.h"

typedef struct vm_platform_ux_handle {
    vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    host_sync_task *kernel_task;
    host_sync_event *kernel_started;
    host_sync_event *console_released;
} vm_platform_ux_handle;

static void vm_platform_signal_started(void *opaque)
{ host_sync_event_signal(opaque); }

static void vm_platform_ux_kernel_task(void *opaque, const host_sync_task *task)
{
    vm_platform_ux_handle *handle = opaque;

    (void)task;
    if (handle == STD_NULL) return;
    vm_session_state_start(handle->context->execution->state);
    vm_platform_signal_started(handle->kernel_started);
    handle->context->execution->run(handle->context->execution->context);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
}

type_status vm_platform_start(vm_platform_run_context *context,
    vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->execution->state == STD_NULL ||
        context->execution->run == STD_NULL || context->execution->stop == STD_NULL ||
        context->input_sink.submit == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    handle = STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    handle->context = context;
    handle->owner = owner;
    owner->context = context;
    context->run_handle = owner;
    owner->backend = handle;
    owner->active = TYPE_TRUE;
    if (host_sync_event_create(&handle->kernel_started) != LIB_STATUS_OK ||
        host_sync_event_create(&handle->console_released) != LIB_STATUS_OK ||
        host_sync_task_create(vm_platform_ux_kernel_task, handle,
            &handle->kernel_task) != LIB_STATUS_OK) goto fail;
    if (host_sync_event_wait(handle->kernel_started,
            VM_PLATFORM_START_TIMEOUT_MILLISECONDS) != HOST_SYNC_WAIT_SIGNALED)
        goto fail;
    if (vm_platform_run_context_set_display_mode(context,
            context->display_mode) != TYPE_STATUS_OK) goto fail;
    return TYPE_STATUS_OK;

fail:
    vm_platform_run_handle_request_stop(owner);
    vm_platform_run_handle_join(owner);
    vm_platform_run_handle_finalize(owner);
    return TYPE_STATUS_INVALID_STATE;
}

C_VOID vm_platform_run_handle_request_stop(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    vm_platform_run_handle_request_presenter_stop(owner);
    vm_platform_run_handle_note_console_released(owner);
    if (handle != STD_NULL) handle->context->execution->stop(
        handle->context->execution->context);
    if (handle != STD_NULL) {
        host_sync_task_request_cancel(handle->kernel_task);
    }
}

C_VOID vm_platform_run_handle_note_console_released(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle != STD_NULL) host_sync_event_signal(handle->console_released);
}

C_VOID vm_platform_run_handle_wait_console_release(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle != STD_NULL) (C_VOID)host_sync_event_wait(handle->console_released,
        0xffffffffu);
}

C_VOID vm_platform_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_join(handle->kernel_task);
}

C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_destroy(handle->kernel_task);
    host_sync_event_destroy(handle->console_released);
    host_sync_event_destroy(handle->kernel_started);
    STD_FREE(handle);
    if (owner->context != STD_NULL && owner->context->run_handle == owner) {
        owner->context->run_handle = STD_NULL;
    }
    vm_platform_run_handle_initialize(owner);
}
