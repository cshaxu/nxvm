#include "type.h"

#include "vm/composition/composition_session_model.h"




#include "vm/profile/default_profile/profile.h"

static ntvdm64_status vm_composition_session_model_configure_media(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config)
{
    ntvdm64_status status;

    vm_product_media_policy_initialize(&session->media);
    if (config->create_fdd) {
        status = vm_product_media_configure_created(&session->media,
            VM_PRODUCT_BOOT_FDD, 0u);
        if (status != NTVDM64_STATUS_OK) return status;
    } else if (config->fdd_path != STD_NULL) {
        status = vm_product_media_configure(&session->media,
            VM_PRODUCT_BOOT_FDD, config->fdd_path, config->fdd_identity);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    if (config->create_hdd_cylinders != 0u) {
        status = vm_product_media_configure_created(&session->media,
            VM_PRODUCT_BOOT_HDD, config->create_hdd_cylinders);
        if (status != NTVDM64_STATUS_OK) return status;
    } else if (config->hdd_path != STD_NULL) {
        status = vm_product_media_configure(&session->media,
            VM_PRODUCT_BOOT_HDD, config->hdd_path, config->hdd_identity);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    status = vm_product_media_set_boot_target(&session->media,
                                                      config->boot_target);
    return status == NTVDM64_STATUS_OK ?
        vm_product_media_freeze(&session->media) : status;
}

ntvdm64_status vm_composition_session_model_create(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config)
{
    core_machine_config machine_config = {
        CORE_MACHINE_PROFILE_CUSTOM, 0u
    };
    ntvdm64_status status;

    if (session == STD_NULL || config == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(session, 0, sizeof(*session));
    core_product_runtime_registry_initialize(&session->registry);
    status = vm_profile_register_default_profile_builtin(&session->registry);
    if (status != NTVDM64_STATUS_OK ||
        core_product_runtime_registry_find_profile(&session->registry,
            VM_PROFILE_PC_AT_PROFILE_ID, NXVM_RUNTIME_PROFILE_MACHINE,
            STD_NULL, STD_NULL) == STD_NULL ||
        core_product_runtime_registry_find_firmware_provider(&session->registry,
            VM_PROFILE_PC_AT_PROVIDER_ID,
            VM_PROFILE_PC_AT_PROFILE_ID) == STD_NULL ||
        core_product_runtime_registry_freeze(&session->registry) != NTVDM64_STATUS_OK) {
        return NTVDM64_STATUS_FAULT;
    }
    core_machine_firmware_initialize(&session->firmware);
    if (vm_profile_default_firmware_compose(&session->firmware, &session->firmware_plan) !=
            NTVDM64_STATUS_OK || core_machine_firmware_freeze(&session->firmware) !=
            NTVDM64_STATUS_OK || core_machine_create(&machine_config,
            &session->firmware_machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(session->firmware_machine) != NTVDM64_STATUS_OK ||
        vm_profile_default_firmware_apply_image(session->firmware_machine,
            config->boot_target == VM_PRODUCT_BOOT_HDD) != NTVDM64_STATUS_OK) {
        vm_composition_session_model_destroy(session);
        return NTVDM64_STATUS_FAULT;
    }
    vm_profile_default_firmware_cmos_initialize(&session->cmos,
        config->boot_target == VM_PRODUCT_BOOT_HDD);
    status = vm_composition_session_model_configure_media(session, config);
    if (status != NTVDM64_STATUS_OK ||
        (status = vm_composition_default_profile_create(&session->default_profile, &session->media)) !=
            NTVDM64_STATUS_OK ||
        vm_product_debugger_initialize(&session->debugger,
            session->firmware_machine) != NTVDM64_STATUS_OK) {
        vm_composition_session_model_destroy(session);
        return status == NTVDM64_STATUS_OK ? NTVDM64_STATUS_FAULT : status;
    }
    vm_product_console_initialize(&session->console);
    vm_product_presentation_initialize(&session->presentation);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_session_model_get_firmware_reset_vector(
    const vm_composition_session_model *session,
    vm_composition_default_profile_reset_vector *out_vector)
{
    uint8_t reset[5];

    if (session == STD_NULL || out_vector == STD_NULL || session->firmware_machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_memory_read(session->firmware_machine, 0xffff0u,
        reset, sizeof(reset)) != NTVDM64_STATUS_OK || reset[0] != 0xeau ||
        reset[4] != 0xf0u) return NTVDM64_STATUS_FAULT;
    out_vector->cs = (uint16_t)session->firmware_plan.reset_segment;
    out_vector->ip = (uint16_t)session->firmware_plan.reset_offset;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_session_model_get_execution_reset_vector(
    const vm_composition_session_model *session,
    vm_composition_default_profile_reset_vector *out_vector)
{
    if (session == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return vm_composition_default_profile_get_reset_vector(&session->default_profile, out_vector);
}

C_VOID vm_composition_session_model_destroy(vm_composition_session_model *session)
{
    if (session == STD_NULL) return;
    vm_composition_default_profile_destroy(&session->default_profile);
    if (session->firmware_machine != STD_NULL) {
        core_machine_destroy(session->firmware_machine);
    }
    STD_MEMSET(session, 0, sizeof(*session));
}
