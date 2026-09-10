/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/machine_info.h"
#include "vm/composition/session/session_private.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/fault.h"

#define VM_SESSION_MACHINE_NAME "IBM PC/AT"

C_VOID vm_session_print_machine(const vm_session *session)
{
    core_machine_cpu_profile cpu_profile;
    STD_SIZE_T memory_bytes = 0u;

    if (session == STD_NULL) return;
    if (core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || core_machine_get_cpu_profile(session->core_machine,
            &cpu_profile) != TYPE_STATUS_OK) return;
    STD_PRINTF("Machine:           %s\n", VM_SESSION_MACHINE_NAME);
    STD_PRINTF("Profile:           %s\n",
        vm_session_profile_name(session->retained_config.profile_kind));
    STD_PRINTF("CPU:               Intel %s\n",
        core_machine_cpu_profile_name(cpu_profile));
    if (memory_bytes < (1u << 20)) {
        STD_PRINTF("RAM Size:          %u KB\n",
            (unsigned int)(memory_bytes >> 10));
    } else {
        STD_PRINTF("RAM Size:          %u MB\n",
            (unsigned int)(memory_bytes >> 20));
    }
    STD_PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", VM_MACHINE_DEVICE_FDD,
        vm_machine_fdd_image_size(&session->fdd) * 1. / VFDD_BYTE_PER_MB,
        vm_machine_fdd_has_media(&session->fdd) ? "inserted" : "not inserted");
    if (session->profile != STD_NULL && session->profile->hdc_present) {
        STD_PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
            vm_machine_hdd_cylinders(&session->hdd),
            vm_machine_hdd_image_size(&session->hdd) * 1. / VHDD_BYTE_PER_MB,
            vm_machine_hdd_has_media(&session->hdd) ? "connected" : "disconnected");
    }
}

C_VOID vm_session_print_bios(const vm_session *session)
{
    if (session == STD_NULL) return;
    STD_PRINTF("BIOS: %s\n", session->firmware_kind ==
        VM_SESSION_FIRMWARE_EXTERNAL_PC_AT_ROM && session->pc_at_rom_external ?
        "external ROM mapped at F0000h" : "profile ROM mapped");
}

C_VOID vm_session_print_status(const vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_control_print_status(&session->control);
    vm_session_fault_print(session);
}
