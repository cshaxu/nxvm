/* NXVM's Linux product binding owns VM threads only.  lib/ux owns ncurses. */

#include "type.h"

#include "lib/host/sync.h"
#include "lib/ux/linux/console.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/linux/linux.h"

typedef struct vm_platform_linux_ux_handle {
    const vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    host_sync_task *kernel_task;
    host_sync_task *presenter_task;
    host_sync_event *kernel_started;
} vm_platform_linux_ux_handle;

static void vm_platform_linux_signal_started(void *opaque)
{ host_sync_event_signal(opaque); }

static void vm_platform_linux_ux_presenter_task(void *opaque,
    const host_sync_task *task)
{
    vm_platform_linux_ux_handle *handle = opaque;
    ux_binding binding;

    (void)task;
    if (handle == STD_NULL || vm_platform_ux_binding_initialize(
            handle->context, handle->owner, &binding) != TYPE_STATUS_OK ||
        ux_linux_run_console(&binding) == UX_RUN_ERROR_RESULT) {
        if (handle != STD_NULL) vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
    }
}

static void vm_platform_linux_ux_kernel_task(void *opaque,
    const host_sync_task *task)
{
    vm_platform_linux_ux_handle *handle = opaque;

    (void)task;
    if (handle == STD_NULL) return;
    lib_session_executor_start(handle->context->execution,
        vm_platform_linux_signal_started, handle->kernel_started);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
}

type_status vm_platform_linux_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle;

    if (context == STD_NULL || owner == STD_NULL || owner->active ||
        context->execution == STD_NULL || context->input_sink.submit == STD_NULL ||
        vm_platform_run_context_get_display_mode(context) ==
            VM_PLATFORM_DISPLAY_WINDOW) return TYPE_STATUS_INVALID_ARGUMENT;
    handle = STD_CALLOC(1u, sizeof(*handle));
    if (handle == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    handle->context = context;
    handle->owner = owner;
    owner->context = context;
    owner->backend = handle;
    owner->window_display = TYPE_FALSE;
    owner->active = TYPE_TRUE;
    if (host_sync_event_create(&handle->kernel_started) != LIB_STATUS_OK ||
        host_sync_task_create(vm_platform_linux_ux_kernel_task, handle,
            &handle->kernel_task) != LIB_STATUS_OK) {
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (host_sync_event_wait(handle->kernel_started,
            VM_PLATFORM_START_TIMEOUT_MILLISECONDS) !=
        HOST_SYNC_WAIT_SIGNALED) {
        vm_platform_linux_run_handle_request_stop(owner);
        vm_platform_linux_run_handle_join(owner);
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (host_sync_task_create(vm_platform_linux_ux_presenter_task, handle,
            &handle->presenter_task) != LIB_STATUS_OK) {
        vm_platform_linux_run_handle_request_stop(owner);
        vm_platform_linux_run_handle_join(owner);
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_linux_run_handle_request_stop(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle != STD_NULL) lib_session_executor_stop(
        handle->context->execution);
    if (handle != STD_NULL) {
        ux_mailbox_wake(handle->context->ux_mailbox);
        host_sync_task_request_cancel(handle->kernel_task);
        host_sync_task_request_cancel(handle->presenter_task);
    }
}

C_VOID vm_platform_linux_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_join(handle->kernel_task);
    host_sync_task_join(handle->presenter_task);
}

C_VOID vm_platform_linux_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    host_sync_task_destroy(handle->kernel_task);
    host_sync_task_destroy(handle->presenter_task);
    host_sync_event_destroy(handle->kernel_started);
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
