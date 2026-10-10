#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/machine/machine_private.h"
#include "core/machine/lifecycle.h"
#include "core/machine/media/fdd_interface.h"
#include "core/machine/media/hdd_interface.h"
#include "test/core/setup/session_ini.h"

static lib_i32 verify(const char *directory, const char *file_name)
{
    integration_ini_session ini_session;
    vm_machine_reset_vector vector;
    vm_machine *session;

    if (integration_ini_session_open(directory, file_name, &ini_session) != LIB_STATUS_OK) {
        return -1;
    }
    session = ini_session.session;
    if (
        vm_machine_control_is_running(&session->control)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    vm_machine_reset(session);
    if (vm_machine_fdd_remove_for(session->floppy[0u]) ||
        vm_machine_hdd_remove(session->fixed_disk[0u]) ||
        vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
    return 0;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    lib_i32 floppy_result;
    lib_i32 fixed_disk_result;

    if (argc != 4) return 1;
    floppy_result = verify(argv[1], argv[2]);
    fixed_disk_result = verify(argv[1], argv[3]);
    if (floppy_result < 0 || fixed_disk_result < 0) return 77;
    if (floppy_result != 0 || fixed_disk_result != 0) return 1;
    puts("NXVM:NXVM-PC-AT:OK");
    return 0;
}
