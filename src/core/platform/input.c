#include "type.h"

#include "core/platform/input_interface.h"

struct core_platform_input_source {
    STD_ATOMIC_FLAG lock;
    C_INT accepting;
    const core_platform_input_sink *sink;
    C_VOID *context;
};

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

type_status core_platform_input_source_create(const core_platform_input_sink *sink,
    C_VOID *context, core_platform_input_source **out_source)
{
    core_platform_input_source *source;

    if (sink == STD_NULL || out_source == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_source = STD_NULL;
    source = STD_MALLOC(sizeof(*source));
    if (source == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&source->lock, STD_MEMORY_ORDER_RELEASE);
    source->accepting = TYPE_TRUE;
    source->sink = sink;
    source->context = context;
    *out_source = source;
    return TYPE_STATUS_OK;
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

C_VOID core_platform_input_source_destroy(core_platform_input_source *source)
{
    if (source == STD_NULL) return;
    core_platform_input_source_lock(source);
    source->accepting = TYPE_FALSE;
    source->sink = STD_NULL;
    source->context = STD_NULL;
    core_platform_input_source_unlock(source);
    STD_FREE(source);
}
