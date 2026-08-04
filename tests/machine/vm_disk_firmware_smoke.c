#include "type.h"




#include "vm/composition/session/session.h"

#include "vm/profile/default_profile/firmware/qddisk.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    C_VOID (*read_handler)(vm_profile_default_context *) =
        vm_profile_default_disk_handle_hdd_read;
    C_VOID (*write_handler)(vm_profile_default_context *) =
        vm_profile_default_disk_handle_hdd_write;
    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    if (read_handler == STD_NULL || write_handler == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T42:S1:DISK-FIRMWARE:OK");
    return 0;
}
