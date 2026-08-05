/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/machine_info.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

#define VM_SESSION_MACHINE_NAME "IBM PC/AT"

C_VOID vm_session_print_machine(const vm_session *session)
{
    STD_SIZE_T memory_bytes = 0u;

    if (session == STD_NULL) return;
    if (core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
        TYPE_STATUS_OK) return;
    STD_PRINTF("Machine:           %s\n", VM_SESSION_MACHINE_NAME);
    STD_PRINTF("CPU:               %s\n", CORE_MACHINE_DEVICE_CPU);
    STD_PRINTF("RAM Size:          %d MB\n",
        memory_bytes >> 20);
    STD_PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", VM_MACHINE_DEVICE_FDD,
        vm_machine_fdd_image_size(&session->fdd) * 1. / VFDD_BYTE_PER_MB,
        session->fdd.connect.flagDiskExist ? "inserted" : "not inserted");
    STD_PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
        session->hdd.data.ncyl,
        vm_machine_hdd_image_size(&session->hdd) * 1. / VHDD_BYTE_PER_MB,
        session->hdd.connect.flagDiskExist ? "connected" : "disconnected");
}
