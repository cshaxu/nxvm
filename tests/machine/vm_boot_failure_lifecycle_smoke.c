#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/profile/default_profile/firmware/bios.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    type_unsigned_8 report = VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine,
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_control_start(&session->control);
    if (vm_session_control_is_running(&session->control) ||
        core_machine_memory_read(session->core_machine,
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK ||
        report != VBIOS_POST_REPORT_NONE) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    STD_PRINTF("M5:T211:S3:BOOT-FAILURE-LIFECYCLE:OK\n");
    return 0;
}
