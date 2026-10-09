#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/machine_board_state.h"
#include "core/x86/machine_interface.h"

#include "core_machine_board_fixture.h"
#include "composition/planar_parity_fixture.h"

static lib_i32 planar_parity_shared_memory(void)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_planar_parity_observation observation;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    config.memory_bytes = 512u * 1024u;
    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) failed = 1;
    else if (core_machine_configure_planar_parity(board, &parity) != LIB_STATUS_OK) failed = 2;
    else if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK) failed = 3;
    else if (core_machine_reset(machine) != LIB_STATUS_OK) failed = 4;
    if (!failed) failed |= test_planar_parity_memory_fault(machine);
    if (!failed) failed |= core_machine_get_planar_parity_observation(board,
        &observation) != LIB_STATUS_OK || !observation.latched ||
        !observation.nmi_signaled;
    core_machine_destroy(machine);
    return failed;
}

static lib_status planar_conflict_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    (void)tick;
    (void)owner;
    if (port != CORE_MACHINE_PC_AT_PORT_B) return LIB_STATUS_INVALID_ARGUMENT;
    *value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_i32 planar_parity_publication_rollback(void)
{
    const core_machine_config config = {.memory_bytes = 512u * 1024u};
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B, .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const core_machine_port_route conflict = {
        .address = CORE_MACHINE_PC_AT_PORT_B, .read = planar_conflict_read,
        .owner = &machine
    };
    lib_i32 failed = 1;

    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) goto done;
    if (core_machine_install_port_routes(machine, &conflict, 1u) != LIB_STATUS_OK ||
        core_machine_configure_planar_parity(board, &parity) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        test_planar_parity_memory_binding(machine, LIB_NULL) ||
        board->planar_parity != LIB_NULL ||
        core_machine_remove_port_routes(machine, &machine) != LIB_STATUS_OK ||
        core_machine_configure_planar_parity(board, &parity) != LIB_STATUS_OK ||
        test_planar_parity_memory_binding(machine, board->planar_parity) ||
        board->planar_parity == LIB_NULL) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_rtc_cmos_config cmos = {0};
    core_machine_planar_parity_observation observation;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 1u;
    cmos.index_port = 0x0070u;
    cmos.data_port = 0x0071u;
    cmos.irq = 8u;
    cmos.nmi_mask_bit = 0x80u;
    cmos.ticks_per_second = 1u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_planar_parity(board, &parity) != LIB_STATUS_OK ||
        core_machine_configure_rtc_cmos(board, &cmos) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x94u) != 0x14u ||
        (x86_pit_advance(board->shared_pit, 19u),
         core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK) ||
        (value & 0x10u) != 0u ||

        core_machine_bus_write(machine, 0x0070u, 0x80u) != LIB_STATUS_OK ||
        core_machine_report_planar_parity_fault(board) != LIB_STATUS_OK ||
        core_machine_get_planar_parity_observation(board, &observation) !=
            LIB_STATUS_OK || !observation.latched || observation.nmi_signaled ||
        core_machine_bus_write(machine, 0x0070u, 0u) != LIB_STATUS_OK ||
        core_machine_get_planar_parity_observation(board, &observation) !=
            LIB_STATUS_OK || !observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x84u) != 0x84u ||
        core_machine_bus_write(machine, 0x0061u, 0xc4u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0xc0u) != 0x80u ||
        core_machine_bus_write(machine, 0x0061u, 0u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0061u, 0x04u) != LIB_STATUS_OK ||
        core_machine_get_planar_parity_observation(board, &observation) !=
            LIB_STATUS_OK || observation.latched || observation.nmi_signaled ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_planar_parity_observation(board, &observation) !=
            LIB_STATUS_OK || !observation.enabled || observation.latched;
    core_machine_destroy(machine);
    if (failed) return 1;
    failed |= planar_parity_shared_memory() || test_planar_parity_unbound_reconfigure() ||
        planar_parity_publication_rollback();
    if (failed) return 1;
    lib_c_printf("PLANAR-MEMORY-PARITY:OK\n");
    return 0;
}
