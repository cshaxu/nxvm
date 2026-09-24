#include "common/session/session_interface.h"
#include "common/session/control.h"
#include <assert.h>
#include <string.h>

static lib_u32 requests, prompts, notices, callbacks, cancellations, commands;
static lib_bool completed, fail_cancel, fail_request, exit_on_request;
static lib_bool collect;
static char output[40000];
static lib_size output_used;
static lib_u32 writes, fail_write;
static lib_u32 waits;
static lib_bool fail_wait;
static lib_i32 take_event(common_session_queue *queue, common_session_event *event,
    lib_u32 timeout_ms)
{
    assert(timeout_ms == LIB_UINT32_MAX);
    ++waits;
    if (fail_wait) {
        assert(waits == 1u); /* A failed wait must never be retried. */
        return 0;
    }
    return common_session_queue_take(queue, event, timeout_ms);
}
static lib_u32 run(const common_machine *m) { (void)m; return 1; }
static lib_bool copy_frame(common_machine *m, common_machine_frame *f, lib_u32 g)
{ (void)m; (void)f; (void)g; return LIB_FALSE; }
static lib_i32 machine_request(common_machine *m) { (void)m; ++commands; return 1; }
#define common_machine_run_generation run
#define common_machine_copy_published_frame copy_frame
#define common_machine_start machine_request
#define common_machine_resume machine_request
#define common_machine_pause machine_request
#define common_machine_reset machine_request
#define common_machine_stop machine_request
#define common_session_queue_take take_event
#include "common/session/session.c"
#undef common_session_queue_take

static common_session *active;
lib_status common_ui_cancel_monitor_line(common_ui *ui, lib_bool *out_completed)
{ (void)ui; ++cancellations; *out_completed = completed;
  return fail_cancel ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK; }
lib_status common_ui_write_monitor(common_ui *ui, const char *text)
{
    (void)ui;
    if (collect) {
        if (++writes == fail_write) return LIB_STATUS_IO_ERROR;
        assert(output_used + strlen(text) < sizeof(output));
        memcpy(output + output_used, text, strlen(text) + 1u);
        output_used += strlen(text);
    }
    if (strcmp(text, "> ") == 0) ++prompts;
    else if (strcmp(text, "notice") == 0) ++notices;
    return LIB_STATUS_OK;
}
lib_status common_ui_request_monitor_line(common_ui *ui)
{
    (void)ui; ++requests;
    if (exit_on_request) {
        lib_console_line line = {0};
        memcpy(line.text, "exit", 5); line.length = 4;
        assert(common_session_queue_push_monitor_line(&active->queue, &line, 0));
    }
    return fail_request ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}
void common_ui_set_run_generation(common_ui *ui, lib_u32 generation)
{ (void)ui; assert(generation == 1); }
lib_status common_ui_apply_action(common_ui *ui, common_ui_action a, common_ui_state s)
{ (void)ui; (void)a; (void)s; return LIB_STATUS_OK; }
lib_status common_ui_set_state(common_ui *ui, common_ui_state s)
{ (void)ui; (void)s; return LIB_STATUS_OK; }
lib_status common_ui_release_window_mouse(common_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status common_ui_publish_frame(common_ui *ui, const kvm_window_frame *f,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool w, lib_bool c, lib_bool status)
{ (void)characters; (void)sequence; (void)ui; (void)f; (void)w; (void)c; (void)status; return LIB_STATUS_OK; }
static void monitor(void *p, lib_bool current, common_session_command_result *out)
{
    (void)p; (void)current; ++callbacks;
    /* Even an eager provider cannot create two outstanding monitor turns. */
    out->arm_prompt = LIB_TRUE;
    memcpy(out->prompt, "> ", 3);
}
static void opened(void *p, common_session_command_result *out) { (void)p; (void)out; }
static void rejected(void *p, common_session_command_result *out)
{
    (void)p;
    assert(!active->pending_line);
    memcpy(out->text, "rejected", 9);
    exit_on_request = LIB_TRUE;
}
static void submitted(void *p, common_session_machine_state state, const char *line,
    common_session_command_result *out)
{
    (void)p; (void)state;
    assert(!active->pending_line && strcmp(line, "exit") == 0);
    out->exit_requested = LIB_TRUE;
}
static void runtime(void *p, common_session_machine_state a,
    common_session_machine_state b, common_session_command_result *out)
{
    (void)p; (void)a; (void)b;
    memcpy(out->text, "notice", 7);
    out->request = COMMON_SESSION_REQUEST_PAUSE;
}
int main(void)
{
    static common_session s;
    common_session_event event = {0};
    common_session_command_result notice = {0};
    lib_console_line line = {0};
    active = &s;
    s.machine = (common_machine *)&s; s.ui = (common_ui *)&s;
    s.command.note_monitor_current = monitor;
    s.command.note_runtime = runtime;
    s.command.open = opened; s.command.reject_line = rejected;
    s.command.submit_line = submitted;
    assert(common_session_queue_initialize(&s.queue));
    common_session_state_initialize(&s.state, COMMON_SESSION_DISPLAY_WINDOW, 1);
    common_session_state_note_runtime(&s.state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_window(&s.state, 1);
    assert(common_session_arm_if_ready(&s) && requests == 1 && prompts == 1);
    for (lib_i32 i = 0; i < 10; ++i) assert(common_session_arm_if_ready(&s));
    assert(requests == 1 && prompts == 1);
    lib_u32 before = callbacks;
    event.kind = COMMON_SESSION_EVENT_FRAME_COMPLETED; event.run_generation = 1;
    for (lib_u32 i = 1; i <= 10; ++i) {
        event.value.frame.sequence = i;
        assert(common_session_process_completed(&s, &event));
    }
    assert(callbacks == before && requests == 1);
    /* Notification cancels the editing fragment before writing. Provider
     * readiness admits the next prompt without storing a second prompt. */
    memcpy(notice.text, "notice", 7);
    assert(common_session_apply_result(&s, &notice));
    assert(cancellations == 1 && notices == 1 && !s.pending_line);
    assert(common_session_arm_if_ready(&s) && requests == 2 && prompts == 2);
    /* Reader completed, but its line is still in the FIFO. Multiple notices
     * and frame events must not start its successor before consumption. */
    completed = LIB_TRUE;
    event.kind = COMMON_SESSION_EVENT_RUNTIME_COMPLETED;
    event.value.runtime_state = COMMON_SESSION_MACHINE_RUNNING;
    assert(common_session_process_completed(&s, &event));
    assert(commands == 1 && notices == 2 && s.pending_line && requests == 2);
    assert(common_session_apply_result(&s, &notice));
    assert(common_session_arm_if_ready(&s) && requests == 2 && notices == 3);
    /* Both rejected and ordinary line events pass through the actual loop. */
    assert(common_session_queue_push_monitor_line(&s.queue, &line, 1));
    assert(common_session_run(&s));
    assert(waits == 2u);
    assert(requests == 3 && prompts == 3);
    exit_on_request = LIB_FALSE;
    /* Demand survives being non-current; only confirmed handoff clears the
     * outstanding turn, not an intention to create/bind another component. */
    s.pending_line = LIB_TRUE;
    s.state.current_console_actual = COMMON_SESSION_CONSOLE_VM;
    assert(common_session_arm_if_ready(&s) && s.pending_line && requests == 3);
    event.kind = COMMON_SESSION_EVENT_BROKER_COMPLETED;
    event.value.broker_vm_console_current = LIB_FALSE;
    assert(common_session_process_completed(&s, &event));
    assert(s.pending_line && requests == 4);
    fail_cancel = LIB_TRUE;
    before = notices;
    assert(!common_session_apply_result(&s, &notice));
    assert(s.pending_line && notices == before && requests == 4);
    fail_cancel = LIB_FALSE; completed = LIB_FALSE;
    assert(common_session_apply_result(&s, &notice));
    fail_request = LIB_TRUE;
    assert(!common_session_arm_if_ready(&s));
    assert(!s.pending_line);
    /* Borrowed long output shares the exact notification/reader transaction.
     * CRLF and lone LF remain correct across the writer's chunk boundaries. */
    {
        static char text[20001], expected[40000];
        lib_size end = 0u;
        memset(text, 'x', sizeof(text) - 1u);
        for (lib_size i = 1020u; i + 1u < sizeof(text) - 1u; i += 1022u) {
            text[i] = '\r'; text[i + 1u] = '\n';
        }
        text[5] = '\n';
        for (lib_size i = 0; text[i]; ++i) {
            if (text[i] == '\n' && (i == 0u || text[i - 1u] != '\r')) expected[end++] = '\r';
            expected[end++] = text[i];
        }
        expected[end] = '\0';
        common_session_command_result large = { .detail = text };
        collect = LIB_TRUE;
        assert(common_session_apply_result(&s, &large));
        assert(strcmp(output, expected) == 0 && writes > 1u);
        assert(!s.pending_line);
        output_used = writes = 0u; output[0] = '\0';
        s.pending_line = LIB_TRUE;
        before = cancellations;
        assert(common_session_apply_result(&s, &large));
        assert(cancellations == before + 1u && !s.pending_line);
        assert(strncmp(output, "\r\n", 2u) == 0 && strcmp(output + 2, expected) == 0);
        output_used = writes = 0u; output[0] = '\0'; fail_write = 2u;
        assert(!common_session_apply_result(&s, &large) && writes == 2u);
        fail_write = 0u; output_used = writes = 0u;
        s.state.current_console_actual = COMMON_SESSION_CONSOLE_VM;
        assert(common_session_apply_result(&s, &large) && writes == 0u);
    }
    /* Run-loop failure propagates without a second wait or new reader. */
    collect = LIB_FALSE;
    fail_request = LIB_FALSE;
    s.state.current_console_actual = COMMON_SESSION_CONSOLE_MONITOR;
    s.pending_line = LIB_TRUE;
    before = requests;
    waits = 0u; fail_wait = LIB_TRUE;
    assert(!common_session_run(&s));
    assert(waits == 1u && requests == before);
    common_session_queue_dispose(&s.queue);
    return 0;
}
