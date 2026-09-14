#include "common/session/session_interface.h"
#include <assert.h>
#include <string.h>

static kvm_frame published;
static lib_u32 current_run = 7u, published_run = 7u, copies, deliveries;
static lib_u32 delivered_sequence;
static lib_bool delivered_graphics, delivered_status;
static lib_u32 fake_run(const common_machine *machine)
{ (void)machine; return current_run; }
static lib_bool fake_copy(common_machine *machine, kvm_frame *frame, lib_u32 run)
{
    (void)machine;
    ++copies;
    if (!published.valid || run != published_run) return LIB_FALSE;
    *frame = published;
    return LIB_TRUE;
}

/* Run the production completion consumer with publication independently ahead
 * of its queued notification. No timing guess or second reducer is involved. */
#define common_machine_run_generation fake_run
#define common_machine_copy_published_frame fake_copy
#include "common/session/session.c"
#undef common_machine_run_generation
#undef common_machine_copy_published_frame

void common_ui_set_run_generation(common_ui *ui, lib_u32 run)
{ (void)ui; assert(run == current_run); }
lib_status common_ui_apply_action(common_ui *ui, common_ui_action action, common_ui_state state)
{ (void)ui; (void)action; (void)state; return LIB_STATUS_OK; }
lib_status common_ui_set_state(common_ui *ui, common_ui_state state)
{ (void)ui; (void)state; return LIB_STATUS_OK; }
lib_status common_ui_write_monitor(common_ui *ui, const char *text)
{ (void)ui; (void)text; return LIB_STATUS_OK; }
lib_status common_ui_request_monitor_line(common_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status common_ui_release_window_mouse(common_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status common_ui_publish_frame(common_ui *ui, const kvm_frame *frame,
    lib_bool window, lib_bool console, lib_bool status)
{
    (void)ui;
    /* An old Window can remain actual until its queued destruction completion;
     * graphics, however, must never publish before Window creation completes. */
    assert(console && (!frame->graphics || window));
    ++deliveries;
    delivered_sequence = frame->sequence;
    delivered_graphics = frame->graphics != 0u;
    delivered_status = status;
    return LIB_STATUS_OK;
}
static void monitor(void *context, lib_bool current, common_session_command_result *result)
{ (void)context; (void)current; (void)result; }

int main(void)
{
    static common_session session;
    common_session_event event = { 0 };
    /* Opaque handles are never dereferenced by the fakes. */
    session.machine = (common_machine *)&current_run;
    session.ui = (common_ui *)&current_run;
    session.command.note_monitor_current = monitor;
    common_session_state_initialize(&session.state, COMMON_SESSION_DISPLAY_CONSOLE, 0);
    common_session_state_note_runtime(&session.state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_vm_console(&session.state, 1);
    common_session_state_note_current_console(&session.state, 1);
    assert(common_session_state_note_frame(&session.state, 1u, 0));
    session.frame.valid = 1u;
    session.frame.sequence = 1u;

    published.valid = 1u;
    published.sequence = 3u;
    published.graphics = 1u;
    event.kind = COMMON_SESSION_EVENT_FRAME_COMPLETED;
    event.run_generation = 7u;
    event.value.frame.sequence = 2u;
    event.value.frame.graphics = 0; /* An earlier text frame. */
    assert(common_session_process_completed(&session, &event));
    assert(copies == 1u && session.state.observed_frame_sequence == 3u);
    assert(session.state.presentation.graphics_actual);
    assert(session.state.presentation.in_flight == COMMON_UI_ACTION_CREATE_WINDOW);
    assert(deliveries == 0u); /* Wait for component completion before publishing. */
    event.kind = COMMON_SESSION_EVENT_COMPONENT_COMPLETED;
    event.value.component.component = COMMON_SESSION_EVENT_COMPONENT_WINDOW;
    event.value.component.exists = 1;
    assert(common_session_process_completed(&session, &event));
    assert(deliveries == 1u && delivered_sequence == 3u &&
        delivered_graphics && delivered_status);

    event.kind = COMMON_SESSION_EVENT_FRAME_COMPLETED;
    event.value.frame.sequence = 3u;
    assert(common_session_process_completed(&session, &event));
    assert(copies == 1u); /* Duplicate notification does not copy again. */
    event.value.frame.sequence = 4u;
    published.valid = 0u;
    assert(common_session_process_completed(&session, &event));
    assert(session.frame.valid && session.frame.sequence == 3u &&
        session.state.presentation.graphics_actual);
    published.valid = 1u;
    published_run = 8u; /* Reset advanced between notification and snapshot. */
    assert(common_session_process_completed(&session, &event));
    assert(session.frame.sequence == 3u && session.state.observed_frame_sequence == 3u);
    current_run = 8u;
    { lib_u32 prior = copies;
      assert(common_session_process_completed(&session, &event));
      assert(copies == prior); } /* Old-run event rejected before snapshot access. */
    current_run = published_run = 7u;
    published.sequence = 5u;
    published.graphics = 0u;
    assert(common_session_process_completed(&session, &event));
    assert(session.state.observed_frame_sequence == 5u &&
        !session.state.presentation.graphics_actual);
    assert(session.state.presentation.in_flight == COMMON_UI_ACTION_DESTROY_WINDOW);
    event.kind = COMMON_SESSION_EVENT_COMPONENT_COMPLETED;
    event.value.component.component = COMMON_SESSION_EVENT_COMPONENT_WINDOW;
    event.value.component.exists = 0;
    assert(common_session_process_completed(&session, &event));
    assert(delivered_sequence == 5u && !delivered_graphics && !delivered_status);
    return 0;
}
