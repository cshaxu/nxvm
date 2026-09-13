#include "common/session/session_interface.h"

#include "lib/host/sync_interface.h"

#define COMMON_SESSION_FACT_CAPACITY 64u
#define COMMON_SESSION_PRESSED_CAPACITY 256u

typedef struct common_session_pressed_key {
    lib_u64 source_identity;
    ui_input_event event;
} common_session_pressed_key;

struct common_session {
    host_sync_event *ready;
    lib_atomic_flag lock;
    common_session_fact facts[COMMON_SESSION_FACT_CAPACITY];
    lib_size first;
    lib_size count;
    lib_bool accepting;
    lib_bool delivery_failed;
    ui_frame latest_frame;
    lib_bool frame_ready;
    lib_u32 latest_frame_run_id;
    lib_u32 run_id;
    common_ui *ui;
    common_session_machine_state lifecycle;
    common_session_target presentation_display;
    lib_bool console_control;
    lib_bool presentation_configured;
    lib_bool presentation_hidden;
    lib_bool presentation_frame_available;
    lib_bool presentation_frame_graphics;
    common_ui_surface_facts surface_facts;
    lib_bool ui_action_in_flight;
    common_ui_action_kind ui_action_in_flight_kind;
    common_session_pressed_key pressed[COMMON_SESSION_PRESSED_CAPACITY];
    lib_size pressed_count;
    common_session_cli_provider cli_provider;
    void *cli_provider_context;
    common_session_cli_machine_observer cli_machine_observer;
    void *cli_machine_observer_context;
    common_session_lifecycle_sink lifecycle_sink;
    void *lifecycle_sink_context;
};

static void common_session_lock(common_session *session)
{
    while (lib_atomic_flag_test_and_set_explicit(&session->lock,
        LIB_MEMORY_ORDER_ACQUIRE)) { }
}

static void common_session_unlock(common_session *session)
{
    lib_atomic_flag_clear_explicit(&session->lock, LIB_MEMORY_ORDER_RELEASE);
}

static void common_session_plan_clear(common_session_plan *plan)
{
    if (plan != LIB_NULL) lib_memory_set(plan, 0, sizeof(*plan));
}

static void common_session_plan_action(common_session *session,
    common_session_plan *plan, common_ui_action_kind kind)
{
    if (session == LIB_NULL || plan == LIB_NULL || session->ui_action_in_flight)
        return;
    plan->ui_action_ready = LIB_TRUE;
    plan->ui_action.kind = kind;
    session->ui_action_in_flight = LIB_TRUE;
    session->ui_action_in_flight_kind = kind;
}

static void common_session_plan_surface(common_session *session,
    common_session_plan *plan)
{
    common_ui_surface_facts facts;
    lib_bool want_window;
    lib_bool want_raw_console;

    if (session == LIB_NULL || plan == LIB_NULL || session->ui_action_in_flight)
        return;
    facts = session->surface_facts;
    want_window = LIB_FALSE;
    want_raw_console = LIB_FALSE;
    if (session->presentation_configured && !session->presentation_hidden &&
        session->lifecycle != COMMON_SESSION_MACHINE_STOPPED &&
        session->lifecycle != COMMON_SESSION_MACHINE_FAULT) {
        if (session->lifecycle == COMMON_SESSION_MACHINE_PAUSED) {
            want_window = session->presentation_display == COMMON_SESSION_TARGET_WINDOW ||
                session->presentation_frame_graphics;
        } else if (session->presentation_display == COMMON_SESSION_TARGET_WINDOW) {
            want_window = LIB_TRUE;
        } else if (session->presentation_frame_available) {
            want_window = session->presentation_frame_graphics;
            want_raw_console = !session->presentation_frame_graphics ||
                !session->console_control;
        }
    }
    if (want_raw_console) {
        if (!facts.raw_console_exists)
            common_session_plan_action(session, plan,
                COMMON_UI_ACTION_CREATE_RAW_CONSOLE);
        else if (!facts.raw_console_current)
            common_session_plan_action(session, plan,
                COMMON_UI_ACTION_BIND_RAW_CONSOLE);
        if (plan->ui_action_ready) return;
    } else {
        if (facts.raw_console_current)
            common_session_plan_action(session, plan,
                COMMON_UI_ACTION_BIND_MONITOR_CONSOLE);
        else if (facts.raw_console_exists)
            common_session_plan_action(session, plan,
                COMMON_UI_ACTION_DESTROY_RAW_CONSOLE);
        if (plan->ui_action_ready) return;
    }
    if (want_window && !facts.window_exists) {
        common_session_plan_action(session, plan, COMMON_UI_ACTION_CREATE_WINDOW);
    } else if (!want_window && facts.window_exists) {
        common_session_plan_action(session, plan, COMMON_UI_ACTION_DESTROY_WINDOW);
    }
}

lib_status common_session_create(common_session **out_session)
{
    common_session *session;

    if (out_session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = LIB_NULL;
    session = lib_allocate_zero(1u, sizeof(*session));
    if (session == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    session->lock = (lib_atomic_flag)LIB_ATOMIC_FLAG_INITIALIZER;
    lib_atomic_flag_clear_explicit(&session->lock, LIB_MEMORY_ORDER_RELEASE);
    if (host_sync_event_create(&session->ready) != LIB_STATUS_OK) {
        lib_release(session);
        return LIB_STATUS_NO_MEMORY;
    }
    session->accepting = LIB_TRUE;
    session->lifecycle = COMMON_SESSION_MACHINE_STOPPED;
    *out_session = session;
    return LIB_STATUS_OK;
}

void common_session_destroy(common_session *session)
{
    if (session == LIB_NULL) return;
    host_sync_event_destroy(session->ready);
    lib_release(session);
}

void common_session_close(common_session *session)
{
    if (session == LIB_NULL) return;
    common_session_lock(session);
    session->accepting = LIB_FALSE;
    host_sync_event_signal(session->ready);
    common_session_unlock(session);
}

lib_status common_session_bind_ui(common_session *session, common_ui *ui)
{
    if (session == LIB_NULL || ui == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting || session->ui != LIB_NULL) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    session->ui = ui;
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_status common_session_request_monitor_line(common_session *session)
{
    return session == LIB_NULL || session->ui == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        common_ui_request_console_line(session->ui);
}

lib_status common_session_write_monitor(common_session *session, const char *text)
{
    return session == LIB_NULL || session->ui == LIB_NULL ? LIB_STATUS_INVALID_STATE :
        common_ui_write_console(session->ui, text);
}

lib_status common_session_set_presentation_policy(common_session *session,
    common_session_target display, lib_bool console_control)
{
    if (session == LIB_NULL || display == COMMON_SESSION_TARGET_NONE ||
        display > COMMON_SESSION_TARGET_WINDOW ||
        (console_control != LIB_FALSE && console_control != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    if (session->lifecycle != COMMON_SESSION_MACHINE_STOPPED)
        return LIB_STATUS_INVALID_STATE;
    session->presentation_display = display;
    session->console_control = console_control;
    session->presentation_configured = LIB_TRUE;
    session->presentation_hidden = LIB_FALSE;
    session->presentation_frame_available = LIB_FALSE;
    session->presentation_frame_graphics = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status common_session_hide_presentation(common_session *session)
{
    if (session == LIB_NULL || !session->presentation_configured)
        return LIB_STATUS_INVALID_STATE;
    session->presentation_hidden = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status common_session_reconcile(common_session *session,
    common_session_plan *out_plan)
{
    if (session == LIB_NULL || out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_plan_clear(out_plan);
    common_session_plan_surface(session, out_plan);
    return LIB_STATUS_OK;
}

lib_status common_session_set_cli_provider(common_session *session,
    common_session_cli_provider provider, void *context)
{
    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    session->cli_provider = provider;
    session->cli_provider_context = context;
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_status common_session_set_cli_machine_observer(common_session *session,
    common_session_cli_machine_observer observer, void *context)
{
    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    session->cli_machine_observer = observer;
    session->cli_machine_observer_context = context;
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_bool common_session_has_cli_provider(const common_session *session)
{
    return session != LIB_NULL && session->cli_provider != LIB_NULL;
}

lib_status common_session_set_lifecycle_sink(common_session *session,
    common_session_lifecycle_sink sink, void *context)
{
    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    session->lifecycle_sink = sink;
    session->lifecycle_sink_context = context;
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_status common_session_request_lifecycle(common_session *session,
    common_session_lifecycle_request_kind request)
{
    common_session_lifecycle_sink sink;
    void *context;

    if (session == LIB_NULL || request == COMMON_SESSION_LIFECYCLE_NONE ||
        request > COMMON_SESSION_LIFECYCLE_STOP) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    sink = session->lifecycle_sink;
    context = session->lifecycle_sink_context;
    common_session_unlock(session);
    return sink == LIB_NULL ? LIB_STATUS_INVALID_STATE : sink(context, request);
}

lib_u32 common_session_begin_run(common_session *session,
    common_session_plan *out_plan)
{
    if (session == LIB_NULL || out_plan == LIB_NULL || !session->presentation_configured)
        return 0u;
    common_session_plan_clear(out_plan);
    common_session_lock(session);
    if (session->run_id == UINT32_MAX) {
        common_session_unlock(session);
        return 0u;
    }
    ++session->run_id;
    session->presentation_hidden = LIB_FALSE;
    session->presentation_frame_available = LIB_FALSE;
    session->presentation_frame_graphics = LIB_FALSE;
    common_session_plan_surface(session, out_plan);
    common_session_unlock(session);
    return session->run_id;
}

lib_bool common_session_is_running(const common_session *session)
{
    return session != LIB_NULL && session->lifecycle == COMMON_SESSION_MACHINE_RUNNING;
}

static lib_status common_session_publish(common_session *session,
    const common_session_fact *fact)
{
    lib_size index;

    if (session == LIB_NULL || fact == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    if (session->count == COMMON_SESSION_FACT_CAPACITY) {
        session->delivery_failed = LIB_TRUE;
        host_sync_event_signal(session->ready);
        common_session_unlock(session);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    index = (session->first + session->count) % COMMON_SESSION_FACT_CAPACITY;
    session->facts[index] = *fact;
    ++session->count;
    host_sync_event_signal(session->ready);
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_status common_session_publish_console_line(void *context, const char *line)
{
    common_session_fact fact = {0};
    lib_size bytes;

    if (context == LIB_NULL || line == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    bytes = lib_text_length(line);
    if (bytes >= sizeof(fact.value.line)) return LIB_STATUS_LIMIT_EXCEEDED;
    fact.kind = COMMON_SESSION_FACT_CONSOLE_LINE;
    lib_memory_copy(fact.value.line, line, bytes + 1u);
    return common_session_publish((common_session *)context, &fact);
}

lib_status common_session_publish_monitor_text(common_session *session,
    const char *text)
{
    common_session_fact fact = {0};
    lib_size bytes;

    if (session == LIB_NULL || text == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    bytes = lib_text_length(text);
    if (bytes >= sizeof(fact.value.line)) return LIB_STATUS_LIMIT_EXCEEDED;
    fact.kind = COMMON_SESSION_FACT_MONITOR_TEXT;
    lib_memory_copy(fact.value.line, text, bytes + 1u);
    return common_session_publish(session, &fact);
}

lib_status common_session_publish_ui_input(void *context,
    const ui_input_event *event)
{
    common_session_fact fact = {0};
    common_session *session = context;

    if (context == LIB_NULL || event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fact.kind = COMMON_SESSION_FACT_UI_INPUT;
    fact.value.input = *event;
    common_session_lock(session);
    fact.run_id = session->run_id;
    common_session_unlock(session);
    return common_session_publish(session, &fact);
}

lib_status common_session_publish_ui_delivery_failed(common_session *session,
    lib_u64 source_identity, lib_status status)
{
    common_session_fact fact = {0};

    if (session == LIB_NULL || status == LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    fact.kind = COMMON_SESSION_FACT_UI_DELIVERY_FAILED;
    fact.run_id = session->run_id;
    fact.value.ui_delivery_failure.source_identity = source_identity;
    fact.value.ui_delivery_failure.status = status;
    common_session_unlock(session);
    return common_session_publish(session, &fact);
}

lib_status common_session_publish_machine(common_session *session,
    common_session_machine_state state, lib_status status)
{
    common_session_fact fact = {0};

    if (session == LIB_NULL || state > COMMON_SESSION_MACHINE_FAULT)
        return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    fact.kind = COMMON_SESSION_FACT_MACHINE;
    fact.run_id = session->run_id;
    fact.value.machine.state = state;
    fact.value.machine.status = status;
    common_session_unlock(session);
    return common_session_publish(session, &fact);
}

lib_status common_session_publish_ui_completion(common_session *session,
    lib_status status, const common_ui_completion *completion)
{
    common_session_fact fact = {0};

    if (session == LIB_NULL || completion == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    fact.kind = COMMON_SESSION_FACT_UI_COMPLETION;
    fact.run_id = session->run_id;
    fact.value.ui_completion.status = status;
    fact.value.ui_completion.completion = *completion;
    common_session_unlock(session);
    return common_session_publish(session, &fact);
}

lib_status common_session_publish_frame(common_session *session,
    const ui_frame *frame)
{
    common_session_fact fact = {0};

    if (session == LIB_NULL || frame == LIB_NULL || !ui_frame_is_valid(frame))
        return LIB_STATUS_INVALID_ARGUMENT;
    common_session_lock(session);
    if (!session->accepting) {
        common_session_unlock(session);
        return LIB_STATUS_INVALID_STATE;
    }
    session->latest_frame = *frame;
    session->latest_frame_run_id = session->run_id;
    if (!session->frame_ready) {
        fact.kind = COMMON_SESSION_FACT_FRAME;
        fact.run_id = session->run_id;
        if (session->count == COMMON_SESSION_FACT_CAPACITY) {
            session->delivery_failed = LIB_TRUE;
            common_session_unlock(session);
            return LIB_STATUS_LIMIT_EXCEEDED;
        }
        session->facts[(session->first + session->count) % COMMON_SESSION_FACT_CAPACITY] = fact;
        ++session->count;
    }
    session->frame_ready = LIB_TRUE;
    host_sync_event_signal(session->ready);
    common_session_unlock(session);
    return LIB_STATUS_OK;
}

lib_status common_session_take(common_session *session,
    common_session_fact *out_fact, ui_frame *out_frame,
    lib_u32 timeout_milliseconds)
{
    host_sync_wait_result wait;
    lib_size index;

    if (session == LIB_NULL || out_fact == LIB_NULL || out_frame == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (;;) {
        common_session_lock(session);
        if (session->delivery_failed) {
            common_session_unlock(session);
            return LIB_STATUS_LIMIT_EXCEEDED;
        }
        if (session->count != 0u) {
            index = session->first;
            *out_fact = session->facts[index];
            session->first = (session->first + 1u) % COMMON_SESSION_FACT_CAPACITY;
            --session->count;
            if (out_fact->kind == COMMON_SESSION_FACT_FRAME) {
                if (!session->frame_ready || out_fact->run_id != session->run_id ||
                    session->latest_frame_run_id != session->run_id) {
                    common_session_unlock(session);
                    continue;
                }
                *out_frame = session->latest_frame;
                session->frame_ready = LIB_FALSE;
            } else {
                lib_memory_set(out_frame, 0, sizeof(*out_frame));
            }
            if (session->count == 0u && !session->frame_ready)
                host_sync_event_reset(session->ready);
            common_session_unlock(session);
            return LIB_STATUS_OK;
        }
        if (!session->accepting) {
            common_session_unlock(session);
            return LIB_STATUS_INVALID_STATE;
        }
        host_sync_event_reset(session->ready);
        common_session_unlock(session);
        wait = host_sync_event_wait(session->ready, timeout_milliseconds);
        if (wait == HOST_SYNC_WAIT_TIMED_OUT) return LIB_STATUS_INVALID_STATE;
        if (wait != HOST_SYNC_WAIT_SIGNALED) return LIB_STATUS_INVALID_STATE;
    }
}

lib_status common_session_reduce_fact(common_session *session,
    const common_session_fact *fact, const ui_frame *frame,
    common_session_plan *out_plan)
{
    common_session_cli_provider provider;
    common_session_cli_machine_observer machine_observer;
    common_session_lifecycle_sink lifecycle_sink;
    common_session_cli_result cli_result;
    void *provider_context;
    void *machine_observer_context;
    lib_status status;

    if (session == LIB_NULL || fact == LIB_NULL || out_plan == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    common_session_plan_clear(out_plan);
    if (fact->run_id != 0u && fact->run_id != session->run_id)
        return LIB_STATUS_INVALID_STATE;
    if (fact->kind == COMMON_SESSION_FACT_FRAME) {
        if (frame == LIB_NULL || !ui_frame_is_valid(frame)) return LIB_STATUS_INVALID_ARGUMENT;
        session->presentation_frame_available = LIB_TRUE;
        session->presentation_frame_graphics = frame->graphics != LIB_FALSE;
        out_plan->frame_ready = LIB_TRUE;
        out_plan->frame = *frame;
        common_session_plan_surface(session, out_plan);
        return LIB_STATUS_OK;
    }
    if (fact->kind == COMMON_SESSION_FACT_UI_COMPLETION) {
        if (!session->ui_action_in_flight ||
            fact->value.ui_completion.completion.action !=
                session->ui_action_in_flight_kind) return LIB_STATUS_INVALID_STATE;
        session->ui_action_in_flight = LIB_FALSE;
        if (fact->value.ui_completion.status != LIB_STATUS_OK) {
            out_plan->ui_failure = LIB_TRUE;
            out_plan->ui_failure_status = fact->value.ui_completion.status;
            return LIB_STATUS_OK;
        }
        session->surface_facts = fact->value.ui_completion.completion.facts;
        common_session_plan_surface(session, out_plan);
        return LIB_STATUS_OK;
    }
    if (fact->kind == COMMON_SESSION_FACT_UI_DELIVERY_FAILED) {
        out_plan->ui_failure = LIB_TRUE;
        out_plan->ui_failure_status = fact->value.ui_delivery_failure.status;
        return LIB_STATUS_OK;
    }
    if (fact->kind == COMMON_SESSION_FACT_MONITOR_TEXT) {
        lib_memory_copy(out_plan->console_text, fact->value.line,
            lib_text_length(fact->value.line) + 1u);
        return LIB_STATUS_OK;
    }
    if (fact->kind == COMMON_SESSION_FACT_CONSOLE_LINE) {
        common_session_lock(session);
        provider = session->cli_provider;
        provider_context = session->cli_provider_context;
        lifecycle_sink = session->lifecycle_sink;
        common_session_unlock(session);
        if (provider == LIB_NULL) return LIB_STATUS_INVALID_STATE;
        lib_memory_set(&cli_result, 0, sizeof(cli_result));
        status = provider(provider_context, fact->value.line, &cli_result);
        if (status != LIB_STATUS_OK) return status;
        lib_memory_copy(out_plan->console_text, cli_result.text,
            sizeof(out_plan->console_text));
        out_plan->console_prompt_ready = cli_result.prompt_ready;
        if (cli_result.prompt_ready) lib_memory_copy(out_plan->console_prompt,
            cli_result.prompt, sizeof(out_plan->console_prompt));
        if (cli_result.lifecycle_request != COMMON_SESSION_LIFECYCLE_NONE) {
            if (lifecycle_sink == LIB_NULL) return LIB_STATUS_INVALID_STATE;
            status = common_session_request_lifecycle(session,
                cli_result.lifecycle_request);
            if (status != LIB_STATUS_OK) return status;
        }
        if (!cli_result.keep_active) {
            common_session_lock(session);
            if (session->cli_provider == provider &&
                session->cli_provider_context == provider_context) {
                session->cli_provider = LIB_NULL;
                session->cli_provider_context = LIB_NULL;
                session->cli_machine_observer = LIB_NULL;
                session->cli_machine_observer_context = LIB_NULL;
            }
            common_session_unlock(session);
        }
        return LIB_STATUS_OK;
    }
    if (fact->kind != COMMON_SESSION_FACT_MACHINE) return LIB_STATUS_OK;
    switch (fact->value.machine.state) {
    case COMMON_SESSION_MACHINE_RUNNING:
        out_plan->notice = session->lifecycle == COMMON_SESSION_MACHINE_PAUSED ?
            COMMON_SESSION_NOTICE_RESUMED : COMMON_SESSION_NOTICE_STARTED;
        out_plan->mouse_capturable_changed = LIB_TRUE;
        out_plan->mouse_capturable = LIB_TRUE;
        break;
    case COMMON_SESSION_MACHINE_PAUSED:
        out_plan->notice = COMMON_SESSION_NOTICE_PAUSED;
        out_plan->mouse_capturable_changed = LIB_TRUE;
        out_plan->mouse_capturable = LIB_FALSE;
        out_plan->release_mouse = LIB_TRUE;
        break;
    case COMMON_SESSION_MACHINE_RESET:
        out_plan->notice = COMMON_SESSION_NOTICE_RESET;
        break;
    case COMMON_SESSION_MACHINE_STOPPED:
    case COMMON_SESSION_MACHINE_FAULT:
        out_plan->notice = COMMON_SESSION_NOTICE_STOPPED;
        out_plan->mouse_capturable_changed = LIB_TRUE;
        out_plan->mouse_capturable = LIB_FALSE;
        out_plan->release_mouse = LIB_TRUE;
        break;
    default:
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    session->lifecycle = fact->value.machine.state;
    common_session_lock(session);
    machine_observer = session->cli_machine_observer;
    machine_observer_context = session->cli_machine_observer_context;
    lifecycle_sink = session->lifecycle_sink;
    common_session_unlock(session);
    if (machine_observer != LIB_NULL) {
        lib_memory_set(&cli_result, 0, sizeof(cli_result));
        status = machine_observer(machine_observer_context,
            fact->value.machine.state, fact->value.machine.status, &cli_result);
        if (status != LIB_STATUS_OK) return status;
        lib_memory_copy(out_plan->console_text, cli_result.text,
            sizeof(out_plan->console_text));
        out_plan->console_prompt_ready = cli_result.prompt_ready;
        if (cli_result.prompt_ready) lib_memory_copy(out_plan->console_prompt,
            cli_result.prompt, sizeof(out_plan->console_prompt));
        if (cli_result.lifecycle_request != COMMON_SESSION_LIFECYCLE_NONE) {
            if (lifecycle_sink == LIB_NULL) return LIB_STATUS_INVALID_STATE;
            return common_session_request_lifecycle(session,
                cli_result.lifecycle_request);
        }
    }
    common_session_plan_surface(session, out_plan);
    return LIB_STATUS_OK;
}

static lib_size common_session_pressed_find(const common_session *session,
    const ui_input_event *event)
{
    lib_size index;

    for (index = 0u; index < session->pressed_count; ++index) {
        const common_session_pressed_key *pressed = &session->pressed[index];
        if (pressed->source_identity == event->source_identity &&
            pressed->event.data.key.scan_code == event->data.key.scan_code &&
            pressed->event.data.key.key == event->data.key.key) return index;
    }
    return session->pressed_count;
}

static void common_session_pressed_forget(common_session *session,
    const ui_input_event *event)
{
    lib_size index = common_session_pressed_find(session, event);
    if (index != session->pressed_count)
        session->pressed[index] = session->pressed[--session->pressed_count];
}

static lib_status common_session_pressed_remember(common_session *session,
    const ui_input_event *event)
{
    if (common_session_pressed_find(session, event) != session->pressed_count)
        return LIB_STATUS_OK;
    if (session->pressed_count == COMMON_SESSION_PRESSED_CAPACITY)
        return LIB_STATUS_LIMIT_EXCEEDED;
    session->pressed[session->pressed_count].source_identity = event->source_identity;
    session->pressed[session->pressed_count].event = *event;
    ++session->pressed_count;
    return LIB_STATUS_OK;
}

lib_status common_session_dispatch_host_input(common_session *session,
    const ui_input_event *event, common_session_input_sink sink,
    void *sink_context)
{
    lib_size index = 0u;
    lib_status status = LIB_STATUS_OK;

    if (session == LIB_NULL || event == LIB_NULL || sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_SOURCE_RETIRED) {
        while (index < session->pressed_count) {
            common_session_pressed_key *pressed = &session->pressed[index];
            if (pressed->source_identity != event->source_identity) { ++index; continue; }
            pressed->event.data.key.pressed = LIB_FALSE;
            if (common_session_is_running(session) && status == LIB_STATUS_OK)
                status = sink(sink_context, &pressed->event);
            session->pressed[index] = session->pressed[--session->pressed_count];
        }
        return status;
    }
    if (!common_session_is_running(session)) return LIB_STATUS_OK;
    if (event->type == UI_EVENT_KEY && event->data.key.pressed) {
        status = common_session_pressed_remember(session, event);
        if (status != LIB_STATUS_OK) return status;
        status = sink(sink_context, event);
        if (status != LIB_STATUS_OK) common_session_pressed_forget(session, event);
        return status;
    }
    if (event->type == UI_EVENT_KEY && !event->data.key.pressed) {
        common_session_pressed_forget(session, event);
        return sink(sink_context, event);
    }
    return (event->type == UI_EVENT_MOUSE || event->type == UI_EVENT_TEXT) ?
        sink(sink_context, event) : LIB_STATUS_OK;
}
