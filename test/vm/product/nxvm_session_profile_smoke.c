#include "type.h"

#include "core/product/session/session_interface.h"
#include "core/product/session/session_provider.h"
#include "vm/composition/session/provider.h"
#include "vm/product/session_catalog.h"

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
        .profile = "default-pc-at", .display = "console"
    };
    const vm_product_session_request model_339 = {
        .profile = "ibm-5170-model-339", .display = "console"
    };
    const vm_product_session_request default_at_8086 = {
        .profile = "default-pc-at", .cpu = "8086", .fpu = "none",
        .display = "console"
    };
    const vm_product_session_request default_at_8086_8087 = {
        .profile = "default-pc-at", .cpu = "8086", .fpu = "8087",
        .display = "console"
    };
    vm_product_session_request model_339_memory = model_339;
    vm_product_session_request model_339_disk = model_339;
    vm_product_session_request model_339_cpu = model_339;
    vm_product_session_request default_at_video_rom = default_at;
    const vm_product_session_request invalid_display = {
        .profile = "default-pc-at", .display = "telepathy"
    };
    const vm_product_session_request missing_model40_firmware = {
        .profile = "compaq-deskpro-386-model-40", .display = "console"
    };
    const vm_product_session_request unknown_profile = {
        .profile = "unknown", .display = "console"
    };
    const vm_product_session_request numeric_profile = {
        .profile = "1", .display = "console"
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
    typedef struct session_request_case {
        const C_CHAR *name;
        const vm_product_session_request *request;
    } session_request_case;
    const session_request_case rejected[] = {
        { "default-at-missing-firmware", &default_at },
        { "model-339-missing-firmware", &model_339 },
        { "default-at-8086-missing-firmware", &default_at_8086 },
        { "default-at-8086-8087-missing-firmware", &default_at_8086_8087 },
        { "model-339-memory", &model_339_memory },
        { "model-339-disk", &model_339_disk },
        { "model-339-cpu", &model_339_cpu },
        { "default-at-video-rom", &default_at_video_rom },
        { "invalid-display", &invalid_display },
        { "missing-model40-firmware", &missing_model40_firmware },
        { "unknown-profile", &unknown_profile },
        { "numeric-profile", &numeric_profile }
    };
    STD_SIZE_T index;

    model_339_memory.memory_bytes = 1024u;
    STD_STRCPY(model_339_disk.fixed_disk[0], "disk.img");
    model_339_disk.fixed_disk_count = 1u;
    STD_STRCPY(model_339_cpu.cpu, "80386");
    STD_STRCPY(default_at_video_rom.video, "video.rom");
    vm_session_provider_initialize(&provider);
    if (core_product_session_manager_create(&provider, &manager) != TYPE_STATUS_OK)
        goto fail;
    if (core_product_session_manager_open_with_options(manager, &legacy, &id) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_product_session_manager_open_with_options(manager, &invalid_size, &id) !=
            TYPE_STATUS_INVALID_ARGUMENT) goto fail;
    for (index = 0u; index < sizeof(rejected) / sizeof(rejected[0]); ++index) {
        if (verify_rejected(manager, rejected[index].request)) {
            STD_PRINTF("session request accepted unexpectedly: %s\n",
                rejected[index].name);
            goto fail;
        }
    }
    core_product_session_manager_destroy(manager);
    STD_PRINTF("M5:T482:S3:SESSION-REQUEST:OK\n");
    return 0;

fail:
    core_product_session_manager_destroy(manager);
    return 1;
}
