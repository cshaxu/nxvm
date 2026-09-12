#include "type.h"
#include "vm/machine/runtime/machine_private.h"

#include <windows.h>





#include "vm/machine/runtime/control.h"

#include "test/integration/support/session_yaml.h"

static C_INT wait_for_running(const vm_machine *machine)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (vm_machine_is_running(machine)) return 1;
        Sleep(1u);
    }
    return vm_machine_is_running(machine);
}

static C_INT wait_for_pause_reason(const vm_machine *machine,
    vm_machine_pause_reason reason)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (machine != STD_NULL && vm_machine_control_is_paused(&machine->control) &&
            vm_machine_control_get_pause_reason(&machine->control) == reason)
            return 1;
        Sleep(1u);
    }
    return machine != STD_NULL && vm_machine_control_is_paused(&machine->control) &&
        vm_machine_control_get_pause_reason(&machine->control) == reason;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    vm_machine *session;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    if (vm_machine_start(session) != TYPE_STATUS_OK || !wait_for_running(session) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK) goto fail;
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
            VM_MACHINE_PAUSE_EXPLICIT || !vm_machine_control_is_paused(
            &session->control)) goto fail;

    if (vm_machine_request_step(session) != TYPE_STATUS_OK ||
        !wait_for_pause_reason(session, VM_MACHINE_PAUSE_STEP))
        goto fail;

    if (vm_machine_resume(session) != TYPE_STATUS_OK || !wait_for_running(session) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK) goto fail;
    if (!vm_machine_control_wait_for_pause(&session->control, 2000u) ||
        vm_machine_control_get_pause_reason(&session->control) !=
            VM_MACHINE_PAUSE_EXPLICIT) goto fail;
    vm_machine_stop(session);
    integration_yaml_session_close(&yaml_session);
    puts("M5:T45:S1:PAUSE-BOUNDARY:OK");
    return 0;

fail:
    if (session != STD_NULL) vm_machine_stop(session);
    integration_yaml_session_close(&yaml_session);
    return 1;
}
