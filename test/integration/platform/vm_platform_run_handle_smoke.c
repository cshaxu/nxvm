#include "type.h"

#include "core/machine/machine.h"
#include "lib/host/sync_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/platform/platform.h"
#include "vm/platform/platform.h"
#include "test/integration/support/session_yaml.h"

static C_INT vm_platform_run_handle_wait_for_keyboard(vm_session *session,
    type_unsigned_8 previous)
{
    C_UINT waited;

    for (waited = 0u; waited != 1000u; ++waited) {
        if (session->core_machine->shared_kbc.data.keyboard_has_output &&
            session->core_machine->shared_kbc.data.last_keyboard_output_byte != previous) {
            return 1;
        }
        host_sync_sleep_milliseconds(1u);
    }
    return 0;
}

int main(C_INT argc, C_CHAR **argv)
{
    vm_session *session = STD_NULL;
    core_machine_guest_input_event input = { 0 };
    vm_platform_run_handle *event_handle = STD_NULL;
    integration_yaml_session yaml_session;

    STD_MEMSET(&yaml_session, 0, sizeof(yaml_session));

    if (vm_platform_run_handle_create(&event_handle) != TYPE_STATUS_OK) return 1;
    vm_platform_run_handle_report(event_handle,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    if (vm_platform_run_handle_get_last_event(event_handle) !=
            VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED ||
        vm_platform_run_handle_take_stop_report(event_handle)) goto event_fail;
    vm_platform_run_handle_report(event_handle,
        VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    if (!vm_platform_run_handle_take_stop_report(event_handle) ||
        vm_platform_run_handle_take_stop_report(event_handle)) goto event_fail;
    vm_platform_run_handle_destroy(event_handle);

    if (argc != 2 || integration_yaml_session_open(argv[1],
            "default-pc-at-80386-1440k.yaml", &yaml_session) != TYPE_STATUS_OK ||
        (session = yaml_session.session) == STD_NULL || !session->active) goto fail;
    vm_platform_run_context_set_window_display(session->platform_run_context, 1);
    if (vm_session_start(session) != TYPE_STATUS_OK ||
        !session->start_outcome.valid ||
        session->start_outcome.status != TYPE_STATUS_OK) goto fail;
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    host_sync_sleep_milliseconds(50u);
    input.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    input.data.key.scan_code = 0x43u;
    input.data.key.virtual_key = 0x78u;
    input.data.key.pressed = TYPE_TRUE;
    if (vm_session_submit_host_input(session, &input) != TYPE_STATUS_OK) goto fail;
    host_sync_sleep_milliseconds(50u);
    if (!vm_session_control_is_running(&session->control) ||
        vm_platform_run_handle_take_stop_report(session->platform_run_handle)) goto fail;
    vm_session_stop(session);
    vm_session_reset(session);
    if (session->start_outcome.valid) goto fail;
    if (vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    if (vm_session_start(session) != TYPE_STATUS_OK ||
        !session->start_outcome.valid ||
        session->start_outcome.status != TYPE_STATUS_OK) goto fail;
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    host_sync_sleep_milliseconds(50u);
    if (session->core_machine->shared_kbc.data.last_keyboard_output_byte == 0x43u) {
        goto fail;
    }
    {
        type_unsigned_8 previous =
            session->core_machine->shared_kbc.data.last_keyboard_output_byte;

        input.data.key.scan_code = 0x1eu;
        input.data.key.virtual_key = 'A';
        input.data.key.pressed = TYPE_TRUE;
        if (vm_session_submit_host_input(session, &input) != TYPE_STATUS_OK) goto fail;
        if (!vm_platform_run_handle_wait_for_keyboard(session, previous)) goto fail;
    }
    vm_session_stop(session);
    if (vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    integration_yaml_session_close(&yaml_session);
    puts("M5:T194:S3:RUN-EVENT:OK");
    return 0;

event_fail:
    vm_platform_run_handle_destroy(event_handle);
    return 1;

fail:
    integration_yaml_session_close(&yaml_session);
    return 1;
}
