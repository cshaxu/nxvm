#include "lib/types/test.h"
#include "emulator/session/session_interface.h"
#include "emulator/session/control.h"

static lib_u32 requests, prompts, notices, callbacks, cancellations, commands;
static lib_bool completed, fail_cancel, fail_request, exit_on_request;
static lib_bool collect;
static char output[40000];
static lib_size output_used;
static lib_u32 writes, fail_write;
static lib_u32 waits;
static lib_bool fail_wait;
static lib_bool fail_session_allocation;
static lib_bool fail_session_queue;
static lib_u32 session_allocations, session_releases;
static void *allocate_session(lib_size count, lib_size size)
{
    if (fail_session_allocation) return LIB_NULL;
    void *memory = lib_allocate_zero(count, size);
    if (memory != LIB_NULL) ++session_allocations;
    return memory;
}
static void release_session(void *memory)
{
    if (memory != LIB_NULL) ++session_releases;
    lib_release(memory);
}
static lib_bool initialize_session_queue(emulator_session_queue *queue)
{ return !fail_session_queue && emulator_session_queue_initialize(queue); }
static lib_bool take_event(emulator_session_queue *queue, emulator_session_event *event,
    lib_u32 timeout_ms)
{
    lib_test_assert(timeout_ms == LIB_UINT32_MAX);
    ++waits;
    if (fail_wait) {
        lib_test_assert(waits == 1u); /* A failed wait must never be retried. */
        return LIB_FALSE;
    }
    return emulator_session_queue_take(queue, event, timeout_ms);
}
static lib_u32 run(const emulator_machine *m) { (void)m; return 1; }
static lib_bool copy_frame(emulator_machine *m, emulator_machine_frame *f, lib_u32 g)
{ (void)m; (void)f; (void)g; return LIB_FALSE; }
static lib_u32 lifecycle_calls[EMULATOR_SESSION_REQUEST_RESET + 1u];
static emulator_session_request opened_request;
static lib_bool machine_request(emulator_machine *m, emulator_session_request request)
{ (void)m; ++commands; ++lifecycle_calls[request]; return LIB_TRUE; }
#define emulator_machine_run_generation run
#define emulator_machine_copy_published_frame copy_frame
#define emulator_machine_start(machine) machine_request(machine, EMULATOR_SESSION_REQUEST_START)
#define emulator_machine_resume(machine) machine_request(machine, EMULATOR_SESSION_REQUEST_RESUME)
#define emulator_machine_pause(machine) machine_request(machine, EMULATOR_SESSION_REQUEST_PAUSE)
#define emulator_machine_reset(machine) machine_request(machine, EMULATOR_SESSION_REQUEST_RESET)
#define emulator_machine_stop(machine) machine_request(machine, EMULATOR_SESSION_REQUEST_STOP)
#define emulator_session_queue_take take_event
#define lib_allocate_zero allocate_session
#define lib_release release_session
#define emulator_session_queue_initialize initialize_session_queue
#include "emulator/session/session.c"
#undef emulator_session_queue_take
#undef lib_allocate_zero
#undef lib_release
#undef emulator_session_queue_initialize

static emulator_session *active;
lib_status emulator_ui_cancel_monitor_line(emulator_ui *ui, lib_bool *out_completed)
{ (void)ui; ++cancellations; *out_completed = completed;
  return fail_cancel ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK; }
lib_status emulator_ui_write_monitor(emulator_ui *ui, const char *text)
{
    (void)ui;
    if (collect) {
        if (++writes == fail_write) return LIB_STATUS_IO_ERROR;
        lib_test_assert(output_used + lib_text_length(text) < sizeof(output));
        lib_memory_copy(output + output_used, text, lib_text_length(text) + 1u);
        output_used += lib_text_length(text);
    }
    if (lib_text_compare(text, "> ") == 0) ++prompts;
    else if (lib_text_compare(text, "notice") == 0) ++notices;
    return LIB_STATUS_OK;
}
lib_status emulator_ui_request_monitor_line(emulator_ui *ui)
{
    (void)ui; ++requests;
    if (exit_on_request) {
        lib_console_line line = {0};
        lib_memory_copy(line.text, "exit", 5); line.length = 4;
        lib_test_assert(emulator_session_queue_push_monitor_line(&active->queue, &line, 0));
    }
    return fail_request ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}
void emulator_ui_set_run_generation(emulator_ui *ui, lib_u32 generation)
{ (void)ui; lib_test_assert(generation == 1); }
lib_status emulator_ui_apply_action(emulator_ui *ui, emulator_ui_action a, emulator_ui_state s)
{ (void)ui; (void)a; (void)s; return LIB_STATUS_OK; }
lib_status emulator_ui_set_state(emulator_ui *ui, emulator_ui_state s)
{ (void)ui; (void)s; return LIB_STATUS_OK; }
lib_status emulator_ui_release_window_mouse(emulator_ui *ui)
{ (void)ui; return LIB_STATUS_OK; }
lib_status emulator_ui_publish_frame(emulator_ui *ui, const kvm_window_frame *f,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool w, lib_bool c, lib_bool status)
{ (void)characters; (void)sequence; (void)ui; (void)f; (void)w; (void)c; (void)status; return LIB_STATUS_OK; }
static void monitor(void *p, lib_bool current, emulator_session_command_result *out)
{
    (void)p; (void)current; ++callbacks;
    /* Even an eager provider cannot create two outstanding monitor turns. */
    out->arm_prompt = LIB_TRUE;
    lib_memory_copy(out->prompt, "> ", 3);
}
static void opened(void *p, emulator_session_command_result *out) { (void)p; out->request = opened_request; }
static void rejected(void *p, emulator_session_command_result *out)
{
    (void)p;
    lib_test_assert(!active->pending_line);
    lib_memory_copy(out->text, "rejected", 9);
    exit_on_request = LIB_TRUE;
}
static void submitted(void *p, emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out)
{
    (void)p; (void)state;
    lib_test_assert(!active->pending_line && lib_text_compare(line, "exit") == 0);
    out->exit_requested = LIB_TRUE;
}
static void runtime(void *p, emulator_session_machine_state a,
    emulator_session_machine_state b, emulator_session_command_result *out)
{
    (void)p; (void)a; (void)b;
    lib_memory_copy(out->text, "notice", 7);
    out->request = EMULATOR_SESSION_REQUEST_PAUSE;
}

static void public_session_contract(void)
{
    emulator_session_options options = {
        .display = EMULATOR_SESSION_DISPLAY_WINDOW,
        .console_control = LIB_TRUE,
        .machine = (emulator_machine *)&options,
        .command = { .open = opened, .submit_line = submitted,
            .note_runtime = runtime, .note_monitor_current = monitor }
    };
    emulator_session *session = LIB_NULL;
    lib_test_assert(emulator_session_create(LIB_NULL, &options) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_session_create(&session, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && session == LIB_NULL);
    for (lib_u32 missing = 0u; missing < 5u; ++missing) {
        emulator_session_options invalid = options;
        switch (missing) {
        case 0u: invalid.machine = LIB_NULL; break;
        case 1u: invalid.command.open = LIB_NULL; break;
        case 2u: invalid.command.submit_line = LIB_NULL; break;
        case 3u: invalid.command.note_runtime = LIB_NULL; break;
        default: invalid.command.note_monitor_current = LIB_NULL; break;
        }
        lib_test_assert(emulator_session_create(&session, &invalid) == LIB_STATUS_INVALID_ARGUMENT && session == LIB_NULL);
    }
    fail_session_allocation = LIB_TRUE;
    lib_test_assert(emulator_session_create(&session, &options) == LIB_STATUS_NO_MEMORY && session == LIB_NULL);
    fail_session_allocation = LIB_FALSE;
    fail_session_queue = LIB_TRUE;
    lib_test_assert(emulator_session_create(&session, &options) == LIB_STATUS_NO_MEMORY && session == LIB_NULL);
    lib_test_assert(session_allocations == session_releases);
    fail_session_queue = LIB_FALSE;
    lib_test_assert(emulator_session_create(&session, &options) == LIB_STATUS_OK);
    lib_test_assert(session_allocations == session_releases + 1u && session->machine == options.machine &&
        session->state.display == options.display && session->ui == LIB_NULL);
    lib_test_assert(!emulator_session_run(session) && !emulator_session_run(LIB_NULL));
    lib_test_assert(emulator_session_bind_ui(LIB_NULL, (emulator_ui *)&options) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_session_bind_ui(session, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_session_bind_ui(session, (emulator_ui *)&options) == LIB_STATUS_OK);
    lib_test_assert(emulator_session_bind_ui(session, (emulator_ui *)session) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(session->ui == (emulator_ui *)&options);

    emulator_ui_event input = { .run_generation = 1u };
    emulator_session_event taken;
    lib_test_assert(!emulator_session_enqueue_ui_event(LIB_NULL, &input));
    lib_test_assert(!emulator_session_enqueue_ui_event(session, LIB_NULL));
    input.kind = (emulator_ui_event_kind)99;
    lib_test_assert(!emulator_session_enqueue_ui_event(session, &input) && session->queue.count == 0u);
    input.kind = EMULATOR_UI_EVENT_KVM_INPUT;
    input.value.kvm.type = KVM_EVENT_TEXT;
    input.value.kvm.data.text.scalar = 'X';
    lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
    input.value.kvm.data.text.scalar = 'Y';
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_KVM_INPUT &&
        taken.run_generation == 1u && taken.value.kvm.data.text.scalar == 'X');
    input = (emulator_ui_event){ .kind = EMULATOR_UI_EVENT_MONITOR_LINE,
        .monitor_line_rejected = LIB_TRUE };
    lib_memory_copy(input.value.line.text, "abc", 4u);
    input.value.line.length = 3u;
    lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
    input.value.line.text[0] = 'z';
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_MONITOR_LINE && taken.monitor_line_rejected &&
        taken.value.line.length == 3u && lib_text_compare(taken.value.line.text, "abc") == 0);
    for (lib_u32 component = 0u; component < 2u; ++component) {
        input = (emulator_ui_event){ .kind = EMULATOR_UI_EVENT_COMPONENT_COMPLETED,
            .run_generation = 7u };
        input.value.component.component = component == 0u ?
            EMULATOR_UI_COMPONENT_WINDOW : EMULATOR_UI_COMPONENT_VM_CONSOLE;
        input.value.component.exists = LIB_TRUE;
        lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
        lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
        lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_COMPONENT_COMPLETED &&
            taken.run_generation == 7u && taken.value.component.exists &&
            taken.value.component.component == (component == 0u ?
                EMULATOR_SESSION_EVENT_COMPONENT_WINDOW : EMULATOR_SESSION_EVENT_COMPONENT_VM_CONSOLE));
    }
    input = (emulator_ui_event){ .kind = EMULATOR_UI_EVENT_BROKER_COMPLETED, .run_generation = 8u };
    input.value.broker_vm_console_current = LIB_TRUE;
    lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_BROKER_COMPLETED &&
        taken.run_generation == 8u && taken.value.broker_vm_console_current);
    input = (emulator_ui_event){ .kind = EMULATOR_UI_EVENT_KVM_DELIVERY_FAILED, .run_generation = 9u };
    input.value.delivery_failure.source_identity = 12u;
    input.value.delivery_failure.status = LIB_STATUS_IO_ERROR;
    lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_KVM_DELIVERY_FAILED &&
        taken.run_generation == 9u && taken.value.delivery_failure.source_identity == 12u &&
        taken.value.delivery_failure.status == LIB_STATUS_IO_ERROR);
    input = (emulator_ui_event){ .kind = EMULATOR_UI_EVENT_CONSOLE_FAILED };
    lib_test_assert(emulator_session_enqueue_ui_event(session, &input));
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_CONSOLE_FAILED);
    lib_test_assert(!emulator_session_enqueue_runtime_completed(LIB_NULL, EMULATOR_SESSION_MACHINE_PAUSED, 1u));
    lib_test_assert(!emulator_session_enqueue_frame_completed(LIB_NULL, 1u, LIB_TRUE, 1u));
    lib_test_assert(emulator_session_enqueue_runtime_completed(session, EMULATOR_SESSION_MACHINE_PAUSED, 10u));
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED &&
        taken.run_generation == 10u && taken.value.runtime_state == EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(emulator_session_enqueue_frame_completed(session, 5u, LIB_TRUE, 11u));
    lib_test_assert(emulator_session_queue_take(&session->queue, &taken, 0u));
    lib_test_assert(taken.kind == EMULATOR_SESSION_EVENT_FRAME_COMPLETED &&
        taken.run_generation == 11u && taken.value.frame.sequence == 5u && taken.value.frame.graphics);

    active = session;
    exit_on_request = LIB_TRUE;
    lib_test_assert(emulator_session_run(session) && commands == 1u && waits == 1u);
    /* Product requests are injected through the real public run loop; each
     * must reach its corresponding Machine API, not merely some lifecycle API. */
    for (lib_u32 request = EMULATOR_SESSION_REQUEST_START;
            request <= EMULATOR_SESSION_REQUEST_RESET; ++request) {
        lib_u32 before_calls[EMULATOR_SESSION_REQUEST_RESET + 1u];
        lib_memory_copy(before_calls, lifecycle_calls, sizeof(before_calls));
        opened_request = (emulator_session_request)request;
        lib_test_assert(emulator_session_run(session));
        for (lib_u32 kind = EMULATOR_SESSION_REQUEST_START;
                kind <= EMULATOR_SESSION_REQUEST_RESET; ++kind)
            lib_test_assert(lifecycle_calls[kind] == before_calls[kind] +
                (kind == request ? 1u : 0u) + (kind == EMULATOR_SESSION_REQUEST_STOP ? 1u : 0u));
    }
    opened_request = (emulator_session_request)99;
    lib_u32 before_invalid = commands;
    lib_test_assert(!emulator_session_run(session) && commands == before_invalid);
    opened_request = EMULATOR_SESSION_REQUEST_NONE;
    lib_test_assert(emulator_session_destroy(session) == LIB_STATUS_OK);
    lib_test_assert(emulator_session_destroy(LIB_NULL) == LIB_STATUS_OK && session_releases == session_allocations);
    requests = prompts = notices = callbacks = cancellations = commands = waits = 0u;
    exit_on_request = LIB_FALSE;
    active = LIB_NULL;
}

int main(void)
{
    public_session_contract();
    static emulator_session s;
    emulator_session_event event = {0};
    emulator_session_command_result notice = {0};
    lib_console_line line = {0};
    active = &s;
    s.machine = (emulator_machine *)&s; s.ui = (emulator_ui *)&s;
    s.command.note_monitor_current = monitor;
    s.command.note_runtime = runtime;
    s.command.open = opened; s.command.reject_line = rejected;
    s.command.submit_line = submitted;
    lib_test_assert(emulator_session_queue_initialize(&s.queue));
    emulator_session_state_initialize(&s.state, EMULATOR_SESSION_DISPLAY_WINDOW, 1);
    emulator_session_state_note_runtime(&s.state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_window(&s.state, 1);
    lib_test_assert(emulator_session_arm_if_ready(&s) && requests == 1 && prompts == 1);
    for (lib_i32 i = 0; i < 10; ++i) lib_test_assert(emulator_session_arm_if_ready(&s));
    lib_test_assert(requests == 1 && prompts == 1);
    lib_u32 before = callbacks;
    event.kind = EMULATOR_SESSION_EVENT_FRAME_COMPLETED; event.run_generation = 1;
    for (lib_u32 i = 1; i <= 10; ++i) {
        event.value.frame.sequence = i;
        lib_test_assert(emulator_session_process_completed(&s, &event));
    }
    lib_test_assert(callbacks == before && requests == 1);
    /* Notification cancels the editing fragment before writing. Provider
     * readiness admits the next prompt without storing a second prompt. */
    lib_memory_copy(notice.text, "notice", 7);
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    lib_test_assert(cancellations == 1 && notices == 1 && !s.pending_line);
    lib_test_assert(emulator_session_arm_if_ready(&s) && requests == 2 && prompts == 2);
    /* Reader completed, but its line is still in the FIFO. Multiple notices
     * and frame events must not start its successor before consumption. */
    completed = LIB_TRUE;
    event.kind = EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED;
    event.value.runtime_state = EMULATOR_SESSION_MACHINE_RUNNING;
    lib_test_assert(emulator_session_process_completed(&s, &event));
    lib_test_assert(commands == 1 && notices == 2 && s.pending_line && requests == 2);
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    lib_test_assert(emulator_session_arm_if_ready(&s) && requests == 2 && notices == 3);
    /* Both rejected and ordinary line events pass through the actual loop. */
    lib_test_assert(emulator_session_queue_push_monitor_line(&s.queue, &line, 1));
    lib_test_assert(emulator_session_run(&s));
    lib_test_assert(waits == 2u);
    lib_test_assert(requests == 3 && prompts == 3);
    exit_on_request = LIB_FALSE;
    /* Demand survives being non-current; only confirmed handoff clears the
     * outstanding turn, not an intention to create/bind another component. */
    s.pending_line = LIB_TRUE;
    s.state.current_console_actual = EMULATOR_SESSION_CONSOLE_VM;
    lib_test_assert(emulator_session_arm_if_ready(&s) && s.pending_line && requests == 3);
    event.kind = EMULATOR_SESSION_EVENT_BROKER_COMPLETED;
    event.value.broker_vm_console_current = LIB_FALSE;
    lib_test_assert(emulator_session_process_completed(&s, &event));
    lib_test_assert(s.pending_line && requests == 4);
    fail_cancel = LIB_TRUE;
    before = notices;
    lib_test_assert(!emulator_session_apply_result(&s, &notice));
    lib_test_assert(s.pending_line && notices == before && requests == 4);
    fail_cancel = LIB_FALSE; completed = LIB_FALSE;
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    fail_request = LIB_TRUE;
    lib_test_assert(!emulator_session_arm_if_ready(&s));
    lib_test_assert(!s.pending_line);
    /* Borrowed long output shares the exact notification/reader transaction.
     * CRLF and lone LF remain correct across the writer's chunk boundaries. */
    {
        static char text[20001], expected[40000];
        lib_size end = 0u;
        lib_memory_set(text, 'x', sizeof(text) - 1u);
        for (lib_size i = 1020u; i + 1u < sizeof(text) - 1u; i += 1022u) {
            text[i] = '\r'; text[i + 1u] = '\n';
        }
        text[5] = '\n';
        for (lib_size i = 0; text[i]; ++i) {
            if (text[i] == '\n' && (i == 0u || text[i - 1u] != '\r')) expected[end++] = '\r';
            expected[end++] = text[i];
        }
        expected[end] = '\0';
        emulator_session_command_result large = { .detail = text };
        collect = LIB_TRUE;
        lib_test_assert(emulator_session_apply_result(&s, &large));
        lib_test_assert(lib_text_compare(output, expected) == 0 && writes > 1u);
        lib_test_assert(!s.pending_line);
        output_used = writes = 0u; output[0] = '\0';
        s.pending_line = LIB_TRUE;
        before = cancellations;
        lib_test_assert(emulator_session_apply_result(&s, &large));
        lib_test_assert(cancellations == before + 1u && !s.pending_line);
        lib_test_assert(lib_text_compare_n(output, "\r\n", 2u) == 0 && lib_text_compare(output + 2, expected) == 0);
        output_used = writes = 0u; output[0] = '\0'; fail_write = 2u;
        lib_test_assert(!emulator_session_apply_result(&s, &large) && writes == 2u);
        fail_write = 0u; output_used = writes = 0u;
        s.state.current_console_actual = EMULATOR_SESSION_CONSOLE_VM;
        lib_test_assert(emulator_session_apply_result(&s, &large) && writes == 0u);
    }
    /* Run-loop failure propagates without a second wait or new reader. */
    collect = LIB_FALSE;
    fail_request = LIB_FALSE;
    s.state.current_console_actual = EMULATOR_SESSION_CONSOLE_MONITOR;
    s.pending_line = LIB_TRUE;
    before = requests;
    waits = 0u; fail_wait = LIB_TRUE;
    lib_test_assert(!emulator_session_run(&s));
    lib_test_assert(waits == 1u && requests == before);
    emulator_session_queue_dispose(&s.queue);
    return 0;
}
