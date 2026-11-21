#include "app-myob300/ob300_machine_interface.h"
#include "app-myob300/ob300_card.h"
#include "lib/storage/file_interface.h"
#include "x86-nxvm/machine/media/media_interface.h"
#include "x86-nxvm/machine/preparation_interface.h"

static lib_status myob300_machine_configure(void *owner, x86_nxvm_board_plan *plan)
{
    /* System 1.0 ABA uses the primary ATA task file and IVT vector 76h. */
    const x86_nxvm_board_hdc_config ata = {
        .protocol = X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO,
        .irq = 14u,
        .bus.task_file = {
            .data_port = 0x01f0u,
            .error_features_port = 0x01f1u,
            .sector_count_port = 0x01f2u,
            .sector_number_port = 0x01f3u,
            .cylinder_low_port = 0x01f4u,
            .cylinder_high_port = 0x01f5u,
            .drive_head_port = 0x01f6u,
            .status_command_port = 0x01f7u,
            .alternate_status_device_control_port = 0x03f6u,
            .ata_power_on_cylinder_low = 0xaau,
            .ata_power_on_cylinder_high = 0x55u
        }
    };
    lib_status status = myob300_board_configure(owner, plan);

    if (status != LIB_STATUS_OK) return status;
    return x86_nxvm_board_plan_configure_hdc(plan, X86_NXVM_MACHINE_MEDIA_HDD_ID,
        X86_NXVM_BOARD_MEDIA_ID_INVALID, &ata);
}

static lib_status myob300_machine_capture_display(void *owner,
    x86_video_snapshot *out_snapshot)
{
    const myob300_board_setup *setup = owner;

    return myob300_board_capture_display(setup->profile, out_snapshot);
}

static lib_bool myob300_machine_handle_input(void *owner, const kvm_input_event *event)
{
    const myob300_board_setup *setup = owner;

    if (event->type == KVM_EVENT_KEY && event->data.key.key == BASE_KEY_F13) {
        myob300_board_set_power_key(setup->profile, event->data.key.pressed);
        return LIB_TRUE;
    }
    if (event->type != KVM_EVENT_MOUSE) return LIB_FALSE;
    myob300_board_receive_mouse(setup->profile, event->data.mouse.delta_x,
        event->data.mouse.delta_y, event->data.mouse.buttons);
    return LIB_TRUE;
}

static void myob300_machine_release(void *owner)
{
    myob300_board_setup *setup = owner;

    lib_release((void *)setup->common.data);
    lib_release(setup);
}

lib_status myob300_machine_prepare(const x86_nxvm_machine_config *config,
    const char *system_card_path, x86_nxvm_machine_construction *out)
{
    const x86_nxvm_machine_assets assets = {0};
    x86_nxvm_machine_construction candidate;
    myob300_board_setup *setup;
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;
    lib_status status = x86_nxvm_machine_construction_begin(config, &assets, out);

    if (status != LIB_STATUS_OK) return status;
    if (system_card_path == LIB_NULL || system_card_path[0] == '\0' ||
        (config->memory_bytes != 0u && config->memory_bytes != myob300_board_configuration.memory_bytes))
        return LIB_STATUS_INVALID_ARGUMENT;
    setup = lib_allocate_zero(1u, sizeof(*setup));
    if (setup == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    setup->fixed_disk_card_installed = config->fixed_disk_image[0] != LIB_NULL ||
        config->create_hdd_cylinders != 0u;
    status = lib_storage_file_read_owned(system_card_path, MYOB300_SYSTEM_CARD_BYTES,
        &bytes, &byte_count);
    setup->common = (myob300_card_image){bytes, byte_count};
    if (status == LIB_STATUS_OK && byte_count != MYOB300_SYSTEM_CARD_BYTES)
        status = LIB_STATUS_INVALID_ARGUMENT;
    candidate = (x86_nxvm_machine_construction){
        .core_config = myob300_board_configuration,
        .topology = myob300_board_topology,
        .fixed_disk_slot_count = 1u,
        .hdc_present = LIB_TRUE,
        .display_snapshot_provider = myob300_machine_capture_display,
        .display_snapshot_context = setup,
        .firmware_provider = &myob300_card_firmware,
        .firmware_context = &setup->common,
        .profile = {
            .context = setup,
            .configure = myob300_machine_configure,
            .handle_input = myob300_machine_handle_input,
            .release = myob300_machine_release
        }
    };
    return x86_nxvm_machine_construction_finish(&candidate, config, &assets, status, out);
}
