#include "lib/types/file.h"
#include "construction_fixture.h"
#include "../board_binding_fixture.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/x86/machine.h"

static lib_i32 ram_create_success(lib_size memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_size installed_bytes = 0u;
    lib_i32 failed = 0;

    failed |= test_core_machine_create_with_allocation(&config, &machine,
        &allocation, LIB_NULL, &board) != LIB_STATUS_OK;
    failed |= machine == LIB_NULL || allocation.attempts != 1u;
    failed |= board == LIB_NULL || (!failed &&
        (machine->attachment.context != board));
    failed |= !failed && core_machine_get_memory_bytes(machine, &installed_bytes) !=
        LIB_STATUS_OK;
    failed |= !failed && installed_bytes != (memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : memory_bytes);
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 ram_create_failure(lib_size memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = { LIB_TRUE, 0u };
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = (core_machine_board_state *)(lib_uptr)1u;
    lib_status status = test_core_machine_create_with_allocation(
        &config, &machine,
        &allocation, LIB_NULL, &board);

    return status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL || board != LIB_NULL ||
        allocation.attempts != 1u;
}

static lib_i32 configuration_board_publication(void)
{
    const core_machine_config configurations[] = {
        {0},
        {.auxiliary_pit_present = LIB_TRUE, .auxiliary_pit_base_port = 0x48u},
        {.keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
            .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u},
            .dma_controller_count = 1u, .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_SINGLE}
    };
    lib_i32 failed = 0;

    for (lib_size i = 0u; i < sizeof(configurations) / sizeof(configurations[0]); ++i) {
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        core_machine_attachment expected;
        lib_status status = test_board_binding_create(&configurations[i], &machine, &expected);
        if (status == LIB_STATUS_OK) board = expected.context;

        failed |= status != LIB_STATUS_OK || machine == LIB_NULL || board == LIB_NULL;
        if (status == LIB_STATUS_OK && machine != LIB_NULL && board != LIB_NULL) {
            failed |= machine->attachment.context != board;
        }
        core_machine_destroy(machine);
    }
    return failed;
}

static lib_i32 ram_fixture_retained(void)
{
    t_ram ram = {0};
    lib_u8 value = 0x5au;
    lib_u8 observed = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_memory_initialize_for(&ram,
        CORE_MACHINE_DEFAULT_MEMORY_BYTES, LIB_NULL) != LIB_STATUS_OK;
    failed |= ram.connect.installed_bytes != CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    failed |= core_machine_memory_allocate_for(&ram,
        CORE_MACHINE_MINIMUM_MEMORY_BYTES) != LIB_STATUS_OK;
    failed |= ram.connect.installed_bytes != CORE_MACHINE_MINIMUM_MEMORY_BYTES;
    failed |= core_machine_memory_write_physical(&ram, 0u,
        (lib_uptr)&value, sizeof(value)) != LIB_STATUS_OK;
    failed |= core_machine_memory_read_physical(&ram, 0u,
        (lib_uptr)&observed, sizeof(observed)) != LIB_STATUS_OK;
    failed |= observed != value;
    core_machine_memory_finalize(&ram);
    return failed;
}

static lib_i32 ram_create_preflight(void)
{
    core_machine_config config = {0};
    core_machine_clock_ratio *ratios[] = {
        &config.clock_plan.provider, &config.clock_plan.dma,
        &config.clock_plan.pit, &config.clock_plan.auxiliary_pit,
        &config.clock_plan.rtc, &config.clock_plan.vadp,
        &config.clock_plan.kbc
    };
    lib_i32 failed = 0;

    for (lib_size i = 0u; i < sizeof(ratios) / sizeof(ratios[0]); ++i) {
        core_machine_memory_test_allocation allocation = {0};
        core_machine *machine = (core_machine *)(lib_uptr)1u;
        core_machine_board_state *board = (core_machine_board_state *)(lib_uptr)1u;
        ratios[i]->numerator = 1u;
        failed |= test_core_machine_create_with_allocation(&config,
            &machine,
            &allocation, LIB_NULL, &board) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= machine != LIB_NULL || board != LIB_NULL || allocation.attempts != 0u;
        if (machine != LIB_NULL && machine != (core_machine *)(lib_uptr)1u)
            core_machine_destroy(machine);
        ratios[i]->numerator = 0u;
    }
    {
        core_machine *machine = (core_machine *)(lib_uptr)1u;
        core_machine_board_state *board = (core_machine_board_state *)(lib_uptr)1u;
        failed |= core_machine_create(LIB_NULL, &machine, &board) !=
            LIB_STATUS_INVALID_ARGUMENT;
        failed |= machine != LIB_NULL || board != LIB_NULL;
        board = (core_machine_board_state *)(lib_uptr)1u;
        failed |= core_machine_create(&config, LIB_NULL, &board) !=
            LIB_STATUS_INVALID_ARGUMENT;
        failed |= board != LIB_NULL;
    }
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= ram_create_success(0u);
    failed |= ram_create_success(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_create_failure(0u);
    failed |= ram_create_failure(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_fixture_retained();
    failed |= ram_create_preflight();
    failed |= configuration_board_publication();
    if (failed) return 1;
    lib_c_printf("%s\n", "RAM-CREATE:OK");
    lib_c_printf("%s\n", "CONFIG-BOARD-PUBLICATION:OK");
    return 0;
}
