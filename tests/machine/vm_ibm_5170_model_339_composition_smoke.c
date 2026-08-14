#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"
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
    vm_session *session = STD_NULL;
    STD_SIZE_T memory_bytes = 0u;
    C_INT failed = 0;

    if (profile == STD_NULL ||
        !vm_profile_default_pc_at_descriptor_is_valid(profile) ||
        profile->hdc_present || !profile->planar_parity_present ||
        vm_profile_has_hdc_firmware(profile) ||
        vm_profile_default_pc_at_port_leaf_find(profile,
            VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f0u) != STD_NULL ||
        vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    failed |= session->profile != profile ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            TYPE_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            TYPE_STATUS_OK || memory_bytes != 512u * 1024u ||
        core_machine_get_planar_parity_observation(session->core_machine,
            &parity) != TYPE_STATUS_OK || !parity.configured || !parity.enabled ||
        core_machine_port_has_read(&session->core_machine->executor_port, 0x01f0u) ||
        core_machine_port_has_write(&session->core_machine->executor_port, 0x01f0u) ||
        vm_session_insert_hdd(session, "unavailable.img") == 0;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_model_339_rejects_hdd_configuration(C_VOID)
{
    const vm_session_config create_config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .create_hdd_cylinders = 1u
    };
    const vm_session_config boot_config = {
        .profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339,
        .boot_hdd = 1
    };
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(&create_config, &session) !=
        TYPE_STATUS_INVALID_ARGUMENT || session != STD_NULL;

    vm_session_destroy(session);
    session = STD_NULL;
    failed |= vm_session_create(&boot_config, &session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session != STD_NULL;
    vm_session_destroy(session);
    return failed;
}

static C_INT vm_default_profile_remains_ata(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL;

    if (!failed) failed |= session->profile != vm_profile_default_pc_at_descriptor_get() ||
        !session->profile->hdc_present ||
        !core_machine_port_has_read(&session->core_machine->executor_port, 0x01f0u) ||
        !core_machine_port_has_write(&session->core_machine->executor_port, 0x01f0u);
    vm_session_destroy(session);
    return failed;
}

C_INT main(C_VOID)
{
    if (vm_model_339_selected_contract() ||
        vm_model_339_rejects_hdd_configuration() ||
        vm_default_profile_remains_ata()) return 1;
    STD_PRINTF("M5:T366:S5:MODEL339-COMPOSITION:OK\n");
    return 0;
}
