#include "lib/session/executor.h"

typedef struct session_executor_smoke_state {
    lib_session_lifecycle lifecycle;
    int starts;
    int stops;
} session_executor_smoke_state;

static void session_executor_smoke_start(void *context)
{
    session_executor_smoke_state *state = context;

    state->starts += 1u;
    state->lifecycle = LIB_SESSION_RUNNING;
}

static void session_executor_smoke_stop(void *context)
{
    session_executor_smoke_state *state = context;

    state->stops += 1u;
    state->lifecycle = LIB_SESSION_STOPPED;
}

int main(void)
{
    session_executor_smoke_state state = {0};
    session_executor_smoke_state second_state = {0};
    lib_session_executor_sink sink = {
        session_executor_smoke_start,
        session_executor_smoke_stop
    };
    lib_session_state *first_lifecycle = LIB_NULL;
    lib_session_state *second_lifecycle = LIB_NULL;
    lib_session_executor *executor = LIB_NULL;
    lib_session_executor *second_executor = LIB_NULL;

    if (lib_session_state_create(&first_lifecycle) != LIB_STATUS_OK ||
        lib_session_state_create(&second_lifecycle) != LIB_STATUS_OK ||
        lib_session_executor_create(first_lifecycle, &sink, &state, &executor) !=
            LIB_STATUS_OK || lib_session_executor_create(second_lifecycle, &sink,
            &second_state, &second_executor) != LIB_STATUS_OK) goto fail;
    lib_session_executor_start(executor, LIB_NULL, LIB_NULL);
    lib_session_executor_start(second_executor, LIB_NULL, LIB_NULL);
    lib_session_executor_stop(executor);
    if (lib_session_executor_is_running(executor) ||
        !lib_session_executor_is_running(second_executor) || state.stops != 1u ||
        second_state.starts != 1u) goto fail;
    lib_session_state_request_pause(second_lifecycle, 1u);
    lib_session_state_acknowledge_pause(second_lifecycle);
    if (lib_session_executor_is_running(second_executor) ||
        lib_session_executor_get_lifecycle(second_executor) != LIB_SESSION_PAUSED ||
        lib_session_executor_get_lifecycle(executor) != LIB_SESSION_STOPPED)
        goto fail;

    lib_session_executor_destroy(second_executor);
    lib_session_executor_destroy(executor);
    lib_session_state_destroy(second_lifecycle);
    lib_session_state_destroy(first_lifecycle);
    puts("M5:T523:S4:SESSION-EXECUTOR:OK");
    return 0;

fail:
    lib_session_executor_destroy(second_executor);
    lib_session_executor_destroy(executor);
    lib_session_state_destroy(second_lifecycle);
    lib_session_state_destroy(first_lifecycle);
    return 1;
}
