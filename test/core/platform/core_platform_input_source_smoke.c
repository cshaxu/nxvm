#include "type.h"




#include "core/platform/input_interface.h"

typedef struct core_platform_input_smoke_state {
    core_platform_input_event event;
    C_UINT submissions;
} core_platform_input_smoke_state;

static C_VOID core_platform_input_smoke_submit(C_VOID *context,
    const core_platform_input_event *event)
{
    core_platform_input_smoke_state *state = context;

    if (state == STD_NULL || event == STD_NULL) return;
    state->event = *event;
    ++state->submissions;
}

C_INT main(C_VOID)
{
    core_platform_input_smoke_state first = {0};
    core_platform_input_smoke_state second = {0};
    core_platform_input_sink sink = {
        core_platform_input_smoke_submit
    };
    core_platform_input_source *source = STD_NULL;
    core_platform_input_source *second_source = STD_NULL;
    core_platform_input_event event = {0};

    if (core_platform_input_source_create(&sink, &first, &source) != TYPE_STATUS_OK ||
        core_platform_input_source_create(&sink, &second, &second_source) != TYPE_STATUS_OK)
        return 1;
    event.kind = CORE_PLATFORM_INPUT_KEY;
    event.data.key.scan_code = 0x1234u;
    event.data.key.virtual_key = 0x5678u;
    event.data.key.pressed = TYPE_FALSE;
    if (core_platform_input_source_submit(second_source, &event) !=
        TYPE_STATUS_OK || second.submissions != 1u ||
        second.event.kind != CORE_PLATFORM_INPUT_KEY ||
        second.event.data.key.scan_code != 0x1234u ||
        second.event.data.key.pressed != TYPE_FALSE || first.submissions != 0u) {
        return 1;
    }
    core_platform_input_source_destroy(second_source);
    event.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
    event.data.relative_mouse.delta_x = 5;
    event.data.relative_mouse.delta_y = -3;
    event.data.relative_mouse.buttons = 0x01u;
    if (core_platform_input_source_submit(source, &event) != TYPE_STATUS_OK ||
        first.submissions != 1u ||
        first.event.kind != CORE_PLATFORM_INPUT_RELATIVE_MOUSE ||
        first.event.data.relative_mouse.delta_x != 5 ||
        first.event.data.relative_mouse.delta_y != -3 ||
        first.event.data.relative_mouse.buttons != 0x01u) return 1;
    core_platform_input_source_destroy(source);
    puts("M5:T249:S2:CORE-PLATFORM-INPUT:OK");
    return 0;
}
