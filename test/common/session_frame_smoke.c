#include "common/session/session_interface.h"
#include <assert.h>
#include <string.h>

static common_machine_frame published;
static lib_u32 current_run = 7u, published_run = 7u, copies, deliveries;
static lib_u32 delivered_sequence;
static lib_bool delivered_graphics, delivered_status;
static lib_u32 monitor_calls;
static lib_u32 fake_run(const common_machine *machine)
{ (void)machine; return current_run; }
static lib_bool fake_copy(common_machine *machine, common_machine_frame *frame, lib_u32 run)
{
    (void)machine;
    ++copies;
    if (!published.window.valid || run != published_run) return LIB_FALSE;
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
lib_status common_ui_cancel_monitor_line(common_ui *ui, lib_bool *out_completed)
{ (void)ui; *out_completed = LIB_FALSE; return LIB_STATUS_OK; }
lib_status common_ui_release_window_mouse(common_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status common_ui_publish_frame(common_ui *ui, const kvm_window_frame *frame,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool window, lib_bool console, lib_bool status)
{
    (void)ui;
    /* An old Window can remain actual until its queued destruction completion;
     * graphics, however, must never publish before Window creation completes. */
    assert(console && (!frame->graphics || window));
    if (frame->graphics) {
        /* Delayed notification A receives full B, including A's earlier pixels. */
        assert(frame->image.width == 4 && frame->image.height == 4);
        assert(frame->image.pixels[0] == 1 && frame->image.pixels[15] == 2);
        assert(frame->image.palette[1] == 0x123456);
    }
    ++deliveries;
    delivered_sequence = sequence;
    (void)characters;
    delivered_graphics = frame->graphics != 0u;
    delivered_status = status;
    return LIB_STATUS_OK;
}
static void monitor(void *context, lib_bool current, common_session_command_result *result)
{
    (void)context;
    ++monitor_calls;
    /* The raw VM Console owns host input here. Product callbacks must receive
       this false fact and must not arm or write a cooked monitor prompt. */
    assert(!current);
    assert(!result->arm_prompt);
}

int main(void)
{
    static common_session session;
    common_session_event event = { 0 };
    common_session *output = &session;
    common_session_options options = { 0 };
    assert(common_session_create(NULL, NULL) == LIB_STATUS_INVALID_ARGUMENT);
    assert(common_session_create(&output, NULL) == LIB_STATUS_INVALID_ARGUMENT && output == NULL);
    output = &session;
    assert(common_session_create(&output, &options) == LIB_STATUS_INVALID_ARGUMENT && output == NULL);
    /* Opaque handles are never dereferenced by the fakes. */
    session.machine = (common_machine *)&current_run;
    session.ui = (common_ui *)&current_run;
    session.command.note_monitor_current = monitor;
    common_session_state_initialize(&session.state, COMMON_SESSION_DISPLAY_CONSOLE, 0);
    common_session_state_note_runtime(&session.state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_vm_console(&session.state, 1);
    common_session_state_note_current_console(&session.state, 1);
    assert(common_session_state_note_frame(&session.state, 1u, 0));
    session.frame.window.valid = 1u;
    session.frame.sequence = 1u;

    published.window.valid = 1u;
    published.sequence = 3u;
    published.window.graphics = 1u;
    published.window.image.width = published.window.image.stride = 4;
    published.window.image.height = 4;
    published.window.image.pixels[0] = 1; /* A changed the first corner. */
    published.window.image.pixels[15] = 2; /* B changed the opposite corner. */
    published.window.image.palette[1] = 0x123456;
    event.kind = COMMON_SESSION_EVENT_FRAME_COMPLETED;
    event.run_generation = 7u;
    event.value.frame.sequence = 2u;
    event.value.frame.graphics = 0; /* An earlier text frame. */
    assert(common_session_process_completed(&session, &event));
    assert(copies == 1u && session.state.observed_frame_sequence == 3u);
    assert(monitor_calls == 0u); /* Frames never request a monitor input turn. */
    assert(session.state.graphics_actual);
    assert(session.state.in_flight == COMMON_UI_ACTION_CREATE_WINDOW);
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
    published.window.valid = 0u;
    assert(common_session_process_completed(&session, &event));
    assert(session.frame.window.valid && session.frame.sequence == 3u &&
        session.state.graphics_actual);
    published.window.valid = 1u;
    published_run = 8u; /* Reset advanced between notification and snapshot. */
    assert(common_session_process_completed(&session, &event));
    assert(session.frame.sequence == 3u && session.state.observed_frame_sequence == 3u);
    current_run = 8u;
    { lib_u32 prior = copies;
      assert(common_session_process_completed(&session, &event));
      assert(copies == prior); } /* Old-run event rejected before snapshot access. */
    current_run = published_run = 7u;
    published.sequence = 5u;
    published.window.graphics = 0u;
    assert(common_session_process_completed(&session, &event));
    assert(session.state.observed_frame_sequence == 5u &&
        !session.state.graphics_actual);
    assert(session.state.in_flight == COMMON_UI_ACTION_DESTROY_WINDOW);
    event.kind = COMMON_SESSION_EVENT_COMPONENT_COMPLETED;
    event.value.component.component = COMMON_SESSION_EVENT_COMPONENT_WINDOW;
    event.value.component.exists = 0;
    assert(common_session_process_completed(&session, &event));
    assert(delivered_sequence == 5u && !delivered_graphics && !delivered_status);
    return 0;
}
