/* NXVM's Linux product binding owns VM threads only.  lib/ux owns ncurses. */

#include "type.h"

#include "lib/ux/linux/console.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/execution_wait.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/linux/linux.h"

#include <pthread.h>

typedef struct vm_platform_linux_ux_handle {
    const vm_platform_run_context *context;
    vm_platform_run_handle *owner;
    pthread_t kernel_thread;
    pthread_t presenter_thread;
    C_INT kernel_started;
    C_INT presenter_started;
} vm_platform_linux_ux_handle;

static C_VOID *vm_platform_linux_ux_presenter_thread(C_VOID *opaque)
{
    vm_platform_linux_ux_handle *handle = opaque;
    ux_binding binding;

    if (handle == STD_NULL || vm_platform_ux_binding_initialize(
            handle->context, handle->owner, &binding) != TYPE_STATUS_OK ||
        ux_linux_run_console(&binding) == UX_RUN_ERROR_RESULT) {
        if (handle != STD_NULL) vm_platform_run_handle_report(handle->owner,
            VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
    }
    return STD_NULL;
}

static C_VOID *vm_platform_linux_ux_kernel_thread(C_VOID *opaque)
{
    vm_platform_linux_ux_handle *handle = opaque;

    if (handle == STD_NULL) return STD_NULL;
    lib_session_executor_start(handle->context->execution);
    vm_platform_run_handle_report(handle->owner,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    return STD_NULL;
}

type_status vm_platform_linux_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle;
    C_INT old_flip;

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
    old_flip = lib_session_executor_get_flip(context->execution);
    if (pthread_create(&handle->kernel_thread, STD_NULL,
            vm_platform_linux_ux_kernel_thread, handle) != 0) {
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->kernel_started = TYPE_TRUE;
    if (!vm_platform_wait_for_execution_flip(context->execution, old_flip,
            VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS)) {
        vm_platform_linux_run_handle_request_stop(owner);
        vm_platform_linux_run_handle_join(owner);
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (pthread_create(&handle->presenter_thread, STD_NULL,
            vm_platform_linux_ux_presenter_thread, handle) != 0) {
        vm_platform_linux_run_handle_request_stop(owner);
        vm_platform_linux_run_handle_join(owner);
        vm_platform_linux_run_handle_finalize(owner);
        return TYPE_STATUS_INVALID_STATE;
    }
    handle->presenter_started = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

C_VOID vm_platform_linux_run_handle_request_stop(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle != STD_NULL) lib_session_executor_stop(
        handle->context->execution);
}

C_VOID vm_platform_linux_run_handle_join(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    if (handle->kernel_started) (C_VOID)pthread_join(handle->kernel_thread,
        STD_NULL);
    if (handle->presenter_started) (C_VOID)pthread_join(handle->presenter_thread,
        STD_NULL);
}

C_VOID vm_platform_linux_run_handle_finalize(vm_platform_run_handle *owner)
{
    vm_platform_linux_ux_handle *handle = owner == STD_NULL ? STD_NULL :
        owner->backend;

    if (handle == STD_NULL) return;
    STD_FREE(handle);
    vm_platform_run_handle_initialize(owner);
}
