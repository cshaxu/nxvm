#include "type.h"

#include "lib/session/executor.h"

struct lib_session_executor {
    const lib_session_executor_sink *sink;
    C_VOID *context;
};

type_status lib_session_executor_create(const lib_session_executor_sink *sink,
    C_VOID *context, lib_session_executor **out_executor)
{
    lib_session_executor *executor;

    if (out_executor == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_executor = STD_NULL;
    executor = STD_MALLOC(sizeof(*executor));
    if (executor == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    executor->sink = sink;
    executor->context = context;
    *out_executor = executor;
    return TYPE_STATUS_OK;
}

C_VOID lib_session_executor_destroy(lib_session_executor *executor)
{
    STD_FREE(executor);
}

C_INT lib_session_executor_is_running(const lib_session_executor *executor)
{
    return lib_session_executor_get_lifecycle(executor) == LIB_SESSION_RUNNING;
}

lib_session_lifecycle lib_session_executor_get_lifecycle(
    const lib_session_executor *executor)
{
    return executor == STD_NULL || executor->sink == STD_NULL ||
        executor->sink->get_lifecycle == STD_NULL ? LIB_SESSION_STOPPED :
        executor->sink->get_lifecycle(executor->context);
}

C_INT lib_session_executor_get_flip(const lib_session_executor *executor)
{
    return executor == STD_NULL || executor->sink == STD_NULL ||
        executor->sink->get_flip == STD_NULL ? 0 :
        executor->sink->get_flip(executor->context);
}

C_VOID lib_session_executor_start(const lib_session_executor *executor)
{
    if (executor != STD_NULL && executor->sink != STD_NULL &&
        executor->sink->start != STD_NULL) executor->sink->start(executor->context);
}

C_VOID lib_session_executor_stop(const lib_session_executor *executor)
{
    if (executor != STD_NULL && executor->sink != STD_NULL &&
        executor->sink->stop != STD_NULL) executor->sink->stop(executor->context);
}
