#ifndef VM_REQUEST_INTERFACE_H
#define VM_REQUEST_INTERFACE_H

#include "type.h"

/* Immutable copied machine-construction value. Product YAML selection resolves
 * this before composition; it carries no Core, executor, presentation or
 * native object. */
#define VM_SESSION_REQUEST_PATH_MAX 1024u
#define VM_SESSION_REQUEST_MEDIA_SLOT_COUNT 2u
#define VM_SESSION_REQUEST_BIOS_SLOT_COUNT 2u

typedef struct vm_session_request {
    C_CHAR file_name[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR profile[64];
    C_CHAR cpu[32];
    C_CHAR fpu[32];
    C_CHAR display[16];
    C_CHAR floppy_format[16];
    C_CHAR floppy[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR fixed_disk[VM_SESSION_REQUEST_MEDIA_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    STD_SIZE_T floppy_count;
    STD_SIZE_T fixed_disk_count;
    C_CHAR bios[VM_SESSION_REQUEST_BIOS_SLOT_COUNT][VM_SESSION_REQUEST_PATH_MAX];
    STD_SIZE_T bios_count;
    C_CHAR video[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR cmos[VM_SESSION_REQUEST_PATH_MAX];
    C_CHAR font[VM_SESSION_REQUEST_PATH_MAX];
    STD_SIZE_T memory_bytes;
} vm_session_request;

#endif
