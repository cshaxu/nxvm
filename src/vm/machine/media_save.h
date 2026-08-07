/* VM-owned, failure-preserving persistence for resident removable media. */

#ifndef VM_MACHINE_MEDIA_SAVE_H
#define VM_MACHINE_MEDIA_SAVE_H

#include "type.h"

C_INT vm_machine_media_save_atomically(const C_CHAR *file_name,
    const C_VOID *bytes, STD_SIZE_T byte_count);

#endif
