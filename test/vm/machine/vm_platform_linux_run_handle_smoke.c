#include "type.h"

#include <pthread.h>

#include "core/platform/sleep.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/platform.h"

static C_VOID *run_session(C_VOID *opaque)
{
    vm_session_start((vm_session *)opaque);
    return STD_NULL;
}

static C_INT run_and_stop(vm_session *session)
{
    pthread_t thread;
    C_UINT waited;

    if (pthread_create(&thread, STD_NULL, run_session, session) != 0) return 0;
    for (waited = 0u; waited < 5000u; ++waited) {
        if (vm_platform_run_handle_is_active(session->platform_run_handle)) break;
        core_platform_sleep_milliseconds(1u);
    }
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) {
        (C_VOID)pthread_join(thread, STD_NULL);
        return 0;
    }
    vm_platform_run_handle_report(session->platform_run_handle,
        VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    if (pthread_join(thread, STD_NULL) != 0) return 0;
    return !vm_platform_run_handle_is_active(session->platform_run_handle);
}

int main(void)
{
    vm_session *session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    C_INT passed = 0;

    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (!session->active) goto done;
    if (!run_and_stop(session) || !run_and_stop(session)) goto done;
    passed = 1;

done:
    vm_session_stop(session);
    vm_session_finalize(session);
    STD_FREE(session);
    if (!passed) return 1;
    STD_PRINTF("M5:T196:S2:LINUX-RUN-HANDLE:OK\n");
    return 0;
}
