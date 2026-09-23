#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/media/fdd.h"
#include "app-nxvm/machine/media/hdd.h"
#include "test/app-nxvm/integration/support/session_ini.h"

static C_INT verify(const C_CHAR *directory, const C_CHAR *file_name)
{
    integration_ini_session ini_session;
    vm_machine_reset_vector vector;
    vm_machine *session;

    if (integration_ini_session_open(directory, file_name, &ini_session) != TYPE_STATUS_OK) {
        return -1;
    }
    session = ini_session.session;
    if (
        vm_machine_control_is_running(&session->control)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    vm_machine_reset(session);
    if (vm_machine_fdd_remove_for(&session->fdd) ||
        vm_machine_hdd_remove(&session->hdd) ||
        vm_machine_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
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
