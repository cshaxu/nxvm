#include "type.h"

#include "core/platform/input_interface.h"

static C_VOID core_platform_input_source_lock(
    core_platform_input_source *source)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&source->lock,
        STD_MEMORY_ORDER_ACQUIRE)) {
    }
}

static C_VOID core_platform_input_source_unlock(
    core_platform_input_source *source)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&source->lock, STD_MEMORY_ORDER_RELEASE);
}

C_VOID core_platform_input_source_initialize(core_platform_input_source *source,
    const core_platform_input_sink *sink, C_VOID *context)
{
    if (source == STD_NULL) return;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&source->lock, STD_MEMORY_ORDER_RELEASE);
    source->accepting = TYPE_TRUE;
    source->sink = sink;
    source->context = context;
}

type_status core_platform_input_source_submit(core_platform_input_source *source,
    const core_platform_input_event *event)
{
    type_status status = TYPE_STATUS_INVALID_STATE;

    if (source == STD_NULL || event == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    core_platform_input_source_lock(source);
    if (source->accepting && source->sink != STD_NULL &&
        source->sink->submit != STD_NULL) {
        source->sink->submit(source->context, event);
        status = TYPE_STATUS_OK;
    }
    core_platform_input_source_unlock(source);
    return status;
}

C_VOID core_platform_input_source_stop(core_platform_input_source *source)
{
    if (source == STD_NULL) return;
    core_platform_input_source_lock(source);
    source->accepting = TYPE_FALSE;
    source->sink = STD_NULL;
    source->context = STD_NULL;
    core_platform_input_source_unlock(source);
}
