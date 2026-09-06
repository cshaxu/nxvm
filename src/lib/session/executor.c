#include "lib/base/base.h"

#include "lib/session/executor.h"

struct lib_session_executor {
    lib_session_state *state;
    const lib_session_executor_sink *sink;
    void *context;
};

lib_status lib_session_executor_create(lib_session_state *state,
    const lib_session_executor_sink *sink, void *context,
    lib_session_executor **out_executor)
{
    lib_session_executor *executor;

    if (state == LIB_NULL || out_executor == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_executor = LIB_NULL;
    executor = malloc(sizeof(*executor));
    if (executor == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    executor->state = state;
    executor->sink = sink;
    executor->context = context;
    *out_executor = executor;
    return LIB_STATUS_OK;
}

void lib_session_executor_destroy(lib_session_executor *executor)
{
    free(executor);
}

int lib_session_executor_is_running(const lib_session_executor *executor)
{
    return lib_session_executor_get_lifecycle(executor) == LIB_SESSION_RUNNING;
}

lib_session_lifecycle lib_session_executor_get_lifecycle(
    const lib_session_executor *executor)
{
    return executor == LIB_NULL ? LIB_SESSION_STOPPED :
        lib_session_state_lifecycle(executor->state);
}

int lib_session_executor_get_flip(const lib_session_executor *executor)
{
    return executor == LIB_NULL ? 0 : lib_session_state_flip(executor->state);
}

void lib_session_executor_start(const lib_session_executor *executor,
    lib_session_executor_start_observer observer, void *observer_context)
{
    if (executor == LIB_NULL) return;
    lib_session_state_start(executor->state);
    if (observer != LIB_NULL) observer(observer_context);
    if (executor->sink != LIB_NULL && executor->sink->start != LIB_NULL) {
        executor->sink->start(executor->context);
    }
}

void lib_session_executor_stop(const lib_session_executor *executor)
{
    if (executor == LIB_NULL) return;
    if (executor->sink != LIB_NULL && executor->sink->stop != LIB_NULL) {
        executor->sink->stop(executor->context);
    }
    lib_session_state_stop(executor->state);
}
