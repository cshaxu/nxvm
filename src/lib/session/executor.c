#include "lib/base/base.h"

#include "lib/session/executor.h"

struct lib_session_executor {
    const lib_session_executor_sink *sink;
    void *context;
};

lib_status lib_session_executor_create(const lib_session_executor_sink *sink,
    void *context, lib_session_executor **out_executor)
{
    lib_session_executor *executor;

    if (out_executor == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_executor = LIB_NULL;
    executor = malloc(sizeof(*executor));
    if (executor == LIB_NULL) return LIB_STATUS_NO_MEMORY;
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
    return executor == LIB_NULL || executor->sink == LIB_NULL ||
        executor->sink->get_lifecycle == LIB_NULL ? LIB_SESSION_STOPPED :
        executor->sink->get_lifecycle(executor->context);
}

int lib_session_executor_get_flip(const lib_session_executor *executor)
{
    return executor == LIB_NULL || executor->sink == LIB_NULL ||
        executor->sink->get_flip == LIB_NULL ? 0 :
        executor->sink->get_flip(executor->context);
}

void lib_session_executor_start(const lib_session_executor *executor)
{
    if (executor != LIB_NULL && executor->sink != LIB_NULL &&
        executor->sink->start != LIB_NULL) executor->sink->start(executor->context);
}

void lib_session_executor_stop(const lib_session_executor *executor)
{
    if (executor != LIB_NULL && executor->sink != LIB_NULL &&
        executor->sink->stop != LIB_NULL) executor->sink->stop(executor->context);
}
