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

    type_status status = core_product_session_manager_open_with_options(manager,
        &options, &id);

    if (status != TYPE_STATUS_OK || id != expected_id ||
        core_product_session_manager_select(manager, id) != TYPE_STATUS_OK ||
        core_product_session_manager_get_selected_snapshot(manager, &snapshot) !=
            TYPE_STATUS_OK || STD_STRCMP(snapshot.details, details)) return 1;
    return 0;
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
    typedef struct session_request_case {
        const C_CHAR *name;
        const vm_product_session_request *request;
        const C_CHAR *details;
    } session_request_case;
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
    vm_product_session_request default_at_video_rom = default_at;
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
    const session_request_case accepted[] = {
        { "default-at", &default_at, "profile=default-pc-at cpu=80386 fpu=none" },
        { "model-339", &model_339, "profile=ibm-5170-model-339 cpu=80286 fpu=none" },
        { "default-at-8086", &default_at_8086, "profile=default-pc-at cpu=8086 fpu=none" },
        { "default-at-8086-8087", &default_at_8086_8087, "profile=default-pc-at cpu=8086 fpu=8087" }
    };
    const session_request_case rejected[] = {
        { "model-339-memory", &model_339_memory, STD_NULL },
        { "model-339-disk", &model_339_disk, STD_NULL },
        { "model-339-cpu", &model_339_cpu, STD_NULL },
        { "default-at-video-rom", &default_at_video_rom, STD_NULL },
        { "invalid-display", &invalid_display, STD_NULL },
        { "invalid-boot", &invalid_boot, STD_NULL },
        { "missing-boot-media", &missing_boot_media, STD_NULL },
        { "missing-model40-firmware", &missing_model40_firmware, STD_NULL },
        { "model40-non-rom-boot", &model40_non_rom_boot, STD_NULL },
        { "unknown-profile", &unknown_profile, STD_NULL },
        { "numeric-profile", &numeric_profile, STD_NULL }
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
    for (index = 0u; index < sizeof(accepted) / sizeof(accepted[0]); ++index) {
        if (verify_request(manager, accepted[index].request,
                accepted[index].details, (core_product_session_id)index)) {
            STD_PRINTF("session request failed: %s\n", accepted[index].name);
            goto fail;
        }
    }
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
