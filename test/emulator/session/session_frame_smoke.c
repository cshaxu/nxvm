#include "lib/types/test.h"
#include "emulator/session/session_interface.h"

static kvm_window_frame_storage published;
static lib_bool publication_ready;
static lib_u32 current_run = 7u, published_run = 7u, copies, deliveries;
static lib_u32 delivered_sequence;
static lib_bool delivered_graphics, delivered_status;
static lib_u32 fake_run(const emulator_machine *machine)
{ (void)machine; return current_run; }
static lib_bool fake_read(emulator_machine *machine, lib_u32 run,
    lib_u32 *sequence, lib_bool *graphics)
{
    (void)machine;
    ++copies;
    if (!publication_ready || run != published_run) return LIB_FALSE;
    *sequence = published.frame.sequence;
    *graphics = published.frame.graphics != LIB_NULL;
    return LIB_TRUE;
}
static kvm_frame_exchange *fake_exchange(emulator_machine *machine)
{ (void)machine; return (kvm_frame_exchange *)&published; }

/* Run the production completion consumer with publication independently ahead
 * of its queued notification. No timing guess or second reducer is involved. */
#define emulator_machine_run_generation fake_run
#define emulator_machine_read_presentation fake_read
#define emulator_machine_frame_exchange fake_exchange
#define emulator_machine_console_exchange fake_exchange
#include "emulator/session/session.c"
#undef emulator_machine_run_generation
#undef emulator_machine_read_presentation
#undef emulator_machine_frame_exchange
#undef emulator_machine_console_exchange

void emulator_ui_set_run_generation(emulator_ui *ui, lib_u32 run)
{ (void)ui; lib_test_assert(run == current_run); }
lib_status emulator_ui_apply_action(emulator_ui *ui, emulator_ui_action action, emulator_ui_state state)
{ (void)ui; (void)action; (void)state; return LIB_STATUS_OK; }
lib_status emulator_ui_set_state(emulator_ui *ui, emulator_ui_state state)
{ (void)ui; (void)state; return LIB_STATUS_OK; }
lib_status emulator_ui_write_monitor(emulator_ui *ui, const char *text)
{ (void)ui; (void)text; return LIB_STATUS_OK; }
lib_status emulator_ui_request_monitor_line(emulator_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status emulator_ui_cancel_monitor_line(emulator_ui *ui, lib_bool *out_completed)
{ (void)ui; *out_completed = LIB_FALSE; return LIB_STATUS_OK; }
lib_status emulator_ui_release_window_mouse(emulator_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status emulator_ui_publish_frame(emulator_ui *ui, kvm_frame_exchange *frames,
    kvm_frame_exchange *console_frames, lib_bool graphics, lib_u32 sequence,
    lib_bool window, lib_bool console, lib_bool status)
{
    (void)ui;
    lib_test_assert(frames == (kvm_frame_exchange *)&published);
    lib_test_assert(console_frames == frames);
    const kvm_window_frame *frame = &published.frame;
    lib_test_assert(graphics == (frame->graphics != LIB_NULL));
    /* An old Window can remain actual until its queued destruction completion;
     * graphics, however, must never publish before Window creation completes. */
    lib_test_assert(console && (!frame->graphics || window));
    if (frame->graphics) {
        /* Delayed notification A receives full B, including A's earlier pixels. */
        lib_test_assert(frame->graphics->width == 4 && frame->graphics->height == 4);
        lib_test_assert(frame->graphics->pixels[0] == 0x123456 && frame->graphics->pixels[15] == 0xabcdef);
    }
    ++deliveries;
    delivered_sequence = sequence;
    delivered_graphics = frame->graphics != LIB_NULL;
    delivered_status = status;
    return LIB_STATUS_OK;
}
int main(void)
{
    static emulator_session session;
    emulator_session_event event = { 0 };
    emulator_session *output = &session;
    emulator_session_options options = { 0 };
    lib_test_assert(emulator_session_create(LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_session_create(&output, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && output == LIB_NULL);
    output = &session;
    lib_test_assert(emulator_session_create(&output, &options) == LIB_STATUS_INVALID_ARGUMENT && output == LIB_NULL);
    /* Opaque handles are never dereferenced by the fakes. */
    session.machine = (emulator_machine *)&current_run;
    session.ui = (emulator_ui *)&current_run;
    emulator_session_state_initialize(&session.state, EMULATOR_SESSION_DISPLAY_CONSOLE, 0);
    emulator_session_state_note_runtime(&session.state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_vm_console(&session.state, 1);
    emulator_session_state_note_current_console(&session.state, 1);
    lib_test_assert(emulator_session_state_note_frame(&session.state, 1u, 0));

    publication_ready = LIB_TRUE;
    published.frame.sequence = 3u;
    (void)kvm_window_frame_begin_graphics(&published);
    published.data.graphics.width = published.data.graphics.stride = 4;
    published.data.graphics.height = 4;
    published.data.graphics.pixels[0] = 0x123456; /* A changed the first corner. */
    published.data.graphics.pixels[15] = 0xabcdef; /* B changed the opposite corner. */
    event.kind = EMULATOR_SESSION_EVENT_FRAME_COMPLETED;
    event.run_generation = 7u;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(copies == 1u && session.state.observed_frame_sequence == 3u);
    lib_test_assert(session.state.graphics_actual);
    lib_test_assert(session.state.in_flight == EMULATOR_UI_ACTION_CREATE_WINDOW);
    lib_test_assert(deliveries == 0u); /* Wait for component completion before publishing. */
    event.kind = EMULATOR_SESSION_EVENT_COMPONENT_COMPLETED;
    event.value.component.component = EMULATOR_SESSION_EVENT_COMPONENT_WINDOW;
    event.value.component.exists = 1;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(deliveries == 1u && delivered_sequence == 3u &&
        delivered_graphics && delivered_status);

    event.kind = EMULATOR_SESSION_EVENT_FRAME_COMPLETED;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(delivered_sequence == 3u); /* Reobserving never regresses content. */
    publication_ready = LIB_FALSE;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(session.state.observed_frame_sequence == 3u &&
        session.state.graphics_actual);
    publication_ready = LIB_TRUE;
    published_run = 8u; /* Reset advanced between notification and snapshot. */
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(session.state.observed_frame_sequence == 3u);
    current_run = 8u;
    { lib_u32 prior = copies;
      lib_test_assert(emulator_session_process_completed(&session, &event));
      lib_test_assert(copies == prior); } /* Old-run event rejected before snapshot access. */
    current_run = published_run = 7u;
    published.frame.sequence = 5u;
    (void)kvm_window_frame_begin_text(&published);
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(session.state.observed_frame_sequence == 5u &&
        !session.state.graphics_actual);
    lib_test_assert(session.state.in_flight == EMULATOR_UI_ACTION_DESTROY_WINDOW);
    event.kind = EMULATOR_SESSION_EVENT_COMPONENT_COMPLETED;
    event.value.component.component = EMULATOR_SESSION_EVENT_COMPONENT_WINDOW;
    event.value.component.exists = 0;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(delivered_sequence == 5u && !delivered_graphics && !delivered_status);
    /* First consumption may already be near wrap; zero is never a frame. */
    {
        emulator_session_state state;
        emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_WINDOW, 1);
        lib_test_assert(!emulator_session_state_note_frame(&state, 0u, 1));
        lib_test_assert(emulator_session_state_note_frame(&state, LIB_UINT32_MAX, 0));
        lib_test_assert(emulator_session_state_note_frame(&state, 1u, 1));
        lib_test_assert(!emulator_session_state_note_frame(&state, LIB_UINT32_MAX, 0));
        lib_test_assert(!emulator_session_state_note_frame(&state, 1u, 0));
        lib_test_assert(!emulator_session_state_note_frame(&state, 0u, 0));
        lib_test_assert(state.observed_frame_sequence == 1u && state.graphics_actual);
        /* Half-range is ambiguous and must not be accepted as newer. */
        lib_test_assert(!emulator_session_state_note_frame(&state, 0x80000001u, 0));
        lib_test_assert(emulator_session_state_note_frame(&state, 0x80000000u, 0));
    }
    session.state.observed_frame_sequence = LIB_UINT32_MAX - 1u;
    published.frame.sequence = 1u;
    event.kind = EMULATOR_SESSION_EVENT_FRAME_COMPLETED;
    lib_u32 before = copies;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(copies == before + 1u && session.state.observed_frame_sequence == 1u);
    lib_test_assert(session.state.observed_frame_sequence == 1u && delivered_sequence == 1u);
    /* Repeated wake hints always inspect the same latest publication. */
    for (lib_u32 i = 0u; i < 4u; ++i) {
        lib_test_assert(emulator_session_process_completed(&session, &event));
        lib_test_assert(copies == before + 2u + i && session.state.observed_frame_sequence == 1u);
    }
    before = copies;
    current_run = published_run = 8u;
    published.frame.sequence = 2u;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(copies == before); /* Old run cannot cross the wrap boundary. */
    emulator_session_state_note_runtime(&session.state, EMULATOR_SESSION_MACHINE_STOPPED);
    emulator_session_state_note_runtime(&session.state, EMULATOR_SESSION_MACHINE_RUNNING);
    event.run_generation = current_run;
    lib_test_assert(emulator_session_process_completed(&session, &event));
    lib_test_assert(copies == before + 1u && session.state.observed_frame_sequence == 2u);
    return 0;
}
