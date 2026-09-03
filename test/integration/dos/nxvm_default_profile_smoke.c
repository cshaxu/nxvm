#include "type.h"

#include "core/machine/memory.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "test/integration/support/session_yaml.h"

static C_INT verify(const C_CHAR *directory, const C_CHAR *file_name)
{
    integration_yaml_session yaml_session;
    vm_session_reset_vector vector;
    vm_session *session;

    if (integration_yaml_session_open(directory, file_name, &yaml_session) != TYPE_STATUS_OK) {
        return -1;
    }
    session = yaml_session.session;
    if (
        vm_session_control_is_running(&session->control)) {
        integration_yaml_session_close(&yaml_session);
        return 1;
    }
    vm_session_reset(session);
    if (vm_machine_fdd_remove_for(&session->fdd, STD_NULL) ||
        vm_machine_hdd_remove(&session->hdd, STD_NULL) ||
        vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_yaml_session_close(&yaml_session);
        return 1;
    }
    vm_platform_run_context_set_window_display(session->platform_run_context, 0);
    integration_yaml_session_close(&yaml_session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    C_INT floppy_result;
    C_INT fixed_disk_result;

    if (argc != 4) return 1;
    floppy_result = verify(argv[1], argv[2]);
    fixed_disk_result = verify(argv[1], argv[3]);
    if (floppy_result < 0 || fixed_disk_result < 0) return 77;
    if (floppy_result != 0 || fixed_disk_result != 0) return 1;
    puts("M5:T5:S2:NXVM-PC-AT:OK");
    return 0;
}
