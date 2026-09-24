#ifndef VM_REQUEST_INTERFACE_H
#define VM_REQUEST_INTERFACE_H
#include "lib/types/types_interface.h"

#include "lib/storage/medium_interface.h"

/* Immutable copied runtime request.  The executable's generated Profile
 * binding supplies all board, CPU and firmware facts; NXVM.ini owns only the
 * user-changeable memory, media and presentation values below. */
#define VM_SESSION_REQUEST_PATH_MAX 1024u
#define VM_SESSION_REQUEST_MEDIA_SLOT_COUNT 2u

typedef struct vm_session_request {
    lib_u8 file_name[VM_SESSION_REQUEST_PATH_MAX];
    lib_u8 display[16];
    lib_i32 console_control;
    lib_u8 floppy[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    lib_u8 fixed_disk[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    lib_storage_medium_mode floppy_mode[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT];
    lib_size floppy_count;
    lib_size fixed_disk_count;
    lib_size memory_bytes;
} vm_session_request;

#endif
