#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "core/machine/machine_private.h"
#include "test/app-nxvm/integration/support/session_ini.h"
#include "product/debug/protocol_interface.h"

static lib_i32 debug_trace_one(integration_ini_session *session)
{
    emulator_machine_debug_lease lease;
    product_debug_request request = {
        .operation = PRODUCT_DEBUG_SET_EXECUTION_PLAN,
        .execution_kind = PRODUCT_DEBUG_EXECUTION_TRACE,
        .instruction_count = 1u
    };
    product_debug_response response;
    lib_size response_size = 0u;

    if (session == LIB_NULL || session->emulator_machine == LIB_NULL ||
        emulator_machine_debug_acquire(session->emulator_machine, &lease) != LIB_STATUS_OK ||
        emulator_machine_debug_execute_with_lease(session->emulator_machine, &lease,
            &request, sizeof(request), &response, sizeof(response),
            &response_size) != LIB_STATUS_OK || response_size != sizeof(response) ||
        integration_ini_session_resume(session, 2000u) != LIB_STATUS_OK ||
        integration_ini_session_wait_for_state(session, EMULATOR_MACHINE_PAUSED,
            2000u) == 0 ||
        emulator_machine_debug_acquire(session->emulator_machine, &lease) != LIB_STATUS_OK) {
        return 0;
    }
    request.operation = PRODUCT_DEBUG_GET_EXECUTION_RESULT;
    request.execution_kind = PRODUCT_DEBUG_EXECUTION_NONE;
    request.instruction_count = 0u;
    response_size = 0u;
    return emulator_machine_debug_execute_with_lease(session->emulator_machine, &lease,
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
            EMULATOR_MACHINE_RUNNING, 2000u) == 0 ||
        integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK) goto fail;
    integration_ini_session_close(&ini_session);
    puts("M5:T46:S1:UNIFIED-DEBUG-BACKEND:OK");
    return 0;

fail:
    integration_ini_session_close(&ini_session);
    return 1;
}
