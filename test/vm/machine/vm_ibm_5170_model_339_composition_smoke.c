#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/profile/default_profile/pc_at_profile.h"

static C_INT vm_profile_has_hdc_firmware(
    const vm_profile_default_pc_at_descriptor *profile)
{
    STD_SIZE_T index;

    if (profile == STD_NULL) return 1;
    for (index = 0u; index < profile->firmware_service_count; ++index) {
        if (profile->firmware_services[index].hook ==
            VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13) return 1;
    }
    return 0;
}

static C_INT vm_model_339_selected_contract(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    const vm_session_config config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    core_machine_cpu_profile cpu_profile;
    core_machine_planar_parity_observation parity;
    core_machine_speaker_observation speaker;
    core_machine_memory_route memory_route;
    vm_session *session = STD_NULL;
    STD_SIZE_T memory_bytes = 0u;
    type_unsigned_8 before = 0u;
    type_unsigned_8 after = 0u;
    C_INT failed = 0;

    if (profile == STD_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        !profile->hdc_present || !profile->planar_parity_present ||
        !profile->monochrome_aperture_absent ||
        profile->firmware_slot !=
            VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT ||
        profile->diskette_drive_a_field_upgrade ||
        profile->cmos.floppy_type != 0x20u || profile->cmos.fixed_disk_type != 0x30u ||
        !vm_profile_has_hdc_firmware(profile) ||
        vm_profile_default_pc_at_port_leaf_find(profile,
            VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f0u) == STD_NULL ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_reset(session);
    failed |= session->profile == profile ||
        STD_STRCMP(session->profile->identity, "pc-at-5170") != 0 ||
        !vm_profile_default_pc_at_descriptor_is_valid(session->profile);
    /* 5170 selects no board-specific external-cycle policy; it reuses the
     * same Core owner with the disabled default rather than a parallel path. */
    failed |= session->core_machine->transaction_contract.cpu_cycle_bus_ready_gate_enabled != TYPE_FALSE ||
        session->core_machine->transaction_contract.cpu_prefetch_reservation_enabled != TYPE_FALSE ||
        session->core_machine->transaction_contract.external_cycle_timing.page_bytes != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_miss_ticks != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.page_hit_ticks != 0u ||
        session->core_machine->transaction_contract.external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED;
    failed |= core_machine_bus_write(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
        0x02u) != TYPE_STATUS_OK || core_machine_get_speaker_observation(
        session->core_machine, &speaker) != TYPE_STATUS_OK || !speaker.configured ||
        speaker.timer_gate || !speaker.data_enabled || !speaker.output ||
        core_machine_bus_write(session->core_machine, CORE_MACHINE_PC_AT_PORT_B,
        0x04u) != TYPE_STATUS_OK;
    failed |= core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
        TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80286;
    failed |= session->core_machine->shared_kbc.connect.aux_present ||
        session->core_machine->shared_kbc.data.aux_enabled ||
        (session->core_machine->shared_kbc.data.command_byte &
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u;
    failed |= core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
        TYPE_STATUS_OK || memory_bytes != 512u * 1024u;
    failed |= core_machine_get_planar_parity_observation(session->core_machine,
        &parity) != TYPE_STATUS_OK || !parity.configured || !parity.enabled;
    failed |= core_machine_set_a20(session->core_machine, 1) != TYPE_STATUS_OK;
    failed |= core_machine_memory_query(session->core_machine, 0x00100003u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &memory_route) != TYPE_STATUS_OK ||
        memory_route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    failed |= core_machine_memory_read(session->core_machine, 0x00100003u, &before,
        sizeof(before)) != TYPE_STATUS_OK || before != 0xffu;
    failed |= core_machine_memory_write(session->core_machine, 0x00100003u,
        &(type_unsigned_8){0x5au}, sizeof(type_unsigned_8)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine, 0x00100003u, &after,
        sizeof(after)) != TYPE_STATUS_OK || after != 0xffu;
    failed |= core_machine_memory_query(session->core_machine, 0x00080e00u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &memory_route) != TYPE_STATUS_OK ||
        memory_route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        core_machine_memory_read(session->core_machine, 0x00080e00u, &before,
            sizeof(before)) != TYPE_STATUS_OK || before != 0xffu ||
        core_machine_memory_write(session->core_machine, 0x00080e00u,
            &(type_unsigned_8){0x5au}, sizeof(type_unsigned_8)) != TYPE_STATUS_OK ||
        core_machine_memory_read(session->core_machine, 0x00080e00u, &after,
            sizeof(after)) != TYPE_STATUS_OK || after != 0xffu;
    failed |= core_machine_memory_query(session->core_machine, 0x000b0000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &memory_route) != TYPE_STATUS_OK ||
        memory_route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        core_machine_memory_read(session->core_machine, 0x000b0000u, &before,
            sizeof(before)) != TYPE_STATUS_OK || before != 0xffu ||
        core_machine_memory_write(session->core_machine, 0x000b0000u,
            &(type_unsigned_8){0x5au}, sizeof(type_unsigned_8)) != TYPE_STATUS_OK ||
        core_machine_memory_read(session->core_machine, 0x000b0000u, &after,
            sizeof(after)) != TYPE_STATUS_OK || after != 0xffu;
    failed |= !core_machine_port_has_read(&session->core_machine->executor_port, 0x01f0u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x01f0u) ||
        vm_session_insert_hdd(session, "unavailable.img") == 0;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_model_339_floppy_contract(C_VOID)
{
    const vm_session_config native = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339
    };
    const vm_session_config compatible = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .floppy_format = VM_SESSION_FLOPPY_FORMAT_360K
    };
    const vm_session_config rejected_720 = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .floppy_format = VM_SESSION_FLOPPY_FORMAT_720K
    };
    const vm_session_config rejected_1440 = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .floppy_format = VM_SESSION_FLOPPY_FORMAT_1440K
    };
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(&native, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->floppy_kind != VM_PROFILE_FLOPPY_525_1200K ||
        session->fdd.data.ncyl != 80u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 15u;

    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_session_create(&compatible, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || session->floppy_kind != VM_PROFILE_FLOPPY_525_360K ||
        session->fdd.data.ncyl != 40u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 9u || session->profile->cmos.floppy_type != 0x20u;
    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_session_create(&rejected_720, &session) == TYPE_STATUS_OK || session != STD_NULL;
    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_session_create(&rejected_1440, &session) == TYPE_STATUS_OK || session != STD_NULL;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_model_339_accepts_hdd_boot(C_VOID)
{
    const vm_session_config boot_config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .boot_hdd = 1
    };
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(&boot_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_default_profile_remains_ata(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;

    if (!failed) failed |= session->profile == vm_profile_default_pc_at_descriptor_get() ||
        STD_STRCMP(session->profile->identity, "default-at") != 0 ||
        !session->profile->hdc_present ||
        !session->core_machine->shared_kbc.connect.aux_present ||
        !core_machine_port_has_read(&session->core_machine->executor_port, 0x01f0u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x01f0u);
    vm_session_destroy(session);
    return failed;
}

C_INT main(C_VOID)
{
    if (vm_model_339_selected_contract() || vm_model_339_floppy_contract() ||
        vm_model_339_accepts_hdd_boot() ||
        vm_default_profile_remains_ata()) return 1;
    STD_PRINTF("M5:T366:S5:MODEL339-COMPOSITION:OK\n");
    STD_PRINTF("M5:T380:S2:MODEL339-NO-XMS-PROBE:OK\n");
    STD_PRINTF("M5:T421:S1:IBM5170-SHARED-SPEAKER:OK\n");
    STD_PRINTF("M5:T476:S3:IBM5170-ROOT-CUTOVER:OK\n");
    STD_PRINTF("M5:T478:S3:DEFAULT-AT-SESSION-CUTOVER:OK\n");
    STD_PRINTF("M5:T479:S5:IBM5170-PROFILE:OK\n");
    STD_PRINTF("M5:T497:S4:IBM5170-FLOPPY:OK\n");
    return 0;
}
