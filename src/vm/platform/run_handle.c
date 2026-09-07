/* Product session threads use only neutral lib host and UX contracts. */

#include "type.h"

#include "lib/host/sync.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"

typedef struct vm_platform_ux_handle {
    vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    host_sync_task *kernel_task;
    host_sync_task *presenter_task;
    host_sync_event *kernel_started;
} vm_platform_ux_handle;

static void vm_platform_signal_started(void *opaque)
{ host_sync_event_signal(opaque); }

static void vm_platform_ux_presenter_task(void *opaque,
    const host_sync_task *task)
{
    vm_platform_ux_handle *handle = opaque;
    ux_binding binding;

    (void)task;
    if (handle == STD_NULL || vm_platform_ux_binding_initialize(
            handle->context, handle->owner, &binding) != TYPE_STATUS_OK ||
        ux_run(&binding) == UX_RUN_ERROR_RESULT) {
        if (handle != STD_NULL) vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
    }
}

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
    owner->backend = handle;
    owner->active = TYPE_TRUE;
    if (host_sync_event_create(&handle->kernel_started) != LIB_STATUS_OK ||
        host_sync_task_create(vm_platform_ux_kernel_task, handle,
            &handle->kernel_task) != LIB_STATUS_OK) goto fail;
    if (host_sync_event_wait(handle->kernel_started,
            VM_PLATFORM_START_TIMEOUT_MILLISECONDS) != HOST_SYNC_WAIT_SIGNALED)
        goto fail;
    if (host_sync_task_create(vm_platform_ux_presenter_task, handle,
            &handle->presenter_task) != LIB_STATUS_OK) goto fail;
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
    if (handle != STD_NULL) handle->context->execution->stop(
        handle->context->execution->context);
    if (handle != STD_NULL) {
        host_sync_task_request_cancel(handle->kernel_task);
        host_sync_task_request_cancel(handle->presenter_task);
    }
}

C_VOID vm_platform_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_join(handle->kernel_task);
    host_sync_task_join(handle->presenter_task);
}

C_VOID vm_platform_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_ux_handle *handle = owner == STD_NULL ? STD_NULL : owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_destroy(handle->kernel_task);
    host_sync_task_destroy(handle->presenter_task);
    host_sync_event_destroy(handle->kernel_started);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
