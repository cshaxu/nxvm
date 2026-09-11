#include "type.h"

#include <limits.h>

#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_interface.h"
#include "vm/machine/request_factory.h"
#include "vm/session/session.h"

struct vm_session {
    vm_machine *machine;
    vm_session_control *control;
    vm_presentation_surface requested_target;
    vm_presentation_surface active_target;
    vm_machine_result_kind lifecycle;
};

static C_VOID vm_session_plan_clear(vm_presentation_plan *plan)
{
    if (plan != STD_NULL) STD_MEMSET(plan, 0, sizeof(*plan));
}

static C_VOID vm_session_plan_target(vm_session *session,
    vm_presentation_plan *plan, vm_presentation_surface target)
{
    if (session == STD_NULL || plan == STD_NULL || session->active_target == target)
        return;
    session->active_target = target;
    plan->target_changed = TYPE_TRUE;
    plan->target = target;
}

static C_VOID vm_session_plan_title(vm_presentation_plan *plan,
    const C_CHAR *title)
{
    if (plan == STD_NULL || title == STD_NULL) return;
    plan->title_changed = TYPE_TRUE;
    (C_VOID)snprintf(plan->title, sizeof(plan->title), "%s", title);
}

type_status vm_session_create(vm_session **out_session)
{
    vm_session *session;

    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_session = STD_NULL;
    session = STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (vm_session_control_create(&session->control) != TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_NO_MEMORY;
    }
    *out_session = session;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_destroy(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_machine_destroy(session->machine);
    vm_session_control_destroy(session->control);
    STD_FREE(session);
}

vm_session_control *vm_session_get_control(vm_session *session)
{
    return session == STD_NULL ? STD_NULL : session->control;
}

type_status vm_session_open_profile(vm_session *session,
    const vm_session_request *request)
{
    if (session == STD_NULL || request == STD_NULL || session->machine != STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    if (vm_machine_create_from_request(request, &session->machine) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    vm_machine_set_result_sink(session->machine,
        vm_session_control_machine_result_sink, session->control);
    return TYPE_STATUS_OK;
}

type_status vm_session_set_presentation_target(vm_session *session,
    vm_presentation_surface target)
{
    if (session == STD_NULL || target > VM_PRESENTATION_SURFACE_WINDOW)
        return TYPE_STATUS_INVALID_ARGUMENT;
    session->requested_target = target;
    return TYPE_STATUS_OK;
}

static vm_machine *vm_session_machine(vm_session *session)
{
    return session == STD_NULL ? STD_NULL : session->machine;
}

C_INT vm_session_is_running(const vm_session *session)
{
    return session == STD_NULL || session->machine == STD_NULL ? TYPE_FALSE :
        vm_machine_is_running(session->machine);
}

C_VOID vm_session_print_machine(const vm_session *session)
{
    if (session != STD_NULL && session->machine != STD_NULL)
        vm_machine_print_machine(session->machine);
}

C_VOID vm_session_print_bios(const vm_session *session)
{
    if (session != STD_NULL && session->machine != STD_NULL)
        vm_machine_print_bios(session->machine);
}

C_VOID vm_session_print_status(const vm_session *session)
{
    if (session != STD_NULL && session->machine != STD_NULL)
        vm_machine_print_status(session->machine);
}

type_status vm_session_get_speed(const vm_session *session, vm_session_speed *out_speed)
{
    vm_machine_speed speed;

    if (session == STD_NULL || session->machine == STD_NULL || out_speed == STD_NULL ||
        vm_machine_get_speed(session->machine, &speed) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    *out_speed = speed == VM_MACHINE_SPEED_TURBO ? VM_SESSION_SPEED_TURBO :
        VM_SESSION_SPEED_STANDARD;
    return TYPE_STATUS_OK;
}

type_status vm_session_set_speed(vm_session *session, vm_session_speed speed)
{
    vm_machine *machine = vm_session_machine(session);

    if (machine == STD_NULL || (speed != VM_SESSION_SPEED_STANDARD &&
        speed != VM_SESSION_SPEED_TURBO)) return TYPE_STATUS_INVALID_STATE;
    return vm_machine_set_speed(machine, speed == VM_SESSION_SPEED_TURBO ?
        VM_MACHINE_SPEED_TURBO : VM_MACHINE_SPEED_STANDARD);
}

type_status vm_session_debug(vm_session *session)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_run_debugger(machine);
}

type_status vm_session_record_start(vm_session *session, const C_CHAR *path)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_record_start(machine, path);
}

type_status vm_session_record_stop(vm_session *session)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_record_stop(machine);
}

C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? -1 : vm_machine_insert_fdd(machine, path);
}

C_INT vm_session_remove_fdd(vm_session *session, const C_CHAR *path)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? -1 : vm_machine_remove_fdd(machine, path);
}

type_status vm_session_start(vm_session *session,
    vm_presentation_plan *out_plan)
{
    vm_machine *machine = vm_session_machine(session);
    if (machine == STD_NULL || out_plan == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    vm_session_plan_clear(out_plan);
    vm_session_plan_target(session, out_plan, session->requested_target);
    (C_VOID)vm_session_control_begin_run(session->control);
    return vm_machine_start(machine);
}

type_status vm_session_reset(vm_session *session)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_reset(machine);
}

type_status vm_session_stop(vm_session *session)
{
    vm_machine *machine = vm_session_machine(session);
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    vm_machine_stop(machine);
    return TYPE_STATUS_OK;
}

type_status vm_session_resume(vm_session *session,
    vm_presentation_plan *out_plan)
{
    vm_machine *machine = vm_session_machine(session);
    if (machine == STD_NULL || out_plan == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    vm_session_plan_clear(out_plan);
    vm_session_plan_target(session, out_plan, session->requested_target);
    (C_VOID)vm_session_control_begin_run(session->control);
    return vm_machine_resume(machine);
}

type_status vm_session_request_pause(vm_session *session)
{
    vm_machine *machine = vm_session_machine(session);
    return machine == STD_NULL ? TYPE_STATUS_INVALID_STATE : vm_machine_request_pause(machine);
}

static C_INT vm_session_key_scan(ui_key key, type_unsigned_16 *scan)
{
    static const struct { ui_key key; type_unsigned_16 scan; } map[] = {
        { UI_KEY_ENTER, 0x1cu }, { UI_KEY_BACKSPACE, 0x0eu },
        { UI_KEY_F1, 0x3bu }, { UI_KEY_F2, 0x3cu }, { UI_KEY_F3, 0x3du },
        { UI_KEY_F4, 0x3eu }, { UI_KEY_F5, 0x3fu }, { UI_KEY_F6, 0x40u },
        { UI_KEY_F7, 0x41u }, { UI_KEY_F8, 0x42u }, { UI_KEY_F9, 0x43u },
        { UI_KEY_F10, 0x44u }, { UI_KEY_F11, 0x57u }, { UI_KEY_F12, 0x58u },
        { UI_KEY_UP, 0x48u }, { UI_KEY_DOWN, 0x50u },
        { UI_KEY_LEFT, 0x4bu }, { UI_KEY_RIGHT, 0x4du },
        { UI_KEY_HOME, 0x47u }, { UI_KEY_END, 0x4fu },
        { UI_KEY_PAGE_UP, 0x49u }, { UI_KEY_PAGE_DOWN, 0x51u },
        { UI_KEY_INSERT, 0x52u }, { UI_KEY_DELETE, 0x53u }
    };
    STD_SIZE_T index;

    if (scan == STD_NULL) return TYPE_FALSE;
    for (index = 0u; index < sizeof(map) / sizeof(map[0]); ++index) {
        if (map[index].key == key) { *scan = map[index].scan; return TYPE_TRUE; }
    }
    return TYPE_FALSE;
}

type_status vm_session_submit_ui_input(vm_session *session,
    const ui_input_event *event)
{
    vm_machine_input input = {0};
    type_unsigned_16 scan = 0u;

    if (session == STD_NULL || session->machine == STD_NULL || event == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (event->type == UI_EVENT_KEY) {
        scan = event->data.key.scan_code;
        if (scan == 0u && !vm_session_key_scan(event->data.key.key, &scan))
            return TYPE_STATUS_UNSUPPORTED;
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.scan_code = scan;
        input.data.key_event.virtual_key = (type_unsigned_16)event->data.key.key;
        input.data.key_event.pressed = event->data.key.pressed;
    } else if (event->type == UI_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = VM_MACHINE_INPUT_KEY_EVENT;
        input.data.key_event.virtual_key = (type_unsigned_16)event->data.text.scalar;
        input.data.key_event.pressed = TYPE_TRUE;
    } else if (event->type == UI_EVENT_MOUSE) {
        input.kind = VM_MACHINE_INPUT_MOUSE_EVENT;
        input.data.mouse_event.delta_x = event->data.mouse.delta_x < INT16_MIN ? INT16_MIN :
            event->data.mouse.delta_x > INT16_MAX ? INT16_MAX : event->data.mouse.delta_x;
        input.data.mouse_event.delta_y = event->data.mouse.delta_y < INT16_MIN ? INT16_MIN :
            event->data.mouse.delta_y > INT16_MAX ? INT16_MAX : event->data.mouse.delta_y;
        input.data.mouse_event.buttons = (event->data.mouse.buttons & UI_MOUSE_BUTTON_LEFT ? 1u : 0u) |
            (event->data.mouse.buttons & UI_MOUSE_BUTTON_RIGHT ? 2u : 0u);
    } else return TYPE_STATUS_UNSUPPORTED;
    return vm_machine_submit_input(session->machine, &input);
}

type_status vm_session_submit_key(vm_session *session, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    vm_machine_input input = {0};

    if (session == STD_NULL || session->machine == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    input.kind = VM_MACHINE_INPUT_KEY_EVENT;
    input.data.key_event.scan_code = scan_code;
    input.data.key_event.virtual_key = virtual_key;
    input.data.key_event.pressed = pressed;
    return vm_machine_submit_input(session->machine, &input);
}

static type_status vm_session_submit_ui_input_sink(C_VOID *context,
    const ui_input_event *event)
{
    return vm_session_submit_ui_input((vm_session *)context, event);
}

static C_VOID vm_session_submit_chord(vm_session *session, C_INT cad)
{
    const type_unsigned_16 scan[] = {cad ? 0x1du : 0x38u, 0x38u,
        cad ? 0x153u : 0u};
    const type_unsigned_16 key[] = {cad ? 0x11u : 0x12u, 0x12u,
        cad ? 0x2eu : 0u};
    const type_unsigned_32 count = cad ? 3u : 2u;
    type_unsigned_32 index;

    for (index = 0u; index < count; ++index)
        (C_VOID)vm_session_submit_key(session, scan[index], key[index], TYPE_TRUE);
    for (index = count; index-- != 0u;)
        (C_VOID)vm_session_submit_key(session, scan[index], key[index], TYPE_FALSE);
}

type_status vm_session_reduce_fact(vm_session *session,
    const vm_session_fact *fact, const vm_machine_display_event *display,
    vm_presentation_plan *out_plan)
{
    type_status status;

    if (session == STD_NULL || fact == STD_NULL || out_plan == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    vm_session_plan_clear(out_plan);
    if (fact->kind == VM_SESSION_FACT_DISPLAY) {
        if (display == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
        out_plan->frame_ready = TYPE_TRUE;
        out_plan->frame = *display;
        return TYPE_STATUS_OK;
    }
    if (fact->kind == VM_SESSION_FACT_MACHINE_RESULT) {
        switch (fact->value.machine.kind) {
        case VM_MACHINE_RESULT_RUNNING:
            out_plan->notice = session->lifecycle == VM_MACHINE_RESULT_PAUSED ?
                VM_PRESENTATION_NOTICE_RESUMED : VM_PRESENTATION_NOTICE_STARTED;
            vm_session_plan_title(out_plan, "NXVM (Running)");
            out_plan->mouse_capturable_changed = TYPE_TRUE;
            out_plan->mouse_capturable = TYPE_TRUE;
            break;
        case VM_MACHINE_RESULT_PAUSED:
            out_plan->notice = VM_PRESENTATION_NOTICE_PAUSED;
            vm_session_plan_title(out_plan, "NXVM (Paused)");
            out_plan->mouse_capturable_changed = TYPE_TRUE;
            out_plan->mouse_capturable = TYPE_FALSE;
            out_plan->release_mouse = TYPE_TRUE;
            if (session->active_target == VM_PRESENTATION_SURFACE_CONSOLE)
                vm_session_plan_target(session, out_plan, VM_PRESENTATION_SURFACE_NONE);
            break;
        case VM_MACHINE_RESULT_RESET:
            out_plan->notice = VM_PRESENTATION_NOTICE_RESET;
            break;
        case VM_MACHINE_RESULT_STOPPED:
            out_plan->notice = VM_PRESENTATION_NOTICE_STOPPED;
            out_plan->mouse_capturable_changed = TYPE_TRUE;
            out_plan->mouse_capturable = TYPE_FALSE;
            out_plan->release_mouse = TYPE_TRUE;
            vm_session_plan_target(session, out_plan, VM_PRESENTATION_SURFACE_NONE);
            break;
        default:
            break;
        }
        session->lifecycle = fact->value.machine.kind;
        return TYPE_STATUS_OK;
    }
    if (fact->kind != VM_SESSION_FACT_PRESENTATION_INPUT) return TYPE_STATUS_OK;
    if (fact->value.presentation_input.type == UI_EVENT_HOTKEY) {
        const C_CHAR *name = fact->value.presentation_input.data.hotkey.identifier;

        if (!STD_STRCMP(name, "pause")) {
            if (vm_session_is_running(session)) return vm_session_request_pause(session);
            return vm_session_resume(session, out_plan);
        }
        if (!STD_STRCMP(name, "release-mouse")) {
            out_plan->release_mouse = TYPE_TRUE;
            return TYPE_STATUS_OK;
        }
        if (!STD_STRCMP(name, "cad") || !STD_STRCMP(name, "alt-enter")) {
            vm_session_submit_chord(session, !STD_STRCMP(name, "cad"));
            return TYPE_STATUS_OK;
        }
        return TYPE_STATUS_OK;
    }
    if (fact->value.presentation_input.type == UI_EVENT_WINDOW_CLOSE) {
        if (vm_session_is_running(session)) (C_VOID)vm_session_request_pause(session);
        vm_session_plan_target(session, out_plan, VM_PRESENTATION_SURFACE_NONE);
        out_plan->mouse_capturable_changed = TYPE_TRUE;
        out_plan->mouse_capturable = TYPE_FALSE;
        out_plan->release_mouse = TYPE_TRUE;
        return TYPE_STATUS_OK;
    }
    status = vm_session_control_dispatch_host_input(session->control,
        &fact->value.presentation_input, vm_session_is_running(session),
        vm_session_submit_ui_input_sink, session);
    return status;
}
