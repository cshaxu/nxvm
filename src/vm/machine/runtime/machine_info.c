/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/fault.h"

type_status vm_machine_get_information(const vm_machine *session,
    vm_machine_information *out_information)
{
    core_machine_cpu_profile cpu_profile;
    const vm_machine_fault_outcome *fault;
    STD_SIZE_T memory_bytes = 0u;

    if (session == STD_NULL || out_information == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || core_machine_get_cpu_profile(session->core_machine,
            &cpu_profile) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    STD_MEMSET(out_information, 0, sizeof(*out_information));
    out_information->profile_kind = session->retained_config.profile_kind;
    out_information->cpu_profile = cpu_profile;
    out_information->memory_bytes = memory_bytes;
    out_information->floppy_image_bytes = vm_machine_fdd_image_size(&session->fdd);
    out_information->floppy_media_inserted = vm_machine_fdd_has_media(&session->fdd);
    out_information->fixed_disk_present = session->profile != STD_NULL &&
        session->profile->hdc_present;
    out_information->fixed_disk_cylinders = vm_machine_hdd_cylinders(&session->hdd);
    out_information->fixed_disk_image_bytes = vm_machine_hdd_image_size(&session->hdd);
    out_information->fixed_disk_media_connected = vm_machine_hdd_has_media(&session->hdd);
    out_information->external_firmware = session->firmware_kind ==
        VM_MACHINE_FIRMWARE_EXTERNAL_PC_AT_ROM && session->pc_at_rom_external;
    out_information->active = vm_machine_executor_state_is_active(
        session->control.state);
    fault = &session->fault_outcome;
    out_information->fault_valid = fault->valid;
    if (fault->valid) {
        out_information->fault_detail = fault->run.detail;
        out_information->fault_linear_pc = fault->run.linear_pc;
        out_information->fault_exception_valid = fault->diagnostic.first_fault.valid;
        out_information->fault_exception_mask = fault->diagnostic.first_fault.exception_mask;
        out_information->fault_exception_code = fault->diagnostic.first_fault.exception_code;
        out_information->fault_exception_cs = fault->diagnostic.first_fault.point.cs;
        out_information->fault_exception_eip = fault->diagnostic.first_fault.point.eip;
    }
    return TYPE_STATUS_OK;
}
