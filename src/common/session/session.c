#include "common/session/session_interface.h"

#include "common/session/control.h"
#include "common/session/control_state.h"
#include "common/ui/ui_interface.h"


struct common_session {
    common_session_queue queue;
    common_session_state state;
    kvm_frame frame;
    common_machine *machine;
    common_session_command_provider command;
    common_ui *ui;
    lib_bool pending_line;
};

static common_ui_state common_session_ui_state(common_session_machine_state state)
{
    switch (state) {
    case COMMON_SESSION_MACHINE_RUNNING: return COMMON_UI_STATE_RUNNING;
    case COMMON_SESSION_MACHINE_PAUSED: return COMMON_UI_STATE_PAUSED;
    case COMMON_SESSION_MACHINE_ERROR: return COMMON_UI_STATE_ERROR;
    default: return COMMON_UI_STATE_STOPPED;
    }
}

static void common_session_clear_result(common_session_command_result *result)
{
    if (result != NULL) *result = (common_session_command_result) { 0 };
}

static int common_session_deliver_machine_input(void *context,
    const kvm_input_event *event)
{
    return common_machine_enqueue_input((common_machine *)context, event) != 0;
}

static int common_session_dispatch_request(common_session *session,
    common_session_request request)
{
    if (request == COMMON_SESSION_REQUEST_NONE) return 1;
    if (session == NULL || session->machine == NULL) return 0;
    switch (request) {
    case COMMON_SESSION_REQUEST_START: return common_machine_start(session->machine);
    case COMMON_SESSION_REQUEST_RESUME: return common_machine_resume(session->machine);
    case COMMON_SESSION_REQUEST_PAUSE: return common_machine_pause(session->machine);
    case COMMON_SESSION_REQUEST_STOP: return common_machine_stop(session->machine);
    case COMMON_SESSION_REQUEST_RESET: return common_machine_reset(session->machine);
    default: return 0;
    }
}

static int common_session_write_text(common_session *session, const char *text)
{
    char chunk[1024];
    lib_size used = 0u;
    char previous = '\0';
    if (text == NULL) return 1;
    for (; *text != '\0'; ++text) {
        if (used + 2u >= sizeof(chunk)) {
            chunk[used] = '\0';
            if (common_ui_write_monitor(session->ui, chunk) != LIB_STATUS_OK) return 0;
            used = 0u;
        }
        if (*text == '\n' && previous != '\r') chunk[used++] = '\r';
        chunk[used++] = *text;
        previous = *text;
    }
    chunk[used] = '\0';
    return used == 0u || common_ui_write_monitor(session->ui, chunk) == LIB_STATUS_OK;
}

static int common_session_apply_result(common_session *session,
    const common_session_command_result *result)
{
    if (result->release_window_mouse &&
        common_ui_release_window_mouse(session->ui) != LIB_STATUS_OK) return 0;
    if ((result->text[0] != '\0' || (result->detail != NULL && result->detail[0] != '\0')) &&
        common_session_state_monitor_is_current(&session->state)) {
        if (session->pending_line) {
            lib_bool completed;
            if (common_ui_cancel_monitor_line(session->ui, &completed) != LIB_STATUS_OK) return 0;
            session->pending_line = completed;
            if (!completed && common_ui_write_monitor(session->ui, "\r\n") != LIB_STATUS_OK)
                return 0;
        }
        if (!common_session_write_text(session, result->text) ||
            !common_session_write_text(session, result->detail)) return 0;
    }
    if (!common_session_dispatch_request(session, result->request)) return 0;
    if (result->request != COMMON_SESSION_REQUEST_NONE ||
        !result->arm_prompt || session->pending_line ||
        !common_session_state_monitor_is_current(&session->state)) return 1;
    if (common_ui_write_monitor(session->ui, result->prompt) != LIB_STATUS_OK ||
        common_ui_request_monitor_line(session->ui) != LIB_STATUS_OK) return 0;
    session->pending_line = LIB_TRUE;
    return 1;
}

static int common_session_arm_if_ready(common_session *session)
{
    common_session_command_result result;
    if (session == NULL || session->command.note_monitor_current == NULL) return 0;
    common_session_clear_result(&result);
    session->command.note_monitor_current(session->command.context,
        common_session_state_monitor_is_current(&session->state), &result);
    return common_session_apply_result(session, &result);
}

static int common_session_drive(common_session *session)
{
    common_ui_action action;
    lib_bool console_status_surface;
    if (session == NULL || session->ui == NULL || session->machine == NULL)
        return 0;
    common_ui_set_run_generation(session->ui,
        common_machine_run_generation(session->machine));
    action = common_session_state_take_action(&session->state);
    if (action != COMMON_UI_ACTION_NONE &&
        common_ui_apply_action(session->ui, action,
            common_session_ui_state(session->state.runtime_actual)) != LIB_STATUS_OK)
        return 0;
    if (session->state.observed_frame_sequence == 0u ||
        !common_session_state_frame_targets_ready(&session->state)) return 1;
    console_status_surface =
        session->state.display == COMMON_SESSION_DISPLAY_CONSOLE &&
        !session->state.console_control && session->frame.graphics != 0u;
    return common_ui_publish_frame(session->ui, &session->frame,
        session->state.window_actual,
        session->state.vm_console_actual &&
            session->state.current_console_actual == COMMON_SESSION_CONSOLE_VM,
        console_status_surface) == LIB_STATUS_OK;
}

static int common_session_handle_kvm_input(common_session *session,
    const kvm_input_event *event)
{
    common_session_command_result result;
    common_session_machine_state state;
    if (session == NULL || event == NULL) return 0;
    state = session->state.monitor_actual;
    if (event->type == KVM_EVENT_WINDOW_CLOSE) {
        if (state == COMMON_SESSION_MACHINE_RUNNING &&
            (session->command.begin_external == NULL ||
             !session->command.begin_external(session->command.context, state,
                 COMMON_SESSION_REQUEST_PAUSE))) return 0;
        common_session_state_note_window_close(&session->state);
        return state != COMMON_SESSION_MACHINE_RUNNING ||
            common_session_dispatch_request(session, COMMON_SESSION_REQUEST_PAUSE);
    }
    if (event->type == KVM_EVENT_HOTKEY) {
        if (session->command.handle_hotkey == NULL) return 0;
        common_session_clear_result(&result);
        if (!session->command.handle_hotkey(session->command.context, state,
                event->data.hotkey.identifier, &result)) return 0;
        return common_session_apply_result(session, &result);
    }
    return common_session_dispatch_input(&session->queue, event, state,
        common_session_deliver_machine_input, session->machine);
}

static int common_session_process_completed(common_session *session,
    const common_session_event *event)
{
    common_session_command_result result;
    lib_bool broker_monitor_completed = LIB_FALSE;
    if (session == NULL || event == NULL) return 0;
    if (event->run_generation != 0u &&
        event->run_generation != common_machine_run_generation(session->machine))
        return 1;
    common_session_clear_result(&result);
    if (event->kind == COMMON_SESSION_EVENT_RUNTIME_COMPLETED) {
        if (session->command.note_runtime == NULL) return 0;
        session->command.note_runtime(session->command.context,
            session->state.monitor_actual, event->value.runtime_state, &result);
        common_session_state_note_runtime(&session->state, event->value.runtime_state);
    } else if (event->kind == COMMON_SESSION_EVENT_FRAME_COMPLETED) {
        lib_u32 sequence = event->value.frame.sequence;
        if (sequence > session->state.observed_frame_sequence &&
            common_machine_copy_published_frame(session->machine, &session->frame,
                event->run_generation))
            (void)common_session_state_note_frame(&session->state, session->frame.sequence,
                session->frame.graphics != 0u);
    } else if (event->kind == COMMON_SESSION_EVENT_COMPONENT_COMPLETED) {
        if (event->value.component.component == COMMON_SESSION_EVENT_COMPONENT_WINDOW)
            common_session_state_note_window(&session->state,
                event->value.component.exists);
        else common_session_state_note_vm_console(&session->state,
            event->value.component.exists);
    } else if (event->kind == COMMON_SESSION_EVENT_BROKER_COMPLETED) {
        /* The broker's completed takeover includes old-reader join. */
        session->pending_line = LIB_FALSE;
        common_session_state_note_current_console(&session->state,
            event->value.broker_vm_console_current);
        broker_monitor_completed = !event->value.broker_vm_console_current;
    } else return 1;
    if (event->kind == COMMON_SESSION_EVENT_RUNTIME_COMPLETED &&
        !common_session_apply_result(session, &result)) return 0;
    /* Runtime wording is held by the injected command provider until the
     * monitor is Current.  A raw Console must never receive monitor status
     * text merely because its VM completion arrived first. */
    if (!common_session_drive(session)) return 0;
    if ((event->kind == COMMON_SESSION_EVENT_RUNTIME_COMPLETED ||
         event->kind == COMMON_SESSION_EVENT_BROKER_COMPLETED) &&
        (session->state.runtime_actual != COMMON_SESSION_MACHINE_RUNNING ||
         common_session_state_frame_targets_ready(&session->state)) &&
        common_ui_set_state(session->ui,
            common_session_ui_state(session->state.runtime_actual)) != LIB_STATUS_OK)
        return 0;
    if (broker_monitor_completed && common_session_state_monitor_is_current(&session->state) &&
        session->command.note_broker != NULL)
        session->command.note_broker(session->command.context,
            session->state.monitor_actual, LIB_FALSE,
            common_session_state_monitor_is_running_graphics_surface(&session->state));
    return event->kind == COMMON_SESSION_EVENT_FRAME_COMPLETED ? 1 :
        common_session_arm_if_ready(session);
}

lib_status common_session_create(common_session **out_session,
    const common_session_options *options)
{
    common_session *session;
    if (out_session == NULL || options == NULL || options->machine == NULL ||
        options->command.open == NULL ||
        options->command.submit_line == NULL || options->command.note_runtime == NULL ||
        options->command.note_monitor_current == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_session = NULL;
    session = lib_allocate_zero(1u, sizeof(*session));
    if (session == NULL) return LIB_STATUS_NO_MEMORY;
    if (!common_session_queue_initialize(&session->queue)) {
        lib_release(session);
        return LIB_STATUS_NO_MEMORY;
    }
    session->machine = options->machine;
    session->command = options->command;
    common_session_state_initialize(&session->state, options->display,
        options->console_control);
    *out_session = session;
    return LIB_STATUS_OK;
}

lib_status common_session_bind_ui(common_session *session, common_ui *ui)
{
    if (session == NULL || ui == NULL || session->ui != NULL) return LIB_STATUS_INVALID_ARGUMENT;
    session->ui = ui;
    return LIB_STATUS_OK;
}

lib_status common_session_destroy(common_session *session)
{
    if (session == NULL) return LIB_STATUS_OK;
    common_session_queue_dispose(&session->queue);
    lib_release(session);
    return LIB_STATUS_OK;
}

int common_session_enqueue_ui_event(void *context, const common_ui_event *event)
{
    common_session *session = (common_session *)context;
    if (session == NULL || event == NULL) return 0;
    switch (event->kind) {
    case COMMON_UI_EVENT_KVM_INPUT:
        return common_session_queue_push_kvm_for_run(&session->queue, &event->value.kvm,
            event->run_generation);
    case COMMON_UI_EVENT_MONITOR_LINE:
        return common_session_queue_push_monitor_line(&session->queue, &event->value.line,
            event->monitor_line_rejected);
    case COMMON_UI_EVENT_COMPONENT_COMPLETED:
        return common_session_queue_push_component_completed(&session->queue,
            event->value.component.component == COMMON_UI_COMPONENT_WINDOW ?
                COMMON_SESSION_EVENT_COMPONENT_WINDOW :
                COMMON_SESSION_EVENT_COMPONENT_VM_CONSOLE,
            event->value.component.exists, event->run_generation);
    case COMMON_UI_EVENT_BROKER_COMPLETED:
        return common_session_queue_push_broker_completed(&session->queue,
            event->value.broker_vm_console_current, event->run_generation);
    case COMMON_UI_EVENT_KVM_DELIVERY_FAILED:
        return common_session_queue_push_kvm_delivery_failed(&session->queue,
            event->value.delivery_failure.source_identity,
            event->value.delivery_failure.status, event->run_generation);
    case COMMON_UI_EVENT_CONSOLE_FAILED:
        return common_session_queue_push_console_failed(&session->queue);
    }
    return 0;
}

int common_session_enqueue_runtime_completed(common_session *session,
    common_session_machine_state state, lib_u32 run_generation)
{
    return session != NULL && common_session_queue_push_runtime_completed(&session->queue,
        state, run_generation);
}

int common_session_enqueue_frame_completed(common_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 run_generation)
{
    return session != NULL && common_session_queue_push_frame_completed(&session->queue,
        sequence, graphics, run_generation);
}

int common_session_run(common_session *session)
{
    common_session_command_result result;
    char line[COMMON_SESSION_TEXT_CAPACITY];
    if (session == NULL || session->ui == NULL) return 0;
    common_session_clear_result(&result);
    session->command.open(session->command.context, &result);
    if (!common_session_apply_result(session, &result) || !common_session_arm_if_ready(session))
        return 0;
    for (;;) {
        common_session_event event;
        /* No periodic work: a failed indefinite wait is terminal, not idle. */
        if (!common_session_queue_take(&session->queue, &event, LIB_UINT32_MAX)) return 0;
        if (event.kind == COMMON_SESSION_EVENT_KVM_INPUT) {
            if (!common_session_accept_kvm_event(&event,
                    common_machine_run_generation(session->machine),
                    session->state.monitor_actual)) continue;
            if (!common_session_handle_kvm_input(session, &event.value.kvm) ||
                !common_session_drive(session) || !common_session_arm_if_ready(session)) return 0;
            continue;
        }
        if (event.kind == COMMON_SESSION_EVENT_MONITOR_LINE) {
            session->pending_line = LIB_FALSE;
            common_session_clear_result(&result);
            if (event.monitor_line_rejected) {
                if (session->command.reject_line == NULL) return 0;
                session->command.reject_line(session->command.context, &result);
            } else {
                if (event.value.line.length >= sizeof(line)) return 0;
                lib_memory_copy(line, event.value.line.text, event.value.line.length);
                line[event.value.line.length] = '\0';
                session->command.submit_line(session->command.context,
                    session->state.monitor_actual, line, &result);
            }
            if (result.exit_requested) {
                (void)common_session_dispatch_request(session, COMMON_SESSION_REQUEST_STOP);
                return 1;
            }
            if (!common_session_apply_result(session, &result) ||
                !common_session_drive(session) || !common_session_arm_if_ready(session)) return 0;
            continue;
        }
        if (event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED ||
            event.kind == COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED ||
            event.kind == COMMON_SESSION_EVENT_QUEUE_DELIVERY_FAILED) {
            const char *text = event.kind == COMMON_SESSION_EVENT_CONSOLE_FAILED ?
                "Console input failed.\r\n" :
                event.kind == COMMON_SESSION_EVENT_KVM_DELIVERY_FAILED ?
                "KVM input delivery failed.\r\n" : "Control queue delivery failed.\r\n";
            (void)common_ui_write_monitor(session->ui, text);
            return 0;
        }
        if (!common_session_process_completed(session, &event)) return 0;
    }
}
