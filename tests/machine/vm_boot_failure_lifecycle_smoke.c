#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/profile/default_profile/firmware/bios.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    uint8_t report = VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        core_machine_memory_write(vm_session_fixture_machine(session),
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_control_start(vm_session_fixture_control(session));
    if (vm_session_control_is_running(vm_session_fixture_control(session)) ||
        core_machine_memory_read(vm_session_fixture_machine(session),
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK ||
        report != VBIOS_POST_REPORT_NONE) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    STD_PRINTF("M5:T211:S3:BOOT-FAILURE-LIFECYCLE:OK\n");
    return 0;
}
