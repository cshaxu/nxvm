#include "type.h"

#include "lib/session/executor.h"

typedef struct session_executor_smoke_state {
    lib_session_lifecycle lifecycle;
    C_INT flip;
    C_UINT starts;
    C_UINT stops;
} session_executor_smoke_state;

static lib_session_lifecycle session_executor_smoke_get_lifecycle(C_VOID *context)
{
    return ((session_executor_smoke_state *)context)->lifecycle;
}

static C_INT session_executor_smoke_get_flip(C_VOID *context)
{
    return ((session_executor_smoke_state *)context)->flip;
}

static C_VOID session_executor_smoke_start(C_VOID *context)
{
    session_executor_smoke_state *state = context;

    state->starts += 1u;
    state->lifecycle = LIB_SESSION_RUNNING;
    state->flip = 1;
}

static C_VOID session_executor_smoke_stop(C_VOID *context)
{
    session_executor_smoke_state *state = context;

    state->stops += 1u;
    state->lifecycle = LIB_SESSION_STOPPED;
}

C_INT main(C_VOID)
{
    session_executor_smoke_state state = {0};
    session_executor_smoke_state second_state = {0};
    lib_session_executor_sink sink = {
        session_executor_smoke_get_lifecycle,
        session_executor_smoke_get_flip,
        session_executor_smoke_start,
        session_executor_smoke_stop
    };
    lib_session_executor *executor = STD_NULL;
    lib_session_executor *second_executor = STD_NULL;

    if (lib_session_executor_create(&sink, &state, &executor) != TYPE_STATUS_OK ||
        lib_session_executor_create(&sink, &second_state, &second_executor) !=
            TYPE_STATUS_OK) goto fail;
    lib_session_executor_start(executor);
    lib_session_executor_start(second_executor);
    lib_session_executor_stop(executor);
    if (lib_session_executor_is_running(executor) ||
        !lib_session_executor_is_running(second_executor) || state.stops != 1u ||
        second_state.starts != 1u) goto fail;
    second_state.lifecycle = LIB_SESSION_PAUSED;
    if (lib_session_executor_is_running(second_executor) ||
        lib_session_executor_get_lifecycle(second_executor) != LIB_SESSION_PAUSED ||
        lib_session_executor_get_lifecycle(executor) != LIB_SESSION_STOPPED)
        goto fail;

    lib_session_executor_destroy(second_executor);
    lib_session_executor_destroy(executor);
    puts("M5:T522:S8:SESSION-EXECUTOR:OK");
    return 0;

fail:
    lib_session_executor_destroy(second_executor);
    lib_session_executor_destroy(executor);
    return 1;
}
