#include "type.h"

#include <windows.h>

#include "vm/composition/session/session.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/platform/platform.h"

int main(void)
{
    vm_session *session;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (!session->active) goto fail;
    vm_platform_run_context_set_window_display(session->platform_run_context, 1);
    vm_session_start(session);
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    Sleep(50u);
    vm_session_stop(session);
    if (vm_platform_run_handle_is_active(session->platform_run_handle) ||
        session->platform_run_handle->backend != STD_NULL) goto fail;
    vm_session_start(session);
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    vm_session_stop(session);
    if (vm_platform_run_handle_is_active(session->platform_run_handle) ||
        session->platform_run_handle->backend != STD_NULL) goto fail;
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T139:S1:RUN-HANDLE:OK");
    return 0;

fail:
    vm_session_stop(session);
    vm_session_finalize(session);
    STD_FREE(session);
    return 1;
}
