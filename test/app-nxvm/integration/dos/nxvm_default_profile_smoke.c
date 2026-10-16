#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "test/x86-nxvm/setup/session_ini.h"

static lib_i32 verify(const char *directory, const char *file_name)
{
    integration_ini_session ini_session;
    x86_nxvm_machine_reset_vector vector;
    x86_nxvm_machine *session;

    if (integration_ini_session_open(directory, file_name, &ini_session) != LIB_STATUS_OK) {
        return -1;
    }
    session = ini_session.session;
    if (
        x86_nxvm_machine_control_is_running(&session->control)) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    x86_nxvm_machine_reset(session);
    if (x86_nxvm_machine_fdd_remove_for(session->floppy[0u]) ||
        x86_nxvm_machine_hdd_remove(session->fixed_disk[0u]) ||
        x86_nxvm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
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
