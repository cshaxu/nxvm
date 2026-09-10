#include "type.h"

#include <windows.h>

#include "lib/host/sync_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/platform.h"
#include "../support/rom/session_assets.h"

typedef struct vm_console_pause_resume_binding {
    STD_ATOMIC_INT claims;
    STD_ATOMIC_INT releases;
} vm_console_pause_resume_binding;

typedef struct vm_console_pause_resume_lifecycle_log {
    vm_session_lifecycle events[3];
    STD_ATOMIC_INT count;
} vm_console_pause_resume_lifecycle_log;

typedef struct vm_console_pause_resume_run {
    vm_session *session;
    type_status status;
    host_sync_event *complete;
} vm_console_pause_resume_run;

static type_status vm_console_pause_resume_claim(C_VOID *opaque,
    C_VOID *logical_console)
{
    vm_console_pause_resume_binding *binding = opaque;

    if (binding == STD_NULL || logical_console == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    (C_VOID)STD_ATOMIC_EXCHANGE(&binding->claims,
        STD_ATOMIC_LOAD(&binding->claims) + 1);
    return TYPE_STATUS_OK;
}

static type_status vm_console_pause_resume_release(C_VOID *opaque,
    C_VOID *logical_console)
{
    vm_console_pause_resume_binding *binding = opaque;

    if (binding == STD_NULL || logical_console == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    (C_VOID)STD_ATOMIC_EXCHANGE(&binding->releases,
        STD_ATOMIC_LOAD(&binding->releases) + 1);
    return TYPE_STATUS_OK;
}

static C_VOID vm_console_pause_resume_report_lifecycle(C_VOID *opaque,
    vm_session_lifecycle lifecycle)
{
    vm_console_pause_resume_lifecycle_log *log = opaque;
    C_INT index;

    if (log == STD_NULL) return;
    index = STD_ATOMIC_LOAD(&log->count);
    if (index < (C_INT)(sizeof(log->events) / sizeof(log->events[0]))) {
        log->events[index] = lifecycle;
        (C_VOID)STD_ATOMIC_EXCHANGE(&log->count, index + 1);
    }
}

static C_VOID vm_console_pause_resume_run_entry(C_VOID *opaque,
    const host_sync_task *task)
{
    vm_console_pause_resume_run *run = opaque;

    (C_VOID)task;
    if (run == STD_NULL) return;
    run->status = vm_session_resume(run->session);
    host_sync_event_signal(run->complete);
}

static C_INT vm_console_pause_resume_wait_for_state(const vm_session *session,
    C_INT paused)
{
    C_UINT waited;

    for (waited = 0u; waited != 2000u; ++waited) {
        if (vm_session_control_is_running(&session->control) == !paused &&
            vm_session_control_is_paused(&session->control) == paused) {
            return TYPE_TRUE;
        }
        Sleep(1u);
    }
    return TYPE_FALSE;
}

static C_INT vm_console_pause_resume_wait_for_console(const vm_session *session)
{
    C_UINT waited;

    for (waited = 0u; waited != 2000u; ++waited) {
        if (vm_platform_run_handle_is_active(session->platform_run_handle) &&
            vm_platform_run_handle_is_console_display(session->platform_run_handle)) {
            return TYPE_TRUE;
        }
        Sleep(1u);
    }
    return TYPE_FALSE;
}

static C_INT vm_console_pause_resume_wait_for_window(const vm_session *session,
    C_INT present)
{
    C_UINT waited;

    for (waited = 0u; waited != 2000u; ++waited) {
        if (vm_platform_run_handle_is_window_display(session->platform_run_handle) ==
                present) {
            return TYPE_TRUE;
        }
        Sleep(1u);
    }
    return TYPE_FALSE;
}

static C_INT vm_console_pause_resume_run_window_close_case(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT passed = TYPE_FALSE;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_control_reset(&session->control) != TYPE_STATUS_OK ||
        vm_platform_run_context_set_display_mode(session->platform_run_context,
            VM_PLATFORM_DISPLAY_WINDOW) != TYPE_STATUS_OK ||
        vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait_for_window(session, TYPE_TRUE)) goto done;
    vm_platform_run_handle_report(session->platform_run_handle,
        VM_PLATFORM_RUN_EVENT_WINDOW_CLOSE_REQUESTED);
    if (!vm_console_pause_resume_wait_for_state(session, TYPE_TRUE) ||
        !vm_console_pause_resume_wait_for_window(session, TYPE_FALSE) ||
        !vm_platform_run_handle_is_active(session->platform_run_handle)) goto done;
    if (vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait_for_state(session, TYPE_FALSE) ||
        !vm_console_pause_resume_wait_for_window(session, TYPE_TRUE)) goto done;
    /* A recreated Window must still have a live runner that accepts a second
     * native close request. */
    vm_platform_run_handle_report(session->platform_run_handle,
        VM_PLATFORM_RUN_EVENT_WINDOW_CLOSE_REQUESTED);
    passed = vm_console_pause_resume_wait_for_state(session, TYPE_TRUE) &&
        vm_console_pause_resume_wait_for_window(session, TYPE_FALSE) &&
        vm_platform_run_handle_is_active(session->platform_run_handle);

done:
    vm_session_stop(session);
    vm_session_destroy(session);
    return passed;
}

static C_INT vm_console_pause_resume_run_bound_console_case(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_console_pause_resume_binding binding_state;
    vm_console_pause_resume_lifecycle_log lifecycle_log;
    const vm_platform_console_binding binding = {
        &binding_state,
        vm_console_pause_resume_claim,
        vm_console_pause_resume_release
    };
    vm_console_pause_resume_run run = {0};
    host_sync_task *task = STD_NULL;
    C_INT passed = TYPE_FALSE;

    STD_ATOMIC_INIT(&binding_state.claims, 0);
    STD_ATOMIC_INIT(&binding_state.releases, 0);
    STD_ATOMIC_INIT(&lifecycle_log.count, 0);
    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_set_console_binding(session, &binding) != TYPE_STATUS_OK ||
        vm_session_control_reset(&session->control) != TYPE_STATUS_OK ||
        host_sync_event_create(&run.complete) != LIB_STATUS_OK) goto done;
    vm_session_set_lifecycle_reporter(session,
        vm_console_pause_resume_report_lifecycle, &lifecycle_log);
    run.session = session;
    if (host_sync_task_create(vm_console_pause_resume_run_entry, &run, &task) !=
            LIB_STATUS_OK || !vm_console_pause_resume_wait_for_console(session)) goto done;
    vm_platform_run_handle_report(session->platform_run_handle,
        VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
    if (host_sync_event_wait(run.complete, 2000u) != HOST_SYNC_WAIT_SIGNALED) goto done;
    host_sync_task_join(task);
    passed = run.status == TYPE_STATUS_OK &&
        vm_session_control_is_paused(&session->control) &&
        vm_session_state_is_active(session->control.state) &&
        !vm_platform_run_handle_is_active(session->platform_run_handle) &&
        STD_ATOMIC_LOAD(&binding_state.claims) == 1 &&
        STD_ATOMIC_LOAD(&binding_state.releases) == 1 &&
        STD_ATOMIC_LOAD(&lifecycle_log.count) == 2 &&
        lifecycle_log.events[0] == VM_SESSION_RUNNING &&
        lifecycle_log.events[1] == VM_SESSION_PAUSED;
    vm_session_stop(session);
    passed = passed && STD_ATOMIC_LOAD(&lifecycle_log.count) == 3 &&
        lifecycle_log.events[2] == VM_SESSION_STOPPED;

done:
    vm_session_stop(session);
    if (task != STD_NULL) host_sync_task_destroy(task);
    if (run.complete != STD_NULL) host_sync_event_destroy(run.complete);
    vm_session_destroy(session);
    return passed;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (!vm_console_pause_resume_run_bound_console_case() ||
        !vm_console_pause_resume_run_window_close_case() ||
        vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_control_reset(&session->control) != TYPE_STATUS_OK ||
        vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait_for_state(session, 0)) {
        failed = 1;
        goto done;
    }
    vm_platform_run_handle_report(session->platform_run_handle,
        VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
    if (!vm_console_pause_resume_wait_for_state(session, 1) ||
        !vm_session_state_is_active(session->control.state)) {
        failed = 1;
        goto done;
    }
    if (vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait_for_state(session, 0)) {
        failed = 1;
        goto done;
    }
    vm_session_stop(session);
    if (vm_session_control_is_running(&session->control) ||
        vm_session_control_is_paused(&session->control)) {
        failed = 1;
    }

done:
    vm_session_stop(session);
    vm_session_destroy(session);
    return failed;
}
