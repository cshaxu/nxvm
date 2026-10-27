#include "lib/types/test.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/media/media_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/media/hdd_interface.h"
#include "x86-core/machine/port_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/types/file.h"

typedef struct construction_probe {
    lib_status configure_status;
    lib_status after_firmware_status;
    lib_status reset_status;
    lib_size configure_count;
    lib_size after_firmware_count;
    lib_size firmware_count;
    lib_size reset_count;
    lib_size reset_notice_count;
    lib_size detach_count;
    lib_size release_count;
    lib_size key_count;
    lib_bool last_key_pressed;
    lib_bool released;
    lib_bool ata;
} construction_probe;

static lib_status configure_plan(void *context, x86_nxvm_board_plan *plan)
{
    construction_probe *probe = context;

    lib_test_assert(plan != LIB_NULL && !probe->released);
    ++probe->configure_count;
    if (probe->ata) {
        const x86_nxvm_board_hdc_config config = {
            .protocol = X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO,
            .irq = 14u,
            .bus.task_file = {.data_port = 0x1f0u, .error_features_port = 0x1f1u,
                .sector_count_port = 0x1f2u, .sector_number_port = 0x1f3u,
                .cylinder_low_port = 0x1f4u, .cylinder_high_port = 0x1f5u,
                .drive_head_port = 0x1f6u, .status_command_port = 0x1f7u,
                .alternate_status_device_control_port = 0x3f6u}};
        return x86_nxvm_board_plan_configure_hdc(plan, X86_NXVM_MACHINE_MEDIA_HDD_ID,
            X86_NXVM_MACHINE_MEDIA_HDD_SECONDARY_ID, &config);
    }
    return probe->configure_status;
}

static lib_status configure_after_firmware(void *context, x86_core_machine *core)
{
    construction_probe *probe = context;

    lib_test_assert(core != LIB_NULL && !probe->released);
    ++probe->after_firmware_count;
    return probe->after_firmware_status;
}

static lib_status configure_firmware(void *context,
    x86_core_machine_firmware_context *firmware)
{
    construction_probe *probe = context;
    const lib_u8 reset_code[16] = {0xf4u};

    lib_test_assert(!probe->released);
    ++probe->firmware_count;
    return x86_core_machine_firmware_register_immutable_rom(firmware,
        0xffff0u, reset_code, sizeof(reset_code));
}

static lib_status reset_firmware(void *context,
    x86_core_machine_firmware_context *firmware)
{
    construction_probe *probe = context;

    lib_test_assert(firmware != LIB_NULL && !probe->released);
    ++probe->reset_count;
    return probe->reset_status;
}

static void notify_profile(void *context, x86_nxvm_machine_profile_event event)
{
    construction_probe *probe = context;

    lib_test_assert(!probe->released);
    if (event == X86_NXVM_MACHINE_PROFILE_RESET_COMPLETED)
        ++probe->reset_notice_count;
    else {
        lib_test_assert(event == X86_NXVM_MACHINE_PROFILE_BOARD_DETACHED);
        ++probe->detach_count;
    }
}

static void release_profile(void *context)
{
    construction_probe *probe = context;

    lib_test_assert(!probe->released);
    probe->released = LIB_TRUE;
    ++probe->release_count;
}

static lib_bool handle_key(void *context, const kvm_input_event *event)
{
    construction_probe *probe = context;

    lib_test_assert(!probe->released && event->type == KVM_EVENT_KEY);
    ++probe->key_count;
    probe->last_key_pressed = event->data.key.pressed;
    return event->data.key.key == BASE_KEY_F12;
}

static const x86_core_machine_firmware_provider firmware = {
    configure_firmware, reset_firmware, LIB_NULL
};

static x86_nxvm_machine_construction prepare(construction_probe *probe)
{
    x86_nxvm_machine_construction value = {
        .core_config = {
            .memory_bytes = (2u * 1024u * 1024u),
            .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042,
            .shared_pit_personality = X86_PIT_PERSONALITY_8254
        },
        .topology = {
            .dma_present = LIB_TRUE,
            .dma = {2u, X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
                X86_NXVM_BOARD_DMA_CASCADE_CHANNEL},
            .fdc_present = LIB_TRUE,
            .fdc_drives = {
                .media_id = {X86_NXVM_MACHINE_MEDIA_FDD_ID},
                .installed_mask = 1u,
                .double_sided_mask = 1u,
                .cylinder_count = {80u}
            },
            .fdc = { .dor_port = 0x3f2u, .status_port = 0x3f4u,
                .data_port = 0x3f5u, .irq = 6u, .dma_channel = 2u,
                .clock_ticks_per_second = 1000000u }
        },
        .floppy_slot_count = 1u,
        .memory_reconfigurable = LIB_TRUE,
        .firmware_provider = &firmware,
        .firmware_context = probe,
        .profile = {.context = probe, .configure = configure_plan,
            .after_firmware = configure_after_firmware,
            .notify = notify_profile, .release = release_profile,
            .handle_key = handle_key}
    };
    return value;
}

static void check_memory_replacement(x86_nxvm_machine *machine)
{
    x86_nxvm_machine *identity = machine;
    x86_core_machine *core = machine->x86_core_machine;
    t_fdd *floppy = machine->floppy[0u];
    t_hdd *disk = machine->fixed_disk[0u];
    t_debug *debug = &machine->debug;
    x86_nxvm_machine_reset_vector vector;
    x86_nxvm_machine_information information;
    lib_size bytes = 0u;
    lib_test_assert(x86_nxvm_machine_reconfigure_memory(machine, 32u * 1024u * 1024u) == LIB_STATUS_OK);
    lib_test_assert(machine == identity && machine->x86_core_machine == core &&
        machine->floppy[0u] == floppy && machine->fixed_disk[0u] == disk && &machine->debug == debug);
    lib_test_assert(x86_core_machine_get_memory_bytes(core, &bytes) == LIB_STATUS_OK && bytes == 32u * 1024u * 1024u);
    lib_test_assert(x86_nxvm_machine_get_information(machine, &information) == LIB_STATUS_OK &&
        information.memory_bytes == 32u * 1024u * 1024u);
    lib_test_assert(x86_nxvm_machine_get_reset_vector(LIB_NULL, &vector) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_get_reset_vector(machine, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_get_reset_vector(machine, &vector) == LIB_STATUS_OK &&
        vector.cs == 0xf000u && vector.ip == 0xfff0u);
}

static void check_floppy_lifecycle(void)
{
    static const char path[] = "machine-owned-lifecycle.img";
    static const lib_u8 zeroes[4096u];
    lib_storage_file_writer *writer = LIB_NULL;
    construction_probe probe = {0};
    x86_nxvm_machine_construction construction = prepare(&probe);
    const x86_nxvm_machine_config runtime = {.create_fdd = 1};
    x86_nxvm_machine *machine = LIB_NULL;
    emulator_machine *executor = LIB_NULL;
    emulator_machine_driver driver;
    x86_nxvm_board_media_info before, after;
    const x86_nxvm_board_media_provider *media = x86_nxvm_machine_fdd_media_provider();
    lib_test_assert(lib_storage_file_writer_open(path, LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    for (lib_size remaining = 1440u * 1024u; remaining != 0u; remaining -= sizeof(zeroes))
        lib_test_assert(lib_storage_file_writer_write(writer, zeroes, sizeof(zeroes)) == LIB_STATUS_OK);
    lib_test_assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_create(&runtime, &construction, &machine) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_describe_emulator_driver(machine, &driver) == LIB_STATUS_OK);
    lib_test_assert(emulator_machine_create(&executor, &driver) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_bind_emulator_machine(machine, executor) == LIB_STATUS_OK);
    lib_test_assert(media->query(machine->floppy[0u], &before) == X86_NXVM_BOARD_MEDIA_RESULT_OK);
    lib_test_assert(x86_nxvm_machine_insert_fdd(machine, path) == 0);
    lib_test_assert(media->query(machine->floppy[0u], &after) == X86_NXVM_BOARD_MEDIA_RESULT_OK &&
        after.generation == before.generation + 1u && machine->floppy_image_path[0u][0] != '\0');
    before = after;
    x86_nxvm_machine_executor_state_start(&machine->control.state);
    lib_test_assert(x86_nxvm_machine_insert_fdd(machine, path) != 0);
    lib_test_assert(media->query(machine->floppy[0u], &after) == X86_NXVM_BOARD_MEDIA_RESULT_OK &&
        after.generation == before.generation && machine->floppy_image_path[0u][0] != '\0');
    lib_test_assert(x86_nxvm_machine_eject_fdd(machine) != 0);
    lib_test_assert(media->query(machine->floppy[0u], &after) == X86_NXVM_BOARD_MEDIA_RESULT_OK &&
        after.generation == before.generation && machine->floppy_image_path[0u][0] != '\0');
    x86_nxvm_machine_executor_state_stop(&machine->control.state);
    lib_test_assert(x86_nxvm_machine_eject_fdd(machine) == 0);
    lib_test_assert(media->query(machine->floppy[0u], &after) == X86_NXVM_BOARD_MEDIA_RESULT_ABSENT &&
        !after.present && machine->floppy_image_path[0u][0] == '\0');
    lib_test_assert(emulator_machine_destroy(executor) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_bind_emulator_machine(machine, LIB_NULL) == LIB_STATUS_OK);
    x86_nxvm_machine_destroy(machine);
    lib_test_assert(probe.released && lib_c_remove(path) == 0);
}

static void check_partial_media_cleanup(void)
{
    x86_nxvm_machine partial = {0};
    const x86_nxvm_board_media_geometry geometry = {2880u, 512u, 80u, 2u, 18u};

    lib_test_assert(x86_nxvm_machine_fdd_allocate(&geometry, &partial.floppy[0u]) ==
        LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_hdd_allocate(&partial.fixed_disk[0u]) ==
        LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_hdd_allocate(&partial.fixed_disk[1u]) ==
        LIB_STATUS_OK);
    x86_nxvm_machine_finalize(&partial);
    lib_test_assert(partial.floppy[0u] == LIB_NULL && partial.fixed_disk[0u] == LIB_NULL &&
        partial.fixed_disk[1u] == LIB_NULL);
    x86_nxvm_machine_finalize(&partial);
}

static void check_fixed_disk_slot_contract(void)
{
    construction_probe probe = {.ata = LIB_TRUE};
    x86_nxvm_machine_construction construction = prepare(&probe);
    const x86_nxvm_machine_config empty = {0};
    const x86_nxvm_machine_config second_on_single = {
        .fixed_disk_image = {LIB_NULL, "second.img"}};
    x86_nxvm_machine *machine = LIB_NULL;
    x86_nxvm_board_media_info information = {0};
    x86_nxvm_board_media_result result = X86_NXVM_BOARD_MEDIA_RESULT_PERMANENT;
    lib_u8 sector[512u] = {0x34u, 0x12u};
    lib_u32 value = 0u;
    x86_core_machine_run_result run = {0};

    construction.hdc_present = LIB_TRUE;
    construction.fixed_disk_slot_count = 2u;
    lib_test_assert(x86_nxvm_machine_create(&empty, &construction, &machine) == LIB_STATUS_OK);
    lib_test_assert(machine->fixed_disk[0u] != LIB_NULL && machine->fixed_disk[1u] != LIB_NULL);
    lib_test_assert(x86_nxvm_board_media_query(machine->media_registry,
        X86_NXVM_MACHINE_MEDIA_HDD_ID, &information, &result) == LIB_STATUS_OK &&
        result == X86_NXVM_BOARD_MEDIA_RESULT_ABSENT);
    lib_test_assert(x86_nxvm_board_media_query(machine->media_registry,
        X86_NXVM_MACHINE_MEDIA_HDD_SECONDARY_ID, &information, &result) == LIB_STATUS_OK &&
        result == X86_NXVM_BOARD_MEDIA_RESULT_ABSENT);
    /* The master is absent; the secondary media is an in-memory overlay. */
    lib_test_assert(x86_nxvm_machine_hdd_replace_bytes(machine->fixed_disk[1u],
        sector, sizeof(sector)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f6u, 0xb0u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f2u, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f3u, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f4u, 0u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f5u, 0u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_write(machine->x86_core_machine, 0x1f7u, 0x20u) == LIB_STATUS_OK);
    /* ATA commands complete on the ordinary Core/device time path. */
    lib_test_assert(x86_core_machine_run(machine->x86_core_machine,
        (x86_core_machine_run_budget){.instructions = 1u}, &run) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_bus_read(machine->x86_core_machine, 0x1f7u, &value) == LIB_STATUS_OK);
    if ((value & 0x08u) == 0u) lib_c_fprintf(lib_c_stderr, "ATA slave status=%02x\n", value);
    lib_test_assert((value & 0x08u) != 0u);
    lib_test_assert(x86_core_machine_bus_read(machine->x86_core_machine, 0x1f0u, &value) == LIB_STATUS_OK &&
        value == 0x1234u);
    x86_nxvm_machine_destroy(machine);
    lib_test_assert(probe.released && probe.release_count == 1u);

    probe = (construction_probe){0};
    construction = prepare(&probe);
    construction.hdc_present = LIB_TRUE;
    construction.fixed_disk_slot_count = 1u;
    lib_test_assert(x86_nxvm_machine_create(&second_on_single, &construction, &machine) ==
        LIB_STATUS_INVALID_ARGUMENT && machine == LIB_NULL && probe.release_count == 1u);
}

static void check_no_floppy_media_contract(void)
{
    construction_probe probe = {0};
    x86_nxvm_machine_construction construction = prepare(&probe);
    const x86_nxvm_machine_config runtime = {0};
    x86_nxvm_machine *machine = LIB_NULL;

    construction.floppy_slot_count = 0u;
    construction.topology.fdc_present = LIB_FALSE;
    lib_test_assert(x86_nxvm_machine_create(&runtime, &construction, &machine) ==
        LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_set_emulator_media(machine, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(x86_nxvm_machine_set_emulator_media(machine, "not-a-floppy.img",
        LIB_STORAGE_MEDIUM_OVERLAY) == LIB_STATUS_UNSUPPORTED);
    x86_nxvm_machine_destroy(machine);
    lib_test_assert(probe.released && probe.release_count == 1u);
}

static void check_cmos_seed_decode_contract(void)
{
    x86_nxvm_machine machine = {0};
    x86_nxvm_board_plan_topology topology = {.rtc_cmos_present = LIB_TRUE};

    machine.construction.cmos_seed_present = LIB_TRUE;
    machine.construction.cmos_seed_bytes = X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES;
    machine.construction.cmos_seed[0x0eu] = 0x1eu;
    machine.construction.cmos_seed[0x7fu] = 0x7fu;
    lib_test_assert(x86_nxvm_machine_apply_cmos_seed(&machine, &topology) ==
        LIB_STATUS_INVALID_STATE);
    topology.rtc_cmos.extended_registers = LIB_TRUE;
    lib_test_assert(x86_nxvm_machine_apply_cmos_seed(&machine, &topology) == LIB_STATUS_OK);
    lib_test_assert(topology.rtc_cmos.default_count == 114u &&
        topology.rtc_cmos.defaults[0u].index == 0x0eu &&
        topology.rtc_cmos.defaults[0u].value == 0x1eu &&
        topology.rtc_cmos.defaults[113u].index == 0x7fu &&
        topology.rtc_cmos.defaults[113u].value == 0x7fu);
}

static void check_transaction(lib_status configure_status,
    lib_status after_firmware_status, lib_status reset_status)
{
    construction_probe probe = {
        .configure_status = configure_status,
        .after_firmware_status = after_firmware_status,
        .reset_status = reset_status
    };
    x86_nxvm_machine_construction construction = prepare(&probe);
    const x86_nxvm_machine_config runtime = {0};
    x86_nxvm_machine *machine = LIB_NULL;
    emulator_machine_driver driver;
    x86_nxvm_machine_information information;
    lib_status status = x86_nxvm_machine_create(&runtime, &construction, &machine);

    lib_test_assert(probe.configure_count == 1u);
    if (configure_status != LIB_STATUS_OK || after_firmware_status != LIB_STATUS_OK ||
        reset_status != LIB_STATUS_OK) {
        lib_test_assert(status != LIB_STATUS_OK && machine == LIB_NULL);
        lib_test_assert(probe.released && probe.release_count == 1u);
        lib_test_assert(probe.detach_count == 1u);
        lib_test_assert(probe.reset_notice_count == 0u);
        return;
    }
    lib_test_assert(status == LIB_STATUS_OK && machine != LIB_NULL);
    lib_test_assert(!probe.released && probe.firmware_count == 1u &&
        probe.after_firmware_count == 1u);
    lib_test_assert(probe.reset_count == 1u && probe.reset_notice_count == 1u);
    /* Caller storage may change: the published owner consumed copied values. */
    construction.core_config.memory_bytes = 0u;
    construction.floppy_slot_count = 0u;
    construction.profile.handle_key = LIB_NULL;
    lib_test_assert(x86_nxvm_machine_get_information(machine, &information) == LIB_STATUS_OK);
    lib_test_assert(information.memory_bytes == (2u * 1024u * 1024u));
    lib_test_assert(x86_nxvm_machine_describe_emulator_driver(machine, &driver) == LIB_STATUS_OK);
    lib_test_assert(driver.context == machine && driver.reset(driver.context));
    lib_test_assert(probe.configure_count == 1u && probe.firmware_count == 1u);
    lib_test_assert(probe.reset_count == 2u && probe.reset_notice_count == 2u);
    kvm_input_event key = {.type = KVM_EVENT_KEY,
        .data.key = {.key = BASE_KEY_F12, .pressed = LIB_TRUE}};
    driver.deliver_input(driver.context, &key);
    lib_test_assert(probe.key_count == 1u && probe.last_key_pressed);
    key.data.key.pressed = LIB_FALSE;
    driver.deliver_input(driver.context, &key);
    lib_test_assert(probe.key_count == 2u && !probe.last_key_pressed);
    key.data.key.key = 'A';
    key.data.key.pressed = LIB_TRUE;
    driver.deliver_input(driver.context, &key);
    lib_test_assert(probe.key_count == 3u && probe.last_key_pressed);
    check_memory_replacement(machine);
    x86_nxvm_machine_destroy(machine);
    lib_test_assert(probe.released && probe.release_count == 1u);
    lib_test_assert(probe.detach_count == 1u);
}

lib_i32 main(void)
{
    construction_probe probe = {0};
    x86_nxvm_machine_construction construction = prepare(&probe);
    const x86_nxvm_machine_config runtime = {0};
    x86_nxvm_machine *machine = LIB_NULL;

    lib_test_assert(x86_nxvm_machine_create(LIB_NULL, &construction, &machine) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_nxvm_machine_create(&runtime, &construction, LIB_NULL) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(!probe.released && probe.configure_count == 0u);
    check_transaction(LIB_STATUS_OK, LIB_STATUS_OK, LIB_STATUS_OK);
    check_transaction(LIB_STATUS_INVALID_ARGUMENT, LIB_STATUS_OK, LIB_STATUS_OK);
    check_transaction(LIB_STATUS_OK, LIB_STATUS_INVALID_ARGUMENT, LIB_STATUS_OK);
    check_transaction(LIB_STATUS_OK, LIB_STATUS_OK, LIB_STATUS_INVALID_STATE);
    check_floppy_lifecycle();
    check_partial_media_cleanup();
    check_fixed_disk_slot_contract();
    check_no_floppy_media_contract();
    check_cmos_seed_decode_contract();
    probe = (construction_probe) {0};
    construction = prepare(&probe);
    const x86_nxvm_machine_config missing_media = {
        .floppy_image = {"missing-construction-medium.img"}
    };
    lib_test_assert(x86_nxvm_machine_create(&missing_media, &construction, &machine) ==
        LIB_STATUS_IO_ERROR);
    lib_test_assert(machine == LIB_NULL && probe.release_count == 1u);
    probe = (construction_probe) {0};
    construction = prepare(&probe);
    construction.media_kind = (x86_nxvm_board_floppy_kind)99;
    lib_test_assert(x86_nxvm_machine_create(&runtime, &construction, &machine) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(machine == LIB_NULL && probe.release_count == 1u);
    return 0;
}
