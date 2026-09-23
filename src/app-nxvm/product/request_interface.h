#ifndef VM_REQUEST_INTERFACE_H
#define VM_REQUEST_INTERFACE_H

#include "type.h"
#include "lib/storage/medium_interface.h"

/* Immutable copied runtime request.  The executable's generated Profile
 * binding supplies all board, CPU and firmware facts; NXVM.ini owns only the
 * user-changeable memory, media and presentation values below. */
#define VM_SESSION_REQUEST_PATH_MAX 1024u
#define VM_SESSION_REQUEST_MEDIA_SLOT_COUNT 2u

typedef struct vm_session_request {
    C_CHAR file_name[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR display[16];
    C_INT console_control;
    C_CHAR floppy[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR fixed_disk[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    lib_storage_medium_mode floppy_mode[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT];
    STD_SIZE_T floppy_count;
    STD_SIZE_T fixed_disk_count;
    STD_SIZE_T memory_bytes;
} vm_session_request;

#endif
