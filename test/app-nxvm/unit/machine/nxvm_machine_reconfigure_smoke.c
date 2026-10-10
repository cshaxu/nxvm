#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "../support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    vm_machine_reset_vector vector;
    vm_machine_information information;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK) return 1;
    failed |= vm_machine_reconfigure_memory(session, 32u * 1024u * 1024u) !=
        LIB_STATUS_OK;
    failed |= vm_machine_get_information(session, &information) !=
        LIB_STATUS_OK || information.memory_bytes != 32u * 1024u * 1024u;
    failed |= vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u;
    vm_machine_destroy(session);
    if (failed) return 1;
    lib_c_printf("SESSION-RECONFIGURE:OK\n");
    return 0;
}
