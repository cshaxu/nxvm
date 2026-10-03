#include "support/board_construction_fixture.h"
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "x86/core/debug_interface.h"
#include <stdio.h>

#include "x86/core/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "x86/ibmpc-common/media_interface.h"

static lib_bool fail_fdc_create;
static lib_status port_assembly_create_fdc(const x86_fdc_connection *connection,
    const x86_fdc_timing *timing, x86_fdc **out_fdc)
{
    if (fail_fdc_create) {
        *out_fdc = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_fdc_create(connection, timing, out_fdc);
}

/* Inject a failed dependency at the adapter's existing create boundary. */
#define x86_fdc_create port_assembly_create_fdc
#include "app-nxvm/devices/fdc.c"
#undef x86_fdc_create

static lib_bool fail_hdc_create;
static lib_status port_assembly_create_hdc(const x86_hdc_config *config,
    const x86_hdc_connection *connection, x86_hdc **out_hdc)
{
    if (fail_hdc_create) {
        *out_hdc = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_hdc_create(config, connection, out_hdc);
}

#define x86_hdc_create port_assembly_create_hdc
#include "app-nxvm/devices/hdc.c"
#undef x86_hdc_create

typedef struct port_assembly_probe_state {
    lib_u32 value;
    lib_u64 read_tick;
    lib_u32 reads;
    lib_bool inconsistent_tick;
} port_assembly_probe_state;

static lib_status port_assembly_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (void)port;
    if (state == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (state->reads != 0u && state->read_tick != tick)
        state->inconsistent_tick = LIB_TRUE;
    state->read_tick = tick;
    ++state->reads;
    *out_value = state->value;
    return LIB_STATUS_OK;
}

static lib_status port_assembly_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (void)port;
    if (state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    state->value = value;
    return LIB_STATUS_OK;
}

static lib_i32 port_assembly_fresh_default_create(void)
{
    core_machine_config config = {0};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_range_transaction(void)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_port_provider provider = { port_assembly_read, port_assembly_write };
    core_machine_port_test_allocation allocation = { 3u, 0u };
    port_assembly_probe_state state = {0u};
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_NO_MEMORY;
        failed |= core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00e1u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e1u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_OK;
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e1u,
            &provider, &state) != LIB_STATUS_INVALID_STATE;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x00e0u, 0x5au) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e0u, &value) != LIB_STATUS_OK ||
            value != 0x5au;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_batch_transaction(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES};
    const core_machine_port_provider existing = {
        port_assembly_read, port_assembly_write};
    port_assembly_probe_state state = {0u};
    core_machine_port_test_allocation allocation = {3u, 0u};
    core_machine_port_route routes[] = {
        {0x00e2u, port_assembly_read, port_assembly_write, &state, LIB_FALSE, 0u},
        {0x00e4u, port_assembly_read, port_assembly_write, &state, LIB_FALSE, 0u}
    };
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    if (!failed) {
        failed |= core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &existing, &state) != LIB_STATUS_OK;
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_NO_MEMORY;
        failed |= !core_machine_port_has_read(&machine->executor_port, 0x00e0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00e2u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e2u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00e4u) ||
            core_machine_port_has_write(&machine->executor_port, 0x00e4u);
        allocation.fail_at = 0u;
        failed |= core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_OK;
        failed |= core_machine_install_port_routes(machine, routes, 2u) !=
            LIB_STATUS_INVALID_STATE;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x00e4u, 0x3cu) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x00e2u, &value) != LIB_STATUS_OK ||
            value != 0x3cu;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_read_time(void)
{
    t_port ports = {0};
    port_assembly_probe_state lane = {.value = 0x5au};
    port_assembly_probe_state contributor = {.value = 0x80u};
    const core_machine_port_route bank = {
        .address = 0x100u, .read = port_assembly_read, .owner = &lane,
        .byte_lane_end = 0x104u
    };
    const core_machine_port_route wired_or = {
        .address = 0x100u, .read = port_assembly_read, .owner = &contributor,
        .wired_or_read = LIB_TRUE
    };
    const lib_u64 tick = 0x100000003ull;
    lib_i32 failed;

    core_machine_port_initialize(&ports);
    failed = core_machine_port_add_route(&ports, &bank) != LIB_STATUS_OK ||
        core_machine_port_add_route(&ports, &wired_or) != LIB_STATUS_OK ||
        core_machine_port_execute_read_width(&ports, 0x100u, 4u, tick) !=
            LIB_STATUS_OK || ports.data.ioDWord != 0x5a5a5adau ||
        lane.reads != 4u || contributor.reads != 1u ||
        lane.read_tick != tick || contributor.read_tick != tick ||
        lane.inconsistent_tick || contributor.inconsistent_tick;
    /* The existing fixture convenience is explicitly zero-time, not live time. */
    lane.reads = contributor.reads = 0u;
    failed |= core_machine_port_read(&ports, 0x100u) != 0xdau ||
        lane.read_tick != 0u || contributor.read_tick != 0u;
    core_machine_port_finalize(&ports);
    return failed;
}

static lib_i32 port_assembly_port_b_time(void)
{
    const core_machine_config config = {.memory_bytes = 512u * 1024u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086};
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B, .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
        .refresh_status_toggle_ticks = 64u
    };
    const lib_u64 ticks[] = {0u, 63u, 64u, 65u, 127u, 128u, 129u,
        0x100000040ull};
    const lib_u8 code[] = {0x90u, 0xe4u, 0x61u};
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {0}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_time_observation time = {0};
    lib_u32 eax = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) failed = core_machine_configure_planar_parity(board, &parity) !=
        LIB_STATUS_OK;
    for (lib_size index = 0u; !failed && index < sizeof(ticks) / sizeof(ticks[0]);
            ++index) {
        /* Core time remains zero: the endpoint must use the supplied value. */
        failed = core_machine_port_execute_read(&machine->executor_port,
            CORE_MACHINE_PC_AT_PORT_B, ticks[index]) != LIB_STATUS_OK ||
            (machine->executor_port.data.ioDWord & 0x10u) !=
                (((ticks[index] / 64u) & 1u) ? 0x10u : 0u) ||
            machine->elapsed_ticks != 0u;
    }
    if (!failed) failed = core_machine_freeze_execution_providers(machine) !=
            LIB_STATUS_OK || core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks == 0u ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX, &eax) !=
            LIB_STATUS_OK ||
        (eax & 0x10u) != (((time.elapsed_ticks / 64u) & 1u) ? 0x10u : 0u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_dma_byte_lanes(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    if (!failed) {
        t_port *port = &machine->executor_port;

        port->data.ioDWord = 0x44332211u;
        failed |= core_machine_port_execute_write_width(port, 0x0081u, 4u) !=
            LIB_STATUS_OK;
        failed |= (lib_u8)core_machine_port_read(port, 0x0081u) != 0x11u ||
            (lib_u8)core_machine_port_read(port, 0x0082u) != 0x22u ||
            (lib_u8)core_machine_port_read(port, 0x0083u) != 0x33u ||
            (lib_u8)core_machine_port_read(port, 0x0084u) != 0x44u;
        failed |= core_machine_port_execute_read_width(port, 0x0081u, 4u, 0u) !=
            LIB_STATUS_OK || port->data.ioDWord != 0x44332211u;
        core_machine_port_write(port, 0x008fu, 0x5au);
        port->data.ioDWord = 0x88776655u;
        failed |= core_machine_port_execute_write_width(port, 0x008eu, 4u) !=
            LIB_STATUS_OK;
        failed |= (lib_u8)core_machine_port_read(port, 0x008eu) != 0x55u ||
            (lib_u8)core_machine_port_read(port, 0x008fu) != 0x5au;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_create_failure(void)
{
    const core_machine_config variants[] = {
        {.memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES},
        {.memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .auxiliary_pit_present = LIB_TRUE, .auxiliary_pit_base_port = 0x48u},
        {.memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
            .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u},
            .dma_controller_count = 1u, .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_SINGLE}
    };

    for (lib_size variant = 0u; variant < sizeof(variants) / sizeof(variants[0]); ++variant) {
        core_machine_port_test_allocation allocation = {0u, 0u};
        core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        lib_status status = test_core_machine_create_with_allocation(
            &variants[variant], &machine,
            LIB_NULL, &allocation, &board);
        lib_size count = allocation.attempts;
        lib_i32 failed = status != LIB_STATUS_OK || machine == LIB_NULL || count == 0u;
        failed |= board == LIB_NULL || (!failed &&
            (machine->attachment.context != board || board->core != machine));

        core_machine_destroy(machine);
        if (failed) return 1;
        for (lib_size fail_at = 1u; fail_at <= count + 1u; ++fail_at) {
            allocation = (core_machine_port_test_allocation) {fail_at, 0u};
            machine = LIB_NULL;
            board = (core_machine_board_state *)(lib_uptr)1u;
            status = test_core_machine_create_with_allocation(
                &variants[variant], &machine,
                LIB_NULL, &allocation, &board);
            failed = fail_at <= count ?
                status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL || board != LIB_NULL ||
                    allocation.attempts != fail_at :
                status != LIB_STATUS_OK || machine == LIB_NULL || board == LIB_NULL ||
                    allocation.attempts != count;
            core_machine_destroy(machine);
            if (failed) return 1;
        }
    }
    return port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_fdc_transaction(lib_size fail_at)
{
    const core_machine_config config = { .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES };
    const core_machine_dma_wiring wiring = { .fdc_channel = 2u,
        .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
        .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding request = {0};
    core_machine_fdc_topology topology = {
        .media_registry = LIB_NULL,
        .drives = {{1u, CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID}},
        .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
            .data_port = 0x03f5u, .direction_port = 0x03f7u,
            .control_port = 0x03f7u, .diagnostic_port = 0x03f0u,
            .irq = 6u, .dma_channel = 2u}
    };
    core_machine_port_test_allocation allocation = { fail_at, 0u };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_fdc fdc_zero = {0};
    core_machine_fdc_topology topology_zero = {0};
    lib_i32 failed = 0;

    failed |= core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_dma(board, &wiring, &request) != LIB_STATUS_OK;
    topology.media_registry = media;
    topology.dma_request = request;
    if (!failed) {
        fail_fdc_create = fail_at == 0u;
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_fdc(board, &topology) != LIB_STATUS_NO_MEMORY ||
            board->fdc_configured ||
            lib_memory_compare(&board->fdc, &fdc_zero, sizeof(fdc_zero)) != 0 ||
            lib_memory_compare(&board->fdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x03f4u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f5u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f7u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f2u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f5u) ||
            core_machine_port_has_write(&machine->executor_port, 0x03f7u) ||
            core_machine_dma_has_pending_request(&board->shared_dma_primary,
                &board->shared_dma_secondary);
        failed |= fail_fdc_create && allocation.attempts != 0u;
        fail_fdc_create = LIB_FALSE;
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_fdc(board, &topology) != LIB_STATUS_OK ||
            !board->fdc_configured;
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_rtc_transaction(lib_size fail_at)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    const core_machine_rtc_cmos_config rtc_config = {
        .index_port = 0x0070u, .data_port = 0x0071u, .nmi_mask_bit = 0x80u,
        .irq = 8u, .ticks_per_second = 1000u, .default_count = 1u,
        .defaults = {{CORE_MACHINE_RTC_EQUIPMENT, 0x2fu}}
    };
    core_machine_port_test_allocation allocation = { fail_at, 0u };
    core_machine_rtc_cmos_config config_zero = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) !=
                LIB_STATUS_NO_MEMORY || board->rtc_cmos_configured ||
            board->shared_rtc != LIB_NULL ||
            lib_memory_compare(&board->rtc_cmos_config, &config_zero,
                sizeof(config_zero)) != 0 ||
            core_machine_port_has_write(&machine->executor_port, 0x0070u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0071u) ||
            core_machine_port_has_write(&machine->executor_port, 0x0071u);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) !=
            LIB_STATUS_OK || !board->rtc_cmos_configured;
    }
    core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_rtc_collision(void)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    const core_machine_rtc_cmos_config rtc_config = {
        .index_port = 0x0070u, .data_port = 0x0071u, .nmi_mask_bit = 0x80u,
        .irq = 8u, .ticks_per_second = 1000u
    };
    const core_machine_port_route existing = {
        .address = 0x0071u, .read = port_assembly_read,
        .write = port_assembly_write
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        failed |= core_machine_install_port_routes(machine, &existing, 1u) != LIB_STATUS_OK;
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) !=
                LIB_STATUS_INVALID_STATE || board->shared_rtc != LIB_NULL ||
            board->rtc_cmos_configured ||
            core_machine_port_has_write(&machine->executor_port, 0x0070u) ||
            !core_machine_port_has_read(&machine->executor_port, 0x0071u) ||
            !core_machine_port_has_write(&machine->executor_port, 0x0071u);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_refresh_count(core_machine_board_state *board)
{
    x86_pit *pit = board->shared_pit;
    lib_u8 low = 0u, high = 0u;

    /* The reload becomes observable on the next input-clock cycle. */
    x86_pit_advance(pit, 1u);
    return x86_pit_write_register(pit, 3u, 0x40u) != LIB_STATUS_OK ||
        x86_pit_read_counter(pit, 1u, &low) != LIB_STATUS_OK ||
        x86_pit_read_counter(pit, 1u, &high) != LIB_STATUS_OK ||
        low != 18u || high != 0u;
}

static lib_i32 port_assembly_port_b_transaction(lib_bool d4, lib_size fail_at)
{
    const core_machine_config config = {
        .memory_bytes = 512u * 1024u, .auxiliary_pit_present = LIB_TRUE,
        .auxiliary_pit_base_port = 0x0048u
    };
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B, .memory_bytes = 512u * 1024u,
        .refresh_status_source =
            CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1
    };
    const core_machine_d4_platform_config d4_config = {
        CORE_MACHINE_PC_AT_PORT_B, 0u
    };
    core_machine_port_test_allocation allocation = {fail_at, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= (d4 ? core_machine_configure_d4_platform(board, &d4_config) :
            core_machine_configure_planar_parity(board, &parity)) !=
                LIB_STATUS_NO_MEMORY ||
            board->planar_parity_configured ||
            board->d4_platform_configured ||
            machine->executor_memory.connect.parity != 0u ||
            board->planar_parity_port_b != 0u ||
            board->d4_platform_port_b != 0u ||
            core_machine_port_has_read(&machine->executor_port, CORE_MACHINE_PC_AT_PORT_B) ||
            core_machine_port_has_write(&machine->executor_port, CORE_MACHINE_PC_AT_PORT_B);
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        failed |= (d4 ? core_machine_configure_d4_platform(board, &d4_config) :
            core_machine_configure_planar_parity(board, &parity)) != LIB_STATUS_OK ||
            !core_machine_port_has_read(&machine->executor_port,
                CORE_MACHINE_PC_AT_PORT_B) ||
            !core_machine_port_has_write(&machine->executor_port,
                CORE_MACHINE_PC_AT_PORT_B);
        if (!failed) failed |= port_assembly_refresh_count(board) ||
            core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->executor_port.data.ioDWord != 0u ||
            port_assembly_refresh_count(board) ||
            core_machine_bus_write(machine, 0x0043u, 0x74u) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x0041u, 2u) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x0041u, 0u) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->executor_port.data.ioDWord != 0u ||
            port_assembly_refresh_count(board);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_assembly_hdc_transaction(core_machine_hdc_protocol protocol,
    lib_size fail_at, lib_bool busy_dma, lib_bool busy_port)
{
    const core_machine_config machine_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine_media_registry *media = LIB_NULL;
    core_machine_hdc_topology topology = {
        .media_registry = LIB_NULL, .media_id = 1u,
        .config = {.protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO, .irq = 14u,
            .bus.task_file = {
                .data_port = 0x01f0u, .error_features_port = 0x01f1u,
                .sector_count_port = 0x01f2u, .sector_number_port = 0x01f3u,
                .cylinder_low_port = 0x01f4u, .cylinder_high_port = 0x01f5u,
                .drive_head_port = 0x01f6u, .status_command_port = 0x01f7u,
                .alternate_status_device_control_port = 0x03f6u,
                .lba28_supported = LIB_TRUE}}
    };
    core_machine_port_test_allocation allocation = {fail_at, 0u};
    core_machine_hdc hdc_zero = {0};
    core_machine_hdc_topology topology_zero = {0};
    const core_machine_dma_channel_provider blocker = {0};
    core_machine_dma_request_binding blocker_binding = {0};
    port_assembly_probe_state conflict = {0};
    const core_machine_port_route conflict_route = {
        .address = 0x01f3u, .read = port_assembly_read, .owner = &conflict};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const lib_bool xebec = protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
    const lib_bool compaq = protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
    const lib_u16 first_port = xebec ? 0x0320u : 0x01f0u;
    const lib_u16 last_port = xebec ? 0x0323u : 0x01f7u;
    lib_u32 fdc_value = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;
    topology.media_registry = media;
    topology.config.protocol = protocol;
    topology.config.bus.task_file.lba28_supported =
        protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO;
    topology.config.bus.task_file.clock_ticks_per_second = 8000000u;
    if (!failed && (xebec || compaq)) {
        const core_machine_dma_wiring wiring = {.fdc_channel = 2u,
            .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
            .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL};
        core_machine_fdc_topology fdc = {.media_registry = media,
            .drives = {{1u, 0u, 0u, 0u}},
            .config = {.dor_port = 0x03f2u, .status_port = 0x03f4u,
                .data_port = 0x03f5u, .direction_port = 0x03f7u,
                .control_port = 0x03f7u, .irq = 6u, .dma_channel = 2u}};
        failed |= core_machine_configure_dma(board, &wiring, &fdc.dma_request) != LIB_STATUS_OK;
        if (compaq) {
            topology.config.bus.task_file.drive_address_port = 0x03f7u;
            failed |= core_machine_configure_fdc(board, &fdc) != LIB_STATUS_OK;
            if (!failed) {
                failed |= core_machine_port_execute_read(&machine->executor_port,
                    0x03f7u, 0u) != LIB_STATUS_OK;
                fdc_value = machine->executor_port.data.ioDWord;
            }
        } else {
            topology.config.irq = 5u;
            topology.config.bus.xebec = (core_machine_hdc_xebec_config){
                .data_port = 0x0320u, .hardware_status_reset_port = 0x0321u,
                .jumpers_select_port = 0x0322u, .dma_irq_mask_port = 0x0323u,
                .dma_channel = 3u, .drive_type = CORE_MACHINE_XEBEC_DRIVE_TYPE_2,
                .expected_media_geometry = {CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT,
                    CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR, CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS,
                    CORE_MACHINE_XEBEC_TYPE_2_HEADS, CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK}};
            if (busy_dma) failed |= core_machine_dma_bind_channel(
                &board->shared_dma_latch, &board->shared_dma_primary,
                &board->shared_dma_secondary, 3u, &blocker,
                &blocker_binding, &blocker_binding) != LIB_STATUS_OK;
        }
    }
    if (!failed) {
        if (busy_port) failed |= core_machine_install_port_routes(machine,
            &conflict_route, 1u) != LIB_STATUS_OK;
    }
    if (!failed) {
        fail_hdc_create = fail_at == 0u && !busy_dma && !busy_port;
        core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
        failed |= core_machine_configure_hdc(board, &topology) !=
                (busy_dma || busy_port ? LIB_STATUS_INVALID_STATE : LIB_STATUS_NO_MEMORY) ||
            board->hdc_configured ||
            lib_memory_compare(&board->hdc, &hdc_zero, sizeof(hdc_zero)) != 0 ||
            lib_memory_compare(&board->hdc_topology, &topology_zero,
                sizeof(topology_zero)) != 0 ||
            core_machine_port_has_read(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_write(&machine->executor_port, 0x01f0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x03f6u);
        for (lib_u16 port = first_port; port <= last_port; ++port)
            failed |= ((port != conflict_route.address || !busy_port) &&
                core_machine_port_has_read(&machine->executor_port, port)) ||
                core_machine_port_has_write(&machine->executor_port, port);
        if (busy_port) failed |= !core_machine_port_has_read(&machine->executor_port,
            conflict_route.address);
        failed |= core_machine_port_has_write(&machine->executor_port, 0x03f6u) ||
            (fail_hdc_create && allocation.attempts != 0u);
        if (compaq) failed |= !board->fdc_configured ||
            !core_machine_port_has_read(&machine->executor_port, 0x03f7u);
        if (xebec) failed |= board->hdc_dma_request.core_token != 0u ||
            core_machine_dma_has_pending_request(&board->shared_dma_primary,
                &board->shared_dma_secondary);
        if (busy_dma) failed |= blocker_binding.core_token == 0u ||
            board->shared_dma_primary.connect.device_owner[3] != &blocker_binding;
        fail_hdc_create = LIB_FALSE;
        allocation.fail_at = 0u;
        allocation.attempts = 0u;
        if (busy_port) failed |= core_machine_remove_port_routes(machine,
            &conflict) != LIB_STATUS_OK;
        if (!busy_dma) {
            failed |= core_machine_configure_hdc(board, &topology) != LIB_STATUS_OK ||
                !board->hdc_configured;
            if (compaq && !failed) {
                lib_u32 hdc_value = 0u;
                lib_u32 combined = 0u;

                failed |= core_machine_hdc_port_provider()->read(&board->hdc,
                    0x03f7u, 0u, &hdc_value) != LIB_STATUS_OK ||
                    core_machine_port_execute_read(&machine->executor_port,
                        0x03f7u, 0u) != LIB_STATUS_OK;
                combined = machine->executor_port.data.ioDWord;
                failed |= combined != (fdc_value | hdc_value);
            }
            if (xebec) failed |= core_machine_port_has_read(&machine->executor_port,
                0x0323u) || !core_machine_port_has_write(&machine->executor_port, 0x0323u);
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) fprintf(stderr, "HDC rollback protocol=%u fail_at=%u\n",
        (lib_u32)protocol, (lib_u32)fail_at);
    return failed || port_assembly_fresh_default_create();
}

static lib_i32 port_assembly_pit_transaction(void)
{
    lib_i32 failed = 0;
    for (lib_u32 fail_at = 1u; fail_at <= 7u; ++fail_at) {
        core_machine machine = {0};
        t_port *ports = &machine.executor_port;
        x86_pit *pit = LIB_NULL;
        core_machine_port_test_allocation allocation = {fail_at, 0u};
        machine.lifecycle = CORE_MACHINE_INITIALIZED;
        core_machine_port_initialize(ports);
        core_machine_port_set_test_allocation(ports, &allocation);
        if (x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK) {
            core_machine_port_finalize(ports);
            return 1;
        }
        failed |= core_machine_pit_install_ports(&machine, pit, 0x0048u) !=
            LIB_STATUS_NO_MEMORY;
        /* A failed port publication does not destroy the caller-owned chip. */
        failed |= x86_pit_write_register(pit, 3u, 0x34u) != LIB_STATUS_OK;
        for (lib_u16 port = 0x0048u; port <= 0x004bu; ++port) {
            failed |= core_machine_port_has_read(ports, port) ||
                core_machine_port_has_write(ports, port);
        }
        x86_pit_destroy(pit);
        core_machine_port_finalize(ports);
    }
    return failed;
}

static lib_i32 port_assembly_pic_transaction(void)
{
    const lib_u16 addresses[] = {0x20u, 0x21u, 0xa0u, 0xa1u};
    lib_i32 failed = 0;
    for (lib_u32 fail_at = 1u; fail_at <= 8u; ++fail_at) {
        core_machine machine = {0};
        t_port *ports = &machine.executor_port;
        core_machine_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
        core_machine_port_test_allocation allocation = {fail_at, 0u};
        machine.lifecycle = CORE_MACHINE_INITIALIZED;
        core_machine_port_initialize(ports);
        core_machine_port_set_test_allocation(ports, &allocation);
        failed |= core_machine_pic_initialize(&master, &slave, &machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_NO_MEMORY ||
            master != LIB_NULL || slave != LIB_NULL;
        for (lib_size index = 0u; index < sizeof(addresses) / sizeof(addresses[0]); ++index) {
            failed |= core_machine_port_has_read(ports, addresses[index]) ||
                core_machine_port_has_write(ports, addresses[index]);
        }
        allocation.fail_at = 0u;
        failed |= core_machine_pic_initialize(&master, &slave, &machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK;
        core_machine_pic_finalize(master, slave);
        core_machine_port_finalize(ports);
    }
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = port_assembly_range_transaction() ||
        port_assembly_batch_transaction() ||
        port_assembly_dma_byte_lanes() || port_assembly_read_time() ||
        port_assembly_port_b_time() ||
        port_assembly_create_failure() ||
        port_assembly_pit_transaction() || port_assembly_pic_transaction();

    /* Chip creation, then all seven routes, including the optional diagnostic. */
    for (lib_size fail_at = 0u; fail_at <= 7u; ++fail_at)
        failed |= port_assembly_fdc_transaction(fail_at);

    if (failed) return 1;
    puts("M5:T313:S3:PORT-ASSEMBLY:OK");

    failed = port_assembly_rtc_transaction(1u) || port_assembly_rtc_transaction(2u) ||
        port_assembly_rtc_collision();
    for (lib_u32 d4 = 0u; d4 <= 1u; ++d4)
        for (lib_size fail_at = 1u; fail_at <= 2u; ++fail_at)
            failed |= port_assembly_port_b_transaction((lib_bool)d4, fail_at);
    for (lib_u32 protocol = CORE_MACHINE_HDC_PROTOCOL_ATA_PIO;
        protocol <= CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT; ++protocol) {
        const lib_size routes = protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT ? 7u :
            protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB ? 19u : 18u;
        for (lib_size fail_at = 0u; fail_at <= routes; ++fail_at)
            failed |= port_assembly_hdc_transaction((core_machine_hdc_protocol)protocol,
                fail_at, LIB_FALSE, LIB_FALSE);
    }
    failed |= port_assembly_hdc_transaction(CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT,
        0u, LIB_TRUE, LIB_FALSE);
    failed |= port_assembly_hdc_transaction(CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
        0u, LIB_FALSE, LIB_TRUE);

    if (failed) return 1;
    puts("M5:T313:S4:CONTROLLER-ROLLBACK:OK");
    return 0;
}
