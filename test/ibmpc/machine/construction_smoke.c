#include "lib/types/test.h"
#include "ibmpc/machine/machine_interface.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/media/media_interface.h"

typedef struct construction_probe {
    lib_status configure_status;
    lib_status reset_status;
    lib_size configure_count;
    lib_size firmware_count;
    lib_size reset_count;
    lib_size reset_notice_count;
    lib_size detach_count;
    lib_size release_count;
    lib_bool released;
} construction_probe;

static lib_status configure_plan(void *context, core_machine_plan *plan)
{
    construction_probe *probe = context;

    lib_test_assert(plan != LIB_NULL && !probe->released);
    ++probe->configure_count;
    return probe->configure_status;
}

static lib_status configure_firmware(void *context,
    core_machine_firmware_context *firmware)
{
    construction_probe *probe = context;
    const lib_u8 reset_code[16] = {0xf4u};

    lib_test_assert(!probe->released);
    ++probe->firmware_count;
    return core_machine_firmware_register_immutable_rom(firmware,
        0xffff0u, reset_code, sizeof(reset_code));
}

static lib_status reset_firmware(void *context,
    core_machine_firmware_context *firmware)
{
    construction_probe *probe = context;

    lib_test_assert(firmware != LIB_NULL && !probe->released);
    ++probe->reset_count;
    return probe->reset_status;
}

static void notify_profile(void *context, vm_machine_profile_event event)
{
    construction_probe *probe = context;

    lib_test_assert(!probe->released);
    if (event == VM_MACHINE_PROFILE_RESET_COMPLETED)
        ++probe->reset_notice_count;
    else {
        lib_test_assert(event == VM_MACHINE_PROFILE_BOARD_DETACHED);
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

static const core_machine_firmware_provider firmware = {
    configure_firmware, reset_firmware, LIB_NULL
};

static vm_machine_construction prepare(construction_probe *probe)
{
    vm_machine_construction value = {
        .core_config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_8042,
            .shared_pit_personality = X86_PIT_PERSONALITY_8254
        },
        .topology = {
            .dma_present = LIB_TRUE,
            .dma = {2u, CORE_MACHINE_DMA_CONTROLLER_COUNT,
                CORE_MACHINE_DMA_CASCADE_CHANNEL},
            .fdc_present = LIB_TRUE,
            .fdc_drives = {
                .media_id = {VM_MACHINE_MEDIA_FDD_ID},
                .installed_mask = 1u,
                .double_sided_mask = 1u,
                .cylinder_count = {80u}
            },
            .fdc = { .dor_port = 0x3f2u, .status_port = 0x3f4u,
                .data_port = 0x3f5u, .irq = 6u, .dma_channel = 2u,
                .clock_ticks_per_second = 1000000u }
        },
        .floppy_slot_count = 1u,
        .firmware_provider = &firmware,
        .firmware_context = probe,
        .profile = {probe, configure_plan, notify_profile, release_profile}
    };
    return value;
}

static void check_transaction(lib_status configure_status,
    lib_status reset_status)
{
    construction_probe probe = {
        .configure_status = configure_status, .reset_status = reset_status
    };
    vm_machine_construction construction = prepare(&probe);
    const vm_machine_config runtime = {0};
    vm_machine *machine = LIB_NULL;
    common_machine_driver driver;
    vm_machine_information information;
    lib_status status = vm_machine_create(&runtime, &construction, &machine);

    lib_test_assert(probe.configure_count == 1u);
    if (configure_status != LIB_STATUS_OK || reset_status != LIB_STATUS_OK) {
        lib_test_assert(status != LIB_STATUS_OK && machine == LIB_NULL);
        lib_test_assert(probe.released && probe.release_count == 1u);
        lib_test_assert(probe.detach_count == 1u);
        lib_test_assert(probe.reset_notice_count == 0u);
        return;
    }
    lib_test_assert(status == LIB_STATUS_OK && machine != LIB_NULL);
    lib_test_assert(!probe.released && probe.firmware_count == 1u);
    lib_test_assert(probe.reset_count == 1u && probe.reset_notice_count == 1u);
    /* Caller storage may change: the published owner consumed copied values. */
    construction.core_config.memory_bytes = 0u;
    construction.floppy_slot_count = 0u;
    lib_test_assert(vm_machine_get_information(machine, &information) == LIB_STATUS_OK);
    lib_test_assert(information.memory_bytes == CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    lib_test_assert(vm_machine_describe_common_driver(machine, &driver) == LIB_STATUS_OK);
    lib_test_assert(driver.context == machine && driver.reset(driver.context));
    lib_test_assert(probe.configure_count == 1u && probe.firmware_count == 1u);
    lib_test_assert(probe.reset_count == 2u && probe.reset_notice_count == 2u);
    vm_machine_destroy(machine);
    lib_test_assert(probe.released && probe.release_count == 1u);
    lib_test_assert(probe.detach_count == 1u);
}

lib_i32 main(void)
{
    construction_probe probe = {0};
    vm_machine_construction construction = prepare(&probe);
    const vm_machine_config runtime = {0};
    vm_machine *machine = LIB_NULL;

    lib_test_assert(vm_machine_create(LIB_NULL, &construction, &machine) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(vm_machine_create(&runtime, &construction, LIB_NULL) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(!probe.released && probe.configure_count == 0u);
    check_transaction(LIB_STATUS_OK, LIB_STATUS_OK);
    check_transaction(LIB_STATUS_INVALID_ARGUMENT, LIB_STATUS_OK);
    check_transaction(LIB_STATUS_OK, LIB_STATUS_INVALID_STATE);
    probe = (construction_probe) {0};
    construction = prepare(&probe);
    const vm_machine_config missing_media = {
        .floppy_image = {"missing-construction-medium.img"}
    };
    lib_test_assert(vm_machine_create(&missing_media, &construction, &machine) ==
        LIB_STATUS_IO_ERROR);
    lib_test_assert(machine == LIB_NULL && probe.release_count == 1u);
    probe = (construction_probe) {0};
    construction = prepare(&probe);
    construction.media_kind = (vm_profile_floppy_kind)LIB_UINT32_MAX;
    lib_test_assert(vm_machine_create(&runtime, &construction, &machine) ==
        LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(machine == LIB_NULL && probe.release_count == 1u);
    return 0;
}
