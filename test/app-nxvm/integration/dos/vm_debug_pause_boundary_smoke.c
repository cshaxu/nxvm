#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/machine/machine_private.h"

#include "test/app-nxvm/integration/support/session_ini.h"
#include "x86/debug/protocol_interface.h"

static lib_i32 debug_trace_one(integration_ini_session *session)
{
    common_machine_debug_lease lease;
    x86_debug_request request = {
        .operation = X86_DEBUG_SET_EXECUTION_PLAN,
        .execution_kind = X86_DEBUG_EXECUTION_TRACE,
        .instruction_count = 1u
    };
    x86_debug_response response;
    lib_size response_size = 0u;

    if (session == LIB_NULL || session->common_machine == LIB_NULL ||
        common_machine_debug_acquire(session->common_machine, &lease) != LIB_STATUS_OK ||
        common_machine_debug_execute_with_lease(session->common_machine, &lease,
            &request, sizeof(request), &response, sizeof(response),
            &response_size) != LIB_STATUS_OK || response_size != sizeof(response) ||
        integration_ini_session_resume(session, 2000u) != LIB_STATUS_OK ||
        !integration_ini_session_wait_for_state(session, COMMON_MACHINE_PAUSED,
            2000u) ||
        common_machine_debug_acquire(session->common_machine, &lease) != LIB_STATUS_OK) {
        return 0;
    }
    request.operation = X86_DEBUG_GET_EXECUTION_RESULT;
    request.execution_kind = X86_DEBUG_EXECUTION_NONE;
    request.instruction_count = 0u;
    response_size = 0u;
    return common_machine_debug_execute_with_lease(session->common_machine, &lease,
        &request, sizeof(request), &response, sizeof(response), &response_size) ==
        LIB_STATUS_OK && response_size == sizeof(response) && response.enabled &&
        response.value == 1u;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK ||
        integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK ||
        integration_ini_session_reset(&ini_session, 2000u) != LIB_STATUS_OK ||
        !debug_trace_one(&ini_session) ||
        integration_ini_session_resume(&ini_session, 2000u) != LIB_STATUS_OK ||
        integration_ini_session_wait_for_state(&ini_session,
            COMMON_MACHINE_RUNNING, 2000u) == 0 ||
        integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK) goto fail;
    integration_ini_session_close(&ini_session);
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    integration_ini_session_close(&ini_session);
    return 1;
}
