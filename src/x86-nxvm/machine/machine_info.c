/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/fault.h"

lib_status x86_nxvm_machine_get_information(const x86_nxvm_machine *session,
    x86_nxvm_machine_information *out_information)
{
    x86_core_cpu_profile cpu_profile;
    const x86_nxvm_machine_fault_outcome *fault;
    lib_size memory_bytes = 0u;
    lib_status status;

    if (session == LIB_NULL || out_information == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (session->x86_core_machine == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    status = x86_core_machine_get_memory_bytes(session->x86_core_machine, &memory_bytes);
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_get_cpu_profile(session->x86_core_machine, &cpu_profile);
    if (status != LIB_STATUS_OK) return status;
    lib_memory_set(out_information, 0, sizeof(*out_information));
    out_information->cpu_profile = cpu_profile;
    out_information->memory_bytes = memory_bytes;
    if (session->construction.floppy_slot_count != 0u) {
        out_information->floppy_image_bytes = x86_nxvm_machine_fdd_image_size(
            session->floppy[0u]);
        out_information->floppy_media_inserted = x86_nxvm_machine_fdd_has_media(
            session->floppy[0u]);
    }
    out_information->fixed_disk_present = session->construction.hdc_present;
    if (session->construction.hdc_present) {
        out_information->fixed_disk_cylinders = x86_nxvm_machine_hdd_cylinders(
            session->fixed_disk[0u]);
        out_information->fixed_disk_image_bytes = x86_nxvm_machine_hdd_image_size(
            session->fixed_disk[0u]);
        out_information->fixed_disk_media_connected = x86_nxvm_machine_hdd_has_media(
            session->fixed_disk[0u]);
    }
    out_information->external_firmware = session->construction.firmware_provider != LIB_NULL;
    out_information->active = x86_nxvm_machine_executor_state_is_active(
        &session->control.state);
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
    return LIB_STATUS_OK;
}
