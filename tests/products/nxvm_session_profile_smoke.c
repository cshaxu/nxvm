#include "type.h"

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/provider.h"
#include "vm/product/session_catalog.h"

static C_INT verify_request(core_product_session_manager *manager,
    const vm_product_session_request *request, const C_CHAR *details,
    core_product_session_id expected_id)
{
    const core_product_session_open_options options = {
        0, STD_NULL, request, sizeof(*request)
    };
    core_product_session_snapshot snapshot;
    core_product_session_id id;

    return core_product_session_manager_open_with_options(manager, &options,
            &id) != TYPE_STATUS_OK || id != expected_id ||
        core_product_session_manager_select(manager, id) != TYPE_STATUS_OK ||
        core_product_session_manager_get_selected_snapshot(manager, &snapshot) !=
            TYPE_STATUS_OK || STD_STRCMP(snapshot.details, details);
}

static C_INT verify_rejected(core_product_session_manager *manager,
    const vm_product_session_request *request)
{
    const core_product_session_open_options options = {
        0, STD_NULL, request, sizeof(*request)
    };
    core_product_session_id id;

    return core_product_session_manager_open_with_options(manager, &options,
        &id) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    const vm_product_session_request default_at = {
        .profile = "default-pc-at", .display = "console", .boot = "rom"
    };
    const vm_product_session_request model_339 = {
        .profile = "ibm-5170-model-339", .display = "console", .boot = "rom"
    };
    const vm_product_session_request default_at_8086 = {
        .profile = "default-pc-at", .cpu = "8086", .fpu = "none",
        .display = "console", .boot = "rom"
    };
    const vm_product_session_request default_at_8086_8087 = {
        .profile = "default-pc-at", .cpu = "8086", .fpu = "8087",
        .display = "console", .boot = "rom"
    };
    vm_product_session_request model_339_memory = model_339;
    vm_product_session_request model_339_disk = model_339;
    vm_product_session_request model_339_cpu = model_339;
    vm_product_session_request default_at_firmware = default_at;
    const vm_product_session_request invalid_display = {
        .profile = "default-pc-at", .display = "telepathy", .boot = "rom"
    };
    const vm_product_session_request invalid_boot = {
        .profile = "default-pc-at", .display = "console", .boot = "tape"
    };
    const vm_product_session_request missing_boot_media = {
        .profile = "default-pc-at", .display = "console", .boot = "hard_disk"
    };
    const vm_product_session_request missing_model40_firmware = {
        .profile = "compaq-deskpro-386-model-40", .display = "console", .boot = "rom"
    };
    const vm_product_session_request model40_non_rom_boot = {
        .profile = "compaq-deskpro-386-model-40", .display = "console", .boot = "floppy"
    };
    const vm_product_session_request unknown_profile = {
        .profile = "unknown", .display = "console", .boot = "rom"
    };
    const vm_product_session_request numeric_profile = {
        .profile = "1", .display = "console", .boot = "rom"
    };
    const C_CHAR *legacy_arguments[] = { "--profile", "default-pc-at" };
    const core_product_session_open_options legacy = {
        2, legacy_arguments, STD_NULL, 0u
    };
    const core_product_session_open_options invalid_size = {
        0, STD_NULL, &default_at, sizeof(default_at) - 1u
    };
    core_product_session_provider provider;
    core_product_session_manager *manager = STD_NULL;
    core_product_session_id id;

    model_339_memory.memory_bytes = 1024u;
    STD_STRCPY(model_339_disk.hard_disk, "disk.img");
    STD_STRCPY(model_339_cpu.cpu, "80386");
    STD_STRCPY(default_at_firmware.model40_provenance, "not allowed");
    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK ||
        verify_request(manager, &default_at,
            "profile=default-pc-at cpu=80386 fpu=none", 0u) ||
        verify_request(manager, &model_339,
            "profile=ibm-5170-model-339 cpu=80286 fpu=none", 1u) ||
        verify_request(manager, &default_at_8086,
            "profile=default-pc-at cpu=8086 fpu=none", 2u) ||
        verify_request(manager, &default_at_8086_8087,
            "profile=default-pc-at cpu=8086 fpu=8087", 3u) ||
        core_product_session_manager_open_with_options(manager, &legacy, &id) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_product_session_manager_open_with_options(manager, &invalid_size, &id) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        verify_rejected(manager, &model_339_memory) ||
        verify_rejected(manager, &model_339_disk) ||
        verify_rejected(manager, &model_339_cpu) ||
        verify_rejected(manager, &default_at_firmware) ||
        verify_rejected(manager, &invalid_display) ||
        verify_rejected(manager, &invalid_boot) ||
        verify_rejected(manager, &missing_boot_media) ||
        verify_rejected(manager, &missing_model40_firmware) ||
        verify_rejected(manager, &model40_non_rom_boot) ||
        verify_rejected(manager, &unknown_profile) ||
        verify_rejected(manager, &numeric_profile)) goto fail;
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T482:S3:SESSION-REQUEST:OK\n");
    return 0;

fail:
    core_product_session_manager_destroy(manager);
    return 1;
}
