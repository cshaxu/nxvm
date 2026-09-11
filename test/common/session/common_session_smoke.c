#include "common/session/session_interface.h"

static lib_status sink(void *context, const ui_input_event *event)
{
    lib_u32 *count = context;
    if (event == LIB_NULL || count == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++*count;
    return LIB_STATUS_OK;
}

static lib_status cli(void *context, const char *line,
    common_session_cli_result *out_result)
{
    lib_u32 *count = context;

    if (count == LIB_NULL || line == LIB_NULL || out_result == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++*count;
    if (line[0] != 'd' || line[1] != '\0') return LIB_STATUS_INVALID_ARGUMENT;
    out_result->text[0] = 'o';
    out_result->text[1] = 'k';
    out_result->prompt[0] = '-';
    out_result->prompt_ready = LIB_TRUE;
    out_result->keep_active = LIB_FALSE;
    out_result->lifecycle_request = COMMON_SESSION_LIFECYCLE_STEP;
    return LIB_STATUS_OK;
}

static lib_status lifecycle(void *context,
    common_session_lifecycle_request_kind request)
{
    lib_u32 *count = context;

    if (count == LIB_NULL || request != COMMON_SESSION_LIFECYCLE_STEP)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++*count;
    return LIB_STATUS_OK;
}

static lib_status cli_machine(void *context, common_session_machine_state state,
    lib_status status, common_session_cli_result *out_result)
{
    lib_u32 *count = context;

    if (count == LIB_NULL || out_result == LIB_NULL ||
        state != COMMON_SESSION_MACHINE_PAUSED || status != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++*count;
    out_result->text[0] = 'p';
    out_result->text[1] = 'a';
    out_result->text[2] = 'u';
    out_result->text[3] = 's';
    out_result->text[4] = 'e';
    out_result->prompt[0] = '-';
    out_result->prompt_ready = LIB_TRUE;
    out_result->keep_active = LIB_TRUE;
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
    lib_u32 cli_calls = 0u;
    lib_u32 lifecycle_calls = 0u;
    lib_u32 machine_calls = 0u;
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
    if (common_session_set_lifecycle_sink(session, lifecycle, &lifecycle_calls) !=
            LIB_STATUS_OK || common_session_set_cli_provider(session, cli,
                &cli_calls) != LIB_STATUS_OK || !common_session_has_cli_provider(session) ||
        common_session_publish_console_line(session, "d") != LIB_STATUS_OK ||
        common_session_take(session, &fact, &frame, 0u) != LIB_STATUS_OK ||
        common_session_reduce_fact(session, &fact, &frame, &plan) != LIB_STATUS_OK ||
        cli_calls != 1u || lifecycle_calls != 1u || plan.console_text[0] != 'o' ||
        !plan.console_prompt_ready || plan.console_prompt[0] != '-' ||
        common_session_has_cli_provider(session)) return 1;
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
    if (common_session_set_cli_machine_observer(session, cli_machine,
            &machine_calls) != LIB_STATUS_OK ||
        common_session_publish_machine(session, COMMON_SESSION_MACHINE_PAUSED,
            LIB_STATUS_OK) != LIB_STATUS_OK || common_session_take(session,
            &fact, &frame, 0u) != LIB_STATUS_OK || common_session_reduce_fact(
            session, &fact, &frame, &plan) != LIB_STATUS_OK ||
        machine_calls != 1u || plan.console_text[0] != 'p' ||
        !plan.console_prompt_ready || plan.console_prompt[0] != '-') return 1;
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
