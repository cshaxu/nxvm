#include "lib/types/test.h"
#include "emulator/session/session_interface.h"
#include "emulator/session/control.h"

static lib_u32 requests, prompts, notices, cancellations, commands, runtime_notices;
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
static void opened(void *p, emulator_session_command_result *out)
{
    (void)p;
    out->request = opened_request;
    out->arm_prompt = opened_request == EMULATOR_SESSION_REQUEST_NONE;
    lib_memory_copy(out->prompt, "> ", 3u);
}
static void rejected(void *p, emulator_session_command_result *out)
{
    (void)p;
    lib_test_assert(!active->pending_line);
    lib_memory_copy(out->text, "rejected", 9);
    out->arm_prompt = LIB_TRUE;
    lib_memory_copy(out->prompt, "> ", 3u);
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
    ++runtime_notices;
    lib_memory_copy(out->text, "notice", 7);
    out->arm_prompt = LIB_TRUE;
    lib_memory_copy(out->prompt, "> ", 3u);
}

static void public_session_contract(void)
{
    emulator_session_options options = {
        .display = EMULATOR_SESSION_DISPLAY_WINDOW,
        .console_control = LIB_TRUE,
        .machine = (emulator_machine *)&options,
        .command = { .open = opened, .submit_line = submitted, .note_runtime = runtime }
    };
    emulator_session *session = LIB_NULL;
    lib_test_assert(emulator_session_create(LIB_NULL, &options) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_session_create(&session, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && session == LIB_NULL);
    for (lib_u32 missing = 0u; missing < 4u; ++missing) {
        emulator_session_options invalid = options;
        switch (missing) {
        case 0u: invalid.machine = LIB_NULL; break;
        case 1u: invalid.command.open = LIB_NULL; break;
        case 2u: invalid.command.submit_line = LIB_NULL; break;
        default: invalid.command.note_runtime = LIB_NULL; break;
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
    opened_request = EMULATOR_SESSION_REQUEST_NONE;
    lib_test_assert(emulator_session_run(session) && commands == 1u && waits == 1u);
    /* Session alone dispatches every accepted product request. A request stays
     * in flight until a terminal runtime completion; a second request is not
     * sent merely because its provider parses another line. */
    for (lib_u32 request = EMULATOR_SESSION_REQUEST_START;
            request <= EMULATOR_SESSION_REQUEST_RESET; ++request) {
        lib_u32 before_calls[EMULATOR_SESSION_REQUEST_RESET + 1u];
        lib_memory_copy(before_calls, lifecycle_calls, sizeof(before_calls));
        emulator_session_command_result request_result = {
            .request = (emulator_session_request)request};
        lib_test_assert(emulator_session_apply_result(session, &request_result));
        lib_test_assert(session->pending_request == (emulator_session_request)request);
        lib_test_assert(emulator_session_apply_result(session, &request_result));
        session->pending_request = EMULATOR_SESSION_REQUEST_NONE;
        for (lib_u32 kind = EMULATOR_SESSION_REQUEST_START;
                kind <= EMULATOR_SESSION_REQUEST_RESET; ++kind)
            lib_test_assert(lifecycle_calls[kind] == before_calls[kind] +
                (kind == request ? 1u : 0u));
    }
    lib_test_assert(emulator_session_destroy(session) == LIB_STATUS_OK);
    lib_test_assert(emulator_session_destroy(LIB_NULL) == LIB_STATUS_OK && session_releases == session_allocations);
    requests = prompts = notices = cancellations = commands = waits = 0u;
    exit_on_request = LIB_FALSE;
    active = LIB_NULL;
}

int main(void)
{
    public_session_contract();
    {
        emulator_session initial = {0};
        emulator_session_event initial_event = {
            .kind = EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED,
            .value.runtime_state = EMULATOR_SESSION_MACHINE_STOPPED};

        active = &initial;
        initial.machine = (emulator_machine *)&initial;
        initial.ui = (emulator_ui *)&initial;
        initial.command.note_runtime = runtime;
        initial.command.open = opened;
        initial.command.reject_line = rejected;
        initial.command.submit_line = submitted;
        lib_test_assert(emulator_session_queue_initialize(&initial.queue));
        emulator_session_state_initialize(&initial.state, EMULATOR_SESSION_DISPLAY_CONSOLE, 1);
        runtime_notices = 0u;
        lib_test_assert(emulator_session_process_completed(&initial, &initial_event));
        lib_test_assert(initial.state.monitor_actual == EMULATOR_SESSION_MACHINE_STOPPED &&
            runtime_notices == 0u);
        emulator_session_state_note_runtime(&initial.state, EMULATOR_SESSION_MACHINE_RUNNING);
        lib_test_assert(emulator_session_process_completed(&initial, &initial_event) &&
            runtime_notices == 1u);
        emulator_session_queue_dispose(&initial.queue);
    }
    requests = prompts = notices = cancellations = commands = writes = waits = 0u;
    output_used = 0u;
    static emulator_session s;
    emulator_session_event event = {0};
    emulator_session_command_result notice = {0};
    active = &s;
    runtime_notices = 0u;
    s.machine = (emulator_machine *)&s; s.ui = (emulator_ui *)&s;
    s.command.note_runtime = runtime;
    s.command.open = opened; s.command.reject_line = rejected;
    s.command.submit_line = submitted;
    lib_test_assert(emulator_session_queue_initialize(&s.queue));
    emulator_session_state_initialize(&s.state, EMULATOR_SESSION_DISPLAY_WINDOW, 1);
    emulator_session_state_note_runtime(&s.state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_window(&s.state, 1);
    s.prompt_due = LIB_TRUE;
    lib_memory_copy(s.pending_prompt, "> ", 3u);
    lib_test_assert(emulator_session_arm_if_ready(&s) && requests == 1 && prompts == 1);
    for (lib_i32 i = 0; i < 10; ++i) lib_test_assert(emulator_session_arm_if_ready(&s));
    lib_test_assert(requests == 1 && prompts == 1);

    /* A visible result cancels the one reader, then Session (not Product)
     * owns re-arming exactly one successor. */
    lib_memory_copy(notice.text, "notice", 7);
    notice.arm_prompt = LIB_TRUE;
    lib_memory_copy(notice.prompt, "> ", 3u);
    collect = LIB_TRUE;
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    lib_test_assert(cancellations == 1 && notices == 1 && !s.pending_line);
    lib_test_assert(emulator_session_arm_if_ready(&s) && requests == 2 && prompts == 2);
    lib_test_assert(lib_text_find_substring(output, "notice\r\n\r\n> ") != LIB_NULL);
    collect = LIB_FALSE;

    /* Raw Console receives no monitor text. Session keeps the copied text and
     * releases it only after the broker confirms Monitor ownership again. */
    s.state.current_console_actual = EMULATOR_SESSION_CONSOLE_VM;
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    lib_test_assert(notices == 1 && s.pending_monitor_text[0] != '\0');
    event.kind = EMULATOR_SESSION_EVENT_BROKER_COMPLETED;
    event.value.broker_vm_console_current = LIB_FALSE;
    lib_test_assert(emulator_session_process_completed(&s, &event));
    lib_test_assert(notices == 1 && s.pending_monitor_text[0] == '\0');

    /* A lifecycle request is dispatched once, remains owned by Session over
     * INIT, and clears only on its terminal runtime completion. */
    lib_u32 pause_calls = lifecycle_calls[EMULATOR_SESSION_REQUEST_PAUSE];
    notice = (emulator_session_command_result){.request = EMULATOR_SESSION_REQUEST_PAUSE};
    lib_test_assert(emulator_session_apply_result(&s, &notice));
    lib_test_assert(s.pending_request == EMULATOR_SESSION_REQUEST_PAUSE &&
        lifecycle_calls[EMULATOR_SESSION_REQUEST_PAUSE] == pause_calls + 1u);
    event.kind = EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED;
    event.value.runtime_state = EMULATOR_SESSION_MACHINE_INIT;
    lib_test_assert(emulator_session_process_completed(&s, &event));
    lib_test_assert(s.pending_request == EMULATOR_SESSION_REQUEST_PAUSE);
    event.value.runtime_state = EMULATOR_SESSION_MACHINE_PAUSED;
    lib_test_assert(emulator_session_process_completed(&s, &event));
    lib_test_assert(s.pending_request == EMULATOR_SESSION_REQUEST_NONE);
    emulator_session_queue_dispose(&s.queue);
    return 0;
}
