#include "common/session/session_interface.h"

static lib_status sink(void *context, const ui_input_event *event)
{
    lib_u32 *count = context;
    if (event == LIB_NULL || count == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++*count;
    return LIB_STATUS_OK;
}

int main(void)
{
    common_session *session = LIB_NULL;
    common_session_fact fact = {0};
    common_session_plan plan = {0};
    ui_frame frame = {0};
    ui_input_event input = {0};
    lib_u32 delivered = 0u;
    lib_u32 index;

    if (common_session_create(&session) != LIB_STATUS_OK ||
        common_session_set_target(session, COMMON_SESSION_TARGET_WINDOW) != LIB_STATUS_OK ||
        common_session_begin_run(session, &plan) == 0u || !plan.target_changed ||
        plan.target != COMMON_SESSION_TARGET_WINDOW) return 1;
    if (common_session_publish_machine(session, COMMON_SESSION_MACHINE_RUNNING,
        LIB_STATUS_OK) != LIB_STATUS_OK || common_session_take(session, &fact, &frame,
        0u) != LIB_STATUS_OK || common_session_reduce_fact(session, &fact, &frame,
        &plan) != LIB_STATUS_OK || plan.notice != COMMON_SESSION_NOTICE_STARTED ||
        !plan.mouse_capturable) return 1;
    input.type = UI_EVENT_KEY;
    input.source_identity = 1u;
    input.data.key.key = UI_KEY_F1;
    input.data.key.pressed = LIB_TRUE;
    if (common_session_dispatch_host_input(session, &input, sink, &delivered) !=
        LIB_STATUS_OK || delivered != 1u) return 1;
    input.type = UI_EVENT_SOURCE_RETIRED;
    if (common_session_dispatch_host_input(session, &input, sink, &delivered) !=
        LIB_STATUS_OK || delivered != 2u) return 1;
    frame.valid = 1u;
    frame.text_columns = 80u;
    frame.text_rows = 25u;
    if (common_session_publish_frame(session, &frame) != LIB_STATUS_OK ||
        common_session_take(session, &fact, &frame, 0u) != LIB_STATUS_OK ||
        fact.kind != COMMON_SESSION_FACT_FRAME || common_session_reduce_fact(session,
            &fact, &frame, &plan) != LIB_STATUS_OK || !plan.frame_ready) return 1;
    fact.kind = COMMON_SESSION_FACT_MACHINE;
    fact.run_id = 0x12345678u;
    if (common_session_reduce_fact(session, &fact, &frame, &plan) !=
        LIB_STATUS_NOT_CURRENT) return 1;
    for (index = 0u; index < 64u; ++index)
        if (common_session_publish_console_line(session, "line") != LIB_STATUS_OK) return 1;
    if (common_session_publish_console_line(session, "overflow") != LIB_STATUS_LIMIT_EXCEEDED)
        return 1;
    common_session_close(session);
    if (common_session_publish_ui_input(session, &input) != LIB_STATUS_INVALID_STATE) return 1;
    common_session_destroy(session);
    return 0;
}
