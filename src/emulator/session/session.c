#include "emulator/session/session_interface.h"

#include "emulator/session/control.h"
#include "emulator/session/control_state.h"
#include "emulator/ui/ui_interface.h"
#include "lib/types/file.h"


struct emulator_session {
    emulator_session_queue queue;
    emulator_session_state state;
    emulator_machine_frame frame;
    emulator_machine *machine;
    emulator_session_command_provider command;
    emulator_ui *ui;
    lib_bool pending_line;
    emulator_session_request pending_request;
    lib_bool prompt_due;
    char pending_monitor_text[EMULATOR_SESSION_TEXT_CAPACITY];
    char pending_prompt[EMULATOR_SESSION_PROMPT_CAPACITY];
};

static emulator_ui_state emulator_session_ui_state(emulator_session_machine_state state)
{
    switch (state) {
    case EMULATOR_SESSION_MACHINE_RUNNING: return EMULATOR_UI_STATE_RUNNING;
    case EMULATOR_SESSION_MACHINE_PAUSED: return EMULATOR_UI_STATE_PAUSED;
    case EMULATOR_SESSION_MACHINE_ERROR: return EMULATOR_UI_STATE_ERROR;
    default: return EMULATOR_UI_STATE_STOPPED;
    }
}

static void emulator_session_clear_result(emulator_session_command_result *result)
{
    if (result != NULL) *result = (emulator_session_command_result) { 0 };
}

static lib_bool emulator_session_deliver_machine_input(void *context,
    const kvm_input_event *event)
{
    return emulator_machine_enqueue_input((emulator_machine *)context, event) != 0;
}

static lib_bool emulator_session_dispatch_request(emulator_session *session,
    emulator_session_request request)
{
    if (request == EMULATOR_SESSION_REQUEST_NONE) return LIB_TRUE;
    if (session == NULL || session->machine == NULL) return LIB_FALSE;
    switch (request) {
    case EMULATOR_SESSION_REQUEST_START: return emulator_machine_start(session->machine);
    case EMULATOR_SESSION_REQUEST_RESUME: return emulator_machine_resume(session->machine);
    case EMULATOR_SESSION_REQUEST_PAUSE: return emulator_machine_pause(session->machine);
    case EMULATOR_SESSION_REQUEST_STOP: return emulator_machine_stop(session->machine);
    case EMULATOR_SESSION_REQUEST_RESET: return emulator_machine_reset(session->machine);
    default: return LIB_FALSE;
    }
}

static lib_bool emulator_session_write_text(emulator_session *session, const char *text)
{
    char chunk[1024];
    lib_size used = 0u;
    char previous = '\0';
    if (text == NULL) return LIB_TRUE;
    for (; *text != '\0'; ++text) {
        if (used + 2u >= sizeof(chunk)) {
            chunk[used] = '\0';
            if (emulator_ui_write_monitor(session->ui, chunk) != LIB_STATUS_OK) return LIB_FALSE;
            used = 0u;
        }
        if (*text == '\n' && previous != '\r') chunk[used++] = '\r';
        chunk[used++] = *text;
        previous = *text;
    }
    chunk[used] = '\0';
    return used == 0u || emulator_ui_write_monitor(session->ui, chunk) == LIB_STATUS_OK;
}

static lib_bool emulator_session_append_pending_text(emulator_session *session, const char *text)
{
    lib_size used;
    lib_size available;
    lib_size length;
    if (session == NULL || text == NULL || text[0] == '\0') return LIB_TRUE;
    used = lib_text_length(session->pending_monitor_text);
    available = sizeof(session->pending_monitor_text) - used;
    length = lib_text_length(text);
    if (available == 0u || length >= available) return LIB_FALSE;
    lib_memory_copy(session->pending_monitor_text + used, text, length + 1u);
    return LIB_TRUE;
}

static void emulator_session_note_prompt(emulator_session *session,
    const emulator_session_command_result *result)
{
    const char *prompt;
    if (session == NULL || result == NULL || !result->arm_prompt) return;
    prompt = result->prompt[0] != '\0' ? result->prompt : "> ";
    (void)lib_c_snprintf(session->pending_prompt, sizeof(session->pending_prompt), "%s", prompt);
    session->prompt_due = LIB_TRUE;
}

static lib_bool emulator_session_arm_if_ready(emulator_session *session)
{
    if (session == NULL || !emulator_session_state_monitor_is_current(&session->state)) return LIB_TRUE;
    if (session->pending_monitor_text[0] != '\0') {
        if (session->pending_line) {
            lib_bool completed;
            if (emulator_ui_cancel_monitor_line(session->ui, &completed) != LIB_STATUS_OK) return LIB_FALSE;
            session->pending_line = completed;
            if (!completed && emulator_ui_write_monitor(session->ui, "\r\n") != LIB_STATUS_OK)
                return LIB_FALSE;
        }
        if (!emulator_session_write_text(session, session->pending_monitor_text)) return LIB_FALSE;
        session->pending_monitor_text[0] = '\0';
    }
    if (session->pending_request != EMULATOR_SESSION_REQUEST_NONE ||
        !session->prompt_due || session->pending_line) return LIB_TRUE;
    if (emulator_ui_write_monitor(session->ui,
            session->pending_prompt[0] != '\0' ? session->pending_prompt : "> ") != LIB_STATUS_OK ||
        emulator_ui_request_monitor_line(session->ui) != LIB_STATUS_OK) return LIB_FALSE;
    session->pending_line = LIB_TRUE;
    session->prompt_due = LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool emulator_session_apply_result(emulator_session *session,
    const emulator_session_command_result *result)
{
    if (result->release_window_mouse &&
        emulator_ui_release_window_mouse(session->ui) != LIB_STATUS_OK) return LIB_FALSE;
    if ((result->text[0] != '\0' || (result->detail != NULL && result->detail[0] != '\0')) &&
        emulator_session_state_monitor_is_current(&session->state)) {
        if (session->pending_line) {
            lib_bool completed;
            if (emulator_ui_cancel_monitor_line(session->ui, &completed) != LIB_STATUS_OK) return LIB_FALSE;
            session->pending_line = completed;
            if (!completed && emulator_ui_write_monitor(session->ui, "\r\n") != LIB_STATUS_OK)
                return LIB_FALSE;
        }
        if (!emulator_session_write_text(session, result->text) ||
            !emulator_session_write_text(session, result->detail)) return LIB_FALSE;
    } else if (!emulator_session_append_pending_text(session, result->text) ||
        !emulator_session_append_pending_text(session, result->detail)) {
        return LIB_FALSE;
    }
    if (result->request != EMULATOR_SESSION_REQUEST_NONE) {
        if (session->pending_request != EMULATOR_SESSION_REQUEST_NONE) {
            return emulator_session_append_pending_text(session,
                "Machine state transition is in progress.\r\n\r\n");
        }
        if (!emulator_session_dispatch_request(session, result->request)) return LIB_FALSE;
        session->pending_request = result->request;
        session->prompt_due = LIB_FALSE;
        return LIB_TRUE;
    }
    emulator_session_note_prompt(session, result);
    return LIB_TRUE;
}

static lib_bool emulator_session_drive(emulator_session *session)
{
    emulator_ui_action action;
    lib_bool console_status_surface;
    if (session == NULL || session->ui == NULL || session->machine == NULL)
        return LIB_FALSE;
    emulator_ui_set_run_generation(session->ui,
        emulator_machine_run_generation(session->machine));
    action = emulator_session_state_take_action(&session->state);
    if (action != EMULATOR_UI_ACTION_NONE &&
        emulator_ui_apply_action(session->ui, action,
            emulator_session_ui_state(session->state.runtime_actual)) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (session->state.observed_frame_sequence == 0u ||
        !emulator_session_state_frame_targets_ready(&session->state)) return LIB_TRUE;
    console_status_surface =
        session->state.display == EMULATOR_SESSION_DISPLAY_CONSOLE &&
        !session->state.console_control && session->frame.window.graphics != 0u;
    return emulator_ui_publish_frame(session->ui, &session->frame.window,
        &session->frame.characters, session->frame.sequence,
        session->state.window_actual,
        session->state.vm_console_actual &&
            session->state.current_console_actual == EMULATOR_SESSION_CONSOLE_VM,
        console_status_surface) == LIB_STATUS_OK;
}

static lib_bool emulator_session_handle_kvm_input(emulator_session *session,
    const kvm_input_event *event)
{
    emulator_session_command_result result;
    emulator_session_machine_state state;
    if (session == NULL || event == NULL) return LIB_FALSE;
    state = session->state.monitor_actual;
    if (event->type == KVM_EVENT_WINDOW_CLOSE) {
        emulator_session_state_note_window_close(&session->state);
        if (state != EMULATOR_SESSION_MACHINE_RUNNING ||
            session->pending_request != EMULATOR_SESSION_REQUEST_NONE) return LIB_TRUE;
        emulator_session_clear_result(&result);
        result.request = EMULATOR_SESSION_REQUEST_PAUSE;
        return emulator_session_apply_result(session, &result);
    }
    if (event->type == KVM_EVENT_HOTKEY) {
        if (session->command.handle_hotkey == NULL) return LIB_FALSE;
        emulator_session_clear_result(&result);
        if (!session->command.handle_hotkey(session->command.context, state,
                event->data.hotkey.identifier, &result)) return LIB_FALSE;
        return emulator_session_apply_result(session, &result);
    }
    return emulator_session_dispatch_input(&session->queue, event, state,
        emulator_session_deliver_machine_input, session->machine);
}

static lib_bool emulator_session_process_completed(emulator_session *session,
    const emulator_session_event *event)
{
    emulator_session_command_result result;
    lib_bool broker_monitor_completed = LIB_FALSE;
    if (session == NULL || event == NULL) return LIB_FALSE;
    if (event->run_generation != 0u &&
        event->run_generation != emulator_machine_run_generation(session->machine))
        return LIB_TRUE;
    emulator_session_clear_result(&result);
    if (event->kind == EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED) {
        if (session->command.note_runtime == NULL) return LIB_FALSE;
        session->command.note_runtime(session->command.context,
            session->state.monitor_actual, event->value.runtime_state, &result);
        emulator_session_state_note_runtime(&session->state, event->value.runtime_state);
        if (event->value.runtime_state != EMULATOR_SESSION_MACHINE_INIT)
            session->pending_request = EMULATOR_SESSION_REQUEST_NONE;
    } else if (event->kind == EMULATOR_SESSION_EVENT_FRAME_COMPLETED) {
        lib_u32 sequence = event->value.frame.sequence;
        if (emulator_session_frame_is_newer(sequence, session->state.observed_frame_sequence) &&
            emulator_machine_copy_published_frame(session->machine, &session->frame,
                event->run_generation))
            (void)emulator_session_state_note_frame(&session->state, session->frame.sequence,
                session->frame.window.graphics != 0u);
    } else if (event->kind == EMULATOR_SESSION_EVENT_COMPONENT_COMPLETED) {
        if (event->value.component.component == EMULATOR_SESSION_EVENT_COMPONENT_WINDOW)
            emulator_session_state_note_window(&session->state,
                event->value.component.exists);
        else emulator_session_state_note_vm_console(&session->state,
            event->value.component.exists);
    } else if (event->kind == EMULATOR_SESSION_EVENT_BROKER_COMPLETED) {
        /* The broker's completed takeover includes old-reader join. */
        session->pending_line = LIB_FALSE;
        emulator_session_state_note_current_console(&session->state,
            event->value.broker_vm_console_current);
        broker_monitor_completed = !event->value.broker_vm_console_current;
    } else return LIB_TRUE;
    if (event->kind == EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED &&
        !emulator_session_apply_result(session, &result)) return LIB_FALSE;
    if (!emulator_session_drive(session)) return LIB_FALSE;
    if ((event->kind == EMULATOR_SESSION_EVENT_RUNTIME_COMPLETED ||
         event->kind == EMULATOR_SESSION_EVENT_BROKER_COMPLETED) &&
        (session->state.runtime_actual != EMULATOR_SESSION_MACHINE_RUNNING ||
         emulator_session_state_frame_targets_ready(&session->state)) &&
        emulator_ui_set_state(session->ui,
            emulator_session_ui_state(session->state.runtime_actual)) != LIB_STATUS_OK)
        return LIB_FALSE;
    if (broker_monitor_completed && emulator_session_state_monitor_is_current(&session->state) &&
        session->state.monitor_actual == EMULATOR_SESSION_MACHINE_RUNNING &&
        emulator_session_state_monitor_is_running_graphics_surface(&session->state))
        session->prompt_due = LIB_TRUE;
    if (event->kind == EMULATOR_SESSION_EVENT_BROKER_COMPLETED &&
        event->value.broker_vm_console_current &&
        session->state.monitor_actual == EMULATOR_SESSION_MACHINE_RUNNING) {
        session->prompt_due = LIB_FALSE;
        session->pending_monitor_text[0] = '\0';
    }
    return event->kind == EMULATOR_SESSION_EVENT_FRAME_COMPLETED ? 1 :
        emulator_session_arm_if_ready(session);
}

lib_status emulator_session_create(emulator_session **out_session,
    const emulator_session_options *options)
{
    emulator_session *session;
    if (out_session == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = NULL;
    if (options == NULL || options->machine == NULL ||
        options->command.open == NULL ||
        options->command.submit_line == NULL || options->command.note_runtime == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    session = lib_allocate_zero(1u, sizeof(*session));
    if (session == NULL) return LIB_STATUS_NO_MEMORY;
    if (!emulator_session_queue_initialize(&session->queue)) {
        lib_release(session);
        return LIB_STATUS_NO_MEMORY;
    }
    session->machine = options->machine;
    session->command = options->command;
    emulator_session_state_initialize(&session->state, options->display,
        options->console_control);
    *out_session = session;
    return LIB_STATUS_OK;
}

lib_status emulator_session_bind_ui(emulator_session *session, emulator_ui *ui)
{
    if (session == NULL || ui == NULL || session->ui != NULL) return LIB_STATUS_INVALID_ARGUMENT;
    session->ui = ui;
    return LIB_STATUS_OK;
}

lib_status emulator_session_destroy(emulator_session *session)
{
    if (session == NULL) return LIB_STATUS_OK;
    emulator_session_queue_dispose(&session->queue);
    lib_release(session);
    return LIB_STATUS_OK;
}

lib_bool emulator_session_enqueue_ui_event(void *context, const emulator_ui_event *event)
{
    emulator_session *session = (emulator_session *)context;
    if (session == NULL || event == NULL) return LIB_FALSE;
    switch (event->kind) {
    case EMULATOR_UI_EVENT_KVM_INPUT:
        return emulator_session_queue_push_kvm_for_run(&session->queue, &event->value.kvm,
            event->run_generation);
    case EMULATOR_UI_EVENT_MONITOR_LINE:
        return emulator_session_queue_push_monitor_line(&session->queue, &event->value.line,
            event->monitor_line_rejected);
    case EMULATOR_UI_EVENT_COMPONENT_COMPLETED:
        return emulator_session_queue_push_component_completed(&session->queue,
            event->value.component.component == EMULATOR_UI_COMPONENT_WINDOW ?
                EMULATOR_SESSION_EVENT_COMPONENT_WINDOW :
                EMULATOR_SESSION_EVENT_COMPONENT_VM_CONSOLE,
            event->value.component.exists, event->run_generation);
    case EMULATOR_UI_EVENT_BROKER_COMPLETED:
        return emulator_session_queue_push_broker_completed(&session->queue,
            event->value.broker_vm_console_current, event->run_generation);
    case EMULATOR_UI_EVENT_KVM_DELIVERY_FAILED:
        return emulator_session_queue_push_kvm_delivery_failed(&session->queue,
            event->value.delivery_failure.source_identity,
            event->value.delivery_failure.status, event->run_generation);
    case EMULATOR_UI_EVENT_CONSOLE_FAILED:
        return emulator_session_queue_push_console_failed(&session->queue);
    }
    return LIB_FALSE;
}

lib_bool emulator_session_enqueue_runtime_completed(emulator_session *session,
    emulator_session_machine_state state, lib_u32 run_generation)
{
    return session != NULL && emulator_session_queue_push_runtime_completed(&session->queue,
        state, run_generation);
}

lib_bool emulator_session_enqueue_frame_completed(emulator_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 run_generation)
{
    return session != NULL && emulator_session_queue_push_frame_completed(&session->queue,
        sequence, graphics, run_generation);
}

lib_bool emulator_session_run(emulator_session *session)
{
    emulator_session_command_result result;
    char line[EMULATOR_SESSION_TEXT_CAPACITY];
    if (session == NULL || session->ui == NULL) return LIB_FALSE;
    emulator_session_clear_result(&result);
    session->command.open(session->command.context, &result);
    if (!emulator_session_apply_result(session, &result) || !emulator_session_arm_if_ready(session))
        return LIB_FALSE;
    for (;;) {
        emulator_session_event event;
        /* No periodic work: a failed indefinite wait is terminal, not idle. */
        if (!emulator_session_queue_take(&session->queue, &event, LIB_UINT32_MAX)) return LIB_FALSE;
        if (event.kind == EMULATOR_SESSION_EVENT_KVM_INPUT) {
            if (!emulator_session_accept_kvm_event(&event,
                    emulator_machine_run_generation(session->machine),
                    session->state.monitor_actual)) continue;
            if (!emulator_session_handle_kvm_input(session, &event.value.kvm) ||
                !emulator_session_drive(session) || !emulator_session_arm_if_ready(session)) return LIB_FALSE;
            continue;
        }
        if (event.kind == EMULATOR_SESSION_EVENT_MONITOR_LINE) {
            session->pending_line = LIB_FALSE;
            emulator_session_clear_result(&result);
            if (event.monitor_line_rejected) {
                if (session->command.reject_line == NULL) return LIB_FALSE;
                session->command.reject_line(session->command.context, &result);
            } else {
                if (event.value.line.length >= sizeof(line)) return LIB_FALSE;
                lib_memory_copy(line, event.value.line.text, event.value.line.length);
                line[event.value.line.length] = '\0';
                session->command.submit_line(session->command.context,
                    session->state.monitor_actual, line, &result);
            }
            if (result.exit_requested) {
                (void)emulator_session_dispatch_request(session, EMULATOR_SESSION_REQUEST_STOP);
                return LIB_TRUE;
            }
            if (!emulator_session_apply_result(session, &result) ||
                !emulator_session_drive(session) || !emulator_session_arm_if_ready(session)) return LIB_FALSE;
            continue;
        }
        if (event.kind == EMULATOR_SESSION_EVENT_CONSOLE_FAILED ||
            event.kind == EMULATOR_SESSION_EVENT_KVM_DELIVERY_FAILED ||
            event.kind == EMULATOR_SESSION_EVENT_QUEUE_DELIVERY_FAILED) {
            const char *text = event.kind == EMULATOR_SESSION_EVENT_CONSOLE_FAILED ?
                "Console input failed.\r\n" :
                event.kind == EMULATOR_SESSION_EVENT_KVM_DELIVERY_FAILED ?
                "KVM input delivery failed.\r\n" : "Control queue delivery failed.\r\n";
            (void)emulator_ui_write_monitor(session->ui, text);
            return LIB_FALSE;
        }
        if (!emulator_session_process_completed(session, &event)) return LIB_FALSE;
    }
}
