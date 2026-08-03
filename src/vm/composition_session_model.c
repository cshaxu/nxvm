#include "vm/composition_session_model.h"

#include <string.h>

#include "vm/profile/default_profile/profile.h"

static ntvdm64_status vm_composition_session_model_configure_media(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config)
{
    ntvdm64_status status;

    nxvm_product_nxvm_media_policy_initialize(&session->media);
    if (config->create_fdd) {
        status = nxvm_product_nxvm_media_configure_created(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_FDD, 0u);
        if (status != NTVDM64_STATUS_OK) return status;
    } else if (config->fdd_path != NULL) {
        status = nxvm_product_nxvm_media_configure(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_FDD, config->fdd_path, config->fdd_identity);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    if (config->create_hdd_cylinders != 0u) {
        status = nxvm_product_nxvm_media_configure_created(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_HDD, config->create_hdd_cylinders);
        if (status != NTVDM64_STATUS_OK) return status;
    } else if (config->hdd_path != NULL) {
        status = nxvm_product_nxvm_media_configure(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_HDD, config->hdd_path, config->hdd_identity);
        if (status != NTVDM64_STATUS_OK) return status;
    }
    status = nxvm_product_nxvm_media_set_boot_target(&session->media,
                                                      config->boot_target);
    return status == NTVDM64_STATUS_OK ?
        nxvm_product_nxvm_media_freeze(&session->media) : status;
}

ntvdm64_status vm_composition_session_model_create(
    vm_composition_session_model *session,
    const vm_composition_session_model_config *config)
{
    core_machine_config machine_config = {
        CORE_MACHINE_PROFILE_CUSTOM, 0u
    };
    ntvdm64_status status;

    if (session == NULL || config == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    memset(session, 0, sizeof(*session));
    nxvm_runtime_registry_initialize(&session->registry);
    status = nxvm_product_nxvm_register_default_profile_builtin(&session->registry);
    if (status != NTVDM64_STATUS_OK ||
        nxvm_runtime_registry_find_profile(&session->registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID, NXVM_RUNTIME_PROFILE_MACHINE,
            NULL, NULL) == NULL ||
        nxvm_runtime_registry_find_firmware_provider(&session->registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID) == NULL ||
        nxvm_runtime_registry_freeze(&session->registry) != NTVDM64_STATUS_OK) {
        return NTVDM64_STATUS_FAULT;
    }
    core_machine_firmware_initialize(&session->firmware);
    if (vm_profile_default_firmware_compose(&session->firmware, &session->firmware_plan) !=
            NTVDM64_STATUS_OK || core_machine_firmware_freeze(&session->firmware) !=
            NTVDM64_STATUS_OK || core_machine_create(&machine_config,
            &session->firmware_machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(session->firmware_machine) != NTVDM64_STATUS_OK ||
        vm_profile_default_firmware_apply_image(session->firmware_machine,
            config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD) != NTVDM64_STATUS_OK) {
        vm_composition_session_model_destroy(session);
        return NTVDM64_STATUS_FAULT;
    }
    vm_profile_default_firmware_cmos_initialize(&session->cmos,
        config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD);
    status = vm_composition_session_model_configure_media(session, config);
    if (status != NTVDM64_STATUS_OK ||
        (status = nxvm_product_nxvm_default_profile_create(&session->default_profile, &session->media)) !=
            NTVDM64_STATUS_OK ||
        nxvm_product_nxvm_debugger_initialize(&session->debugger,
            session->firmware_machine) != NTVDM64_STATUS_OK) {
        vm_composition_session_model_destroy(session);
        return status == NTVDM64_STATUS_OK ? NTVDM64_STATUS_FAULT : status;
    }
    nxvm_product_nxvm_console_initialize(&session->console);
    nxvm_product_nxvm_presentation_initialize(&session->presentation);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_composition_session_model_get_firmware_reset_vector(
    const vm_composition_session_model *session,
    nxvm_product_nxvm_reset_vector *out_vector)
{
    uint8_t reset[5];

    if (session == NULL || out_vector == NULL || session->firmware_machine == NULL) {
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
    nxvm_product_nxvm_reset_vector *out_vector)
{
    if (session == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    return nxvm_product_nxvm_default_profile_get_reset_vector(&session->default_profile, out_vector);
}

void vm_composition_session_model_destroy(vm_composition_session_model *session)
{
    if (session == NULL) return;
    nxvm_product_nxvm_default_profile_destroy(&session->default_profile);
    if (session->firmware_machine != NULL) {
        core_machine_destroy(session->firmware_machine);
    }
    memset(session, 0, sizeof(*session));
}
