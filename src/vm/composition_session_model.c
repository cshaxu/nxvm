#include "vm/product/session.h"

#include <string.h>

#include "vm/profile/default_profile/profile.h"

static nxvm_core_status nxvm_product_nxvm_session_configure_media(
    nxvm_product_nxvm_session *session,
    const nxvm_product_nxvm_session_config *config)
{
    nxvm_core_status status;

    nxvm_product_nxvm_media_policy_initialize(&session->media);
    if (config->create_fdd) {
        status = nxvm_product_nxvm_media_configure_created(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_FDD, 0u);
        if (status != NXVM_CORE_STATUS_OK) return status;
    } else if (config->fdd_path != NULL) {
        status = nxvm_product_nxvm_media_configure(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_FDD, config->fdd_path, config->fdd_identity);
        if (status != NXVM_CORE_STATUS_OK) return status;
    }
    if (config->create_hdd_cylinders != 0u) {
        status = nxvm_product_nxvm_media_configure_created(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_HDD, config->create_hdd_cylinders);
        if (status != NXVM_CORE_STATUS_OK) return status;
    } else if (config->hdd_path != NULL) {
        status = nxvm_product_nxvm_media_configure(&session->media,
            NXVM_PRODUCT_NXVM_BOOT_HDD, config->hdd_path, config->hdd_identity);
        if (status != NXVM_CORE_STATUS_OK) return status;
    }
    status = nxvm_product_nxvm_media_set_boot_target(&session->media,
                                                      config->boot_target);
    return status == NXVM_CORE_STATUS_OK ?
        nxvm_product_nxvm_media_freeze(&session->media) : status;
}

nxvm_core_status nxvm_product_nxvm_session_create(
    nxvm_product_nxvm_session *session,
    const nxvm_product_nxvm_session_config *config)
{
    nxvm_core_machine_config machine_config = {
        NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_CUSTOM, 0u
    };
    nxvm_core_cpu_capability_manifest capabilities;
    nxvm_core_status status;

    if (session == NULL || config == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    memset(session, 0, sizeof(*session));
    nxvm_core_cpu_capability_manifest_initialize(&capabilities);
    nxvm_runtime_registry_initialize(&session->registry);
    status = nxvm_product_nxvm_register_default_profile_builtin(&session->registry);
    if (status != NXVM_CORE_STATUS_OK ||
        nxvm_runtime_registry_find_profile(&session->registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID, NXVM_RUNTIME_PROFILE_MACHINE,
            &capabilities) == NULL ||
        nxvm_runtime_registry_find_firmware_provider(&session->registry,
            NXVM_PRODUCT_NXVM_PC_AT_PROVIDER_ID,
            NXVM_PRODUCT_NXVM_PC_AT_PROFILE_ID) == NULL ||
        nxvm_runtime_registry_freeze(&session->registry) != NXVM_CORE_STATUS_OK) {
        return NXVM_CORE_STATUS_FAULT;
    }
    nxvm_firmware_initialize(&session->firmware);
    if (nxvm_firmware_default_profile_compose(&session->firmware, &session->firmware_plan) !=
            NXVM_CORE_STATUS_OK || nxvm_firmware_freeze(&session->firmware) !=
            NXVM_CORE_STATUS_OK || nxvm_core_machine_create(&machine_config,
            &session->firmware_machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_reset(session->firmware_machine) != NXVM_CORE_STATUS_OK ||
        nxvm_firmware_default_profile_apply_image(session->firmware_machine,
            config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD) != NXVM_CORE_STATUS_OK) {
        nxvm_product_nxvm_session_destroy(session);
        return NXVM_CORE_STATUS_FAULT;
    }
    nxvm_firmware_default_profile_cmos_initialize(&session->cmos,
        config->boot_target == NXVM_PRODUCT_NXVM_BOOT_HDD);
    status = nxvm_product_nxvm_session_configure_media(session, config);
    if (status != NXVM_CORE_STATUS_OK ||
        (status = nxvm_product_nxvm_default_profile_create(&session->default_profile, &session->media)) !=
            NXVM_CORE_STATUS_OK ||
        nxvm_product_nxvm_debugger_initialize(&session->debugger,
            session->firmware_machine) != NXVM_CORE_STATUS_OK) {
        nxvm_product_nxvm_session_destroy(session);
        return status == NXVM_CORE_STATUS_OK ? NXVM_CORE_STATUS_FAULT : status;
    }
    nxvm_product_nxvm_console_initialize(&session->console);
    nxvm_product_nxvm_presentation_initialize(&session->presentation);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_session_get_firmware_reset_vector(
    const nxvm_product_nxvm_session *session,
    nxvm_product_nxvm_reset_vector *out_vector)
{
    uint8_t reset[5];

    if (session == NULL || out_vector == NULL || session->firmware_machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (nxvm_core_machine_memory_read(session->firmware_machine, 0xffff0u,
        reset, sizeof(reset)) != NXVM_CORE_STATUS_OK || reset[0] != 0xeau ||
        reset[4] != 0xf0u) return NXVM_CORE_STATUS_FAULT;
    out_vector->cs = (uint16_t)session->firmware_plan.reset_segment;
    out_vector->ip = (uint16_t)session->firmware_plan.reset_offset;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_session_get_execution_reset_vector(
    const nxvm_product_nxvm_session *session,
    nxvm_product_nxvm_reset_vector *out_vector)
{
    if (session == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    return nxvm_product_nxvm_default_profile_get_reset_vector(&session->default_profile, out_vector);
}

void nxvm_product_nxvm_session_destroy(nxvm_product_nxvm_session *session)
{
    if (session == NULL) return;
    nxvm_product_nxvm_default_profile_destroy(&session->default_profile);
    if (session->firmware_machine != NULL) {
        nxvm_core_machine_destroy(session->firmware_machine);
    }
    memset(session, 0, sizeof(*session));
}
