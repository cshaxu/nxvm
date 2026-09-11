#include "type.h"

#include "vm/session/control.h"
#include "vm/session/session.h"

int main(C_VOID)
{
    vm_session_control *control = STD_NULL;
    vm_session_fact received = {0};
    vm_machine_result result = {0};
    vm_machine_display_event received_display = {0};
    ui_input_event input = {0};
    vm_session *session = STD_NULL;
    vm_presentation_plan plan = {0};

    if (vm_session_control_create(&control) != TYPE_STATUS_OK) return 1;
    if (vm_session_create(&session) != TYPE_STATUS_OK) {
        vm_session_control_destroy(control);
        return 1;
    }
    result.kind = VM_MACHINE_RESULT_DISPLAY;
    result.value.display.generation = 7u;
    if (vm_session_control_publish_machine_result(control, &result) != TYPE_STATUS_OK) {
        vm_session_control_destroy(control);
        return 1;
    }
    result = (vm_machine_result){0};
    result.kind = VM_MACHINE_RESULT_RUNNING;
    if (vm_session_control_publish_machine_result(control, &result) != TYPE_STATUS_OK ||
        vm_session_control_take(control, &received, &received_display, 0u) !=
            TYPE_STATUS_OK ||
        received.kind != VM_SESSION_FACT_MACHINE_RESULT ||
        received.value.machine.kind != VM_MACHINE_RESULT_RUNNING ||
        STD_STRCMP(vm_session_control_note_machine_result(control,
            received.value.machine.kind), "started") != 0 ||
        vm_session_control_take(control, &received, &received_display, 0u) !=
            TYPE_STATUS_OK || received.kind != VM_SESSION_FACT_DISPLAY ||
        received_display.generation != 7u) {
        vm_session_control_destroy(control);
        return 1;
    }
    result = (vm_machine_result){0};
    result.kind = VM_MACHINE_RESULT_DISPLAY;
    result.value.display.generation = 8u;
    if (vm_session_control_publish_machine_result(control, &result) != TYPE_STATUS_OK) {
        vm_session_control_destroy(control);
        return 1;
    }
    result.value.display.generation = 9u;
    if (vm_session_control_publish_machine_result(control, &result) != TYPE_STATUS_OK ||
        vm_session_control_take(control, &received, &received_display, 0u) !=
            TYPE_STATUS_OK || received.kind != VM_SESSION_FACT_DISPLAY ||
        received_display.generation != 9u) {
        vm_session_control_destroy(control);
        return 1;
    }
    input.type = UI_EVENT_KEY;
    input.data.key.scan_code = 0x3bu;
    if (vm_session_control_publish_presentation_input(control, &input) != TYPE_STATUS_OK ||
        vm_session_control_begin_run(control) == 0u ||
        vm_session_control_take(control, &received, &received_display, 0u) ==
            TYPE_STATUS_OK ||
        vm_session_control_publish_presentation_input(control, &input) != TYPE_STATUS_OK ||
        vm_session_control_take(control, &received, &received_display, 0u) !=
            TYPE_STATUS_OK || received.kind != VM_SESSION_FACT_PRESENTATION_INPUT ||
        received.value.presentation_input.data.key.scan_code != 0x3bu) {
        vm_session_control_destroy(control);
        return 1;
    }
    vm_session_control_close(control);
    if (vm_session_control_publish_presentation_input(control, &input) == TYPE_STATUS_OK) {
        vm_session_control_destroy(control);
        vm_session_destroy(session);
        return 1;
    }
    received = (vm_session_fact){0};
    received.kind = VM_SESSION_FACT_DISPLAY;
    received_display.generation = 10u;
    if (vm_session_reduce_fact(session, &received, &received_display, &plan) !=
            TYPE_STATUS_OK || !plan.frame_ready || plan.frame.generation != 10u ||
        vm_session_set_presentation_target(session, VM_PRESENTATION_SURFACE_WINDOW) !=
            TYPE_STATUS_OK) {
        vm_session_control_destroy(control);
        vm_session_destroy(session);
        return 1;
    }
    received = (vm_session_fact){0};
    received.kind = VM_SESSION_FACT_MACHINE_RESULT;
    received.value.machine.kind = VM_MACHINE_RESULT_PAUSED;
    if (vm_session_reduce_fact(session, &received, &received_display, &plan) !=
            TYPE_STATUS_OK || plan.notice != VM_PRESENTATION_NOTICE_PAUSED ||
        !plan.title_changed || STD_STRCMP(plan.title, "NXVM (Paused)") != 0 ||
        !plan.mouse_capturable_changed || plan.mouse_capturable) {
        vm_session_control_destroy(control);
        vm_session_destroy(session);
        return 1;
    }
    received = (vm_session_fact){0};
    received.kind = VM_SESSION_FACT_PRESENTATION_INPUT;
    received.value.presentation_input.type = UI_EVENT_HOTKEY;
    (C_VOID)snprintf(received.value.presentation_input.data.hotkey.identifier,
        sizeof(received.value.presentation_input.data.hotkey.identifier), "%s",
        "release-mouse");
    if (vm_session_reduce_fact(session, &received, &received_display, &plan) !=
            TYPE_STATUS_OK || !plan.release_mouse) {
        vm_session_control_destroy(control);
        vm_session_destroy(session);
        return 1;
    }
    vm_session_control_destroy(control);
    vm_session_destroy(session);
    return 0;
}
