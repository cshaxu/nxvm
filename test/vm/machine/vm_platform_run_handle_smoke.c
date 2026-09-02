#include "type.h"

#include "core/machine/machine.h"
#include "core/platform/sleep.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/platform/platform.h"
#include "vm/platform/win32/win32.h"

static C_INT vm_platform_run_handle_wait_for_keyboard(vm_session *session,
    type_unsigned_8 previous)
{
    C_UINT waited;

    for (waited = 0u; waited != 1000u; ++waited) {
        if (session->core_machine->shared_kbc.data.keyboard_has_output &&
            session->core_machine->shared_kbc.data.last_keyboard_output_byte != previous) {
            return 1;
        }
        core_platform_sleep_milliseconds(1u);
    }
    return 0;
}

int main(void)
{
    vm_session *session;
    vm_platform_run_handle *event_handle = STD_NULL;

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

    session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    if (session == STD_NULL) return 1;
    STD_STRCPY(session->font_path, "default-cp437-8x16.bin");
    session->retained_config.font_path = session->font_path;
    vm_session_initialize(session);
    if (!session->active) goto fail;
    vm_platform_run_context_set_window_display(session->platform_run_context, 1);
    if (vm_session_start(session) != TYPE_STATUS_OK ||
        !session->start_outcome.valid ||
        session->start_outcome.status != TYPE_STATUS_OK) goto fail;
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    core_platform_sleep_milliseconds(50u);
    vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
        session->platform_run_handle, 0x43u, VK_F9, 1);
    core_platform_sleep_milliseconds(50u);
    if (vm_session_control_is_running(&session->control)) goto fail;
    vm_session_reset(session);
    if (session->start_outcome.valid) goto fail;
    if (vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    if (vm_session_start(session) != TYPE_STATUS_OK ||
        !session->start_outcome.valid ||
        session->start_outcome.status != TYPE_STATUS_OK) goto fail;
    if (!vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    core_platform_sleep_milliseconds(50u);
    if (session->core_machine->shared_kbc.data.last_keyboard_output_byte == 0x43u) {
        goto fail;
    }
    {
        type_unsigned_8 previous =
            session->core_machine->shared_kbc.data.last_keyboard_output_byte;

        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, 0x1eu, 'A', 1);
        if (!vm_platform_run_handle_wait_for_keyboard(session, previous)) goto fail;
    }
    vm_session_stop(session);
    if (vm_platform_run_handle_is_active(session->platform_run_handle)) goto fail;
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T194:S3:RUN-EVENT:OK");
    return 0;

event_fail:
    vm_platform_run_handle_destroy(event_handle);
    return 1;

fail:
    vm_session_stop(session);
    vm_session_finalize(session);
    STD_FREE(session);
    return 1;
}
