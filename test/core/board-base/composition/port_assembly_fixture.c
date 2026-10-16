#include "lib/types/file.h"
#include "port_assembly_fixture.h"
#include "bus_fixture.h"
#include "x86-core/machine/machine_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "core/board-base/pit_bus_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "../port_assembly_board_fixture.h"
#include "../board_binding_fixture.h"
#include "../../board-at/support/parity_registration_fixture.h"
lib_status port_assembly_read(void *owner, lib_u16 port, lib_u64 tick,
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

lib_status port_assembly_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    port_assembly_probe_state *state = (port_assembly_probe_state *)owner;

    (void)port;
    if (state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    state->value = value;
    return LIB_STATUS_OK;
}

lib_i32 port_assembly_fresh_default_create(void)
{
    core_machine_config config = {0};
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_port_b_time(void)
{
    const core_machine_config config = {.memory_bytes = 512u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086};
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B, .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
        .refresh_status_toggle_ticks = 64u
    };
    const lib_u8 code[] = {0x90u, 0xe4u, 0x61u};
    const x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {0}
    };
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_time_observation time = {0};
    lib_u32 eax = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) failed = core_machine_configure_planar_parity(board, &parity) !=
        LIB_STATUS_OK;
    if (!failed) failed = x86_core_machine_freeze_execution_providers(machine) !=
            LIB_STATUS_OK || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
        x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks == 0u ||
        x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EAX, &eax) !=
            LIB_STATUS_OK ||
        (eax & 0x10u) != (((time.elapsed_ticks / 64u) & 1u) ? 0x10u : 0u);
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_dma_byte_lanes(void)
{
    const core_machine_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const lib_u8 code[] = {
        0xbau, 0x81u, 0x00u,
        0x66u, 0xb8u, 0x11u, 0x22u, 0x33u, 0x44u,
        0x66u, 0xefu,
        0x66u, 0xedu,
        0xb0u, 0x5au, 0xe6u, 0x8fu,
        0xbau, 0x8eu, 0x00u,
        0x66u, 0xb8u, 0x55u, 0x66u, 0x77u, 0x88u,
        0x66u, 0xefu
    };
    const x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x200u}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    lib_u32 value = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;

    if (!failed) {
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x200u, code, sizeof(code)) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK;
        for (lib_size index = 0u; !failed && index < 3u; ++index)
            failed |= x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK;
        for (lib_u16 index = 0u; !failed && index < 4u; ++index)
            failed |= x86_core_machine_bus_read(machine, (lib_u16)(0x81u + index),
                &value) != LIB_STATUS_OK || (lib_u8)value != (lib_u8)(0x44332211u >> (8u * index));
        if (!failed) failed |= x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EAX, &value) !=
                LIB_STATUS_OK || value != 0x44332211u;
        for (lib_size index = 0u; !failed && index < 5u; ++index)
            failed |= x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK;
        if (!failed) failed |= x86_core_machine_bus_read(machine, 0x8eu, &value) !=
                LIB_STATUS_OK || (lib_u8)value != 0x55u ||
            x86_core_machine_bus_read(machine, 0x8fu, &value) != LIB_STATUS_OK ||
                (lib_u8)value != 0x5au;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_create_failure(void)
{
    const core_machine_config variants[] = {
        {.memory_bytes = (2u * 1024u * 1024u)},
        {.memory_bytes = (2u * 1024u * 1024u),
            .auxiliary_pit_present = LIB_TRUE, .auxiliary_pit_base_port = 0x48u},
        {.memory_bytes = (2u * 1024u * 1024u),
            .keyboard_topology = CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI,
            .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u},
            .dma_controller_count = 1u, .pic_topology = CORE_MACHINE_PIC_TOPOLOGY_SINGLE}
    };

    for (lib_size variant = 0u; variant < sizeof(variants) / sizeof(variants[0]); ++variant) {
        x86_core_machine *machine = LIB_NULL;
        core_machine_board_state *board = LIB_NULL;
        lib_i32 failed = core_machine_create(&variants[variant], &machine,
            &board) != LIB_STATUS_OK || machine == LIB_NULL || board == LIB_NULL;

        x86_core_machine_destroy(machine);
        if (failed) return 1;
        for (lib_u32 dependency = 1u; dependency <= 8u; ++dependency) {
            if ((dependency == 5u && variant == 2u) ||
                (dependency == 6u && variant != 2u) ||
                (dependency == 7u && variant != 1u)) continue;
            if (test_board_device_failure(&variants[variant], dependency)) return 1;
        }
    }
    return port_assembly_fresh_default_create();
}

lib_i32 port_assembly_pit_transaction(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    x86_core_machine *machine = LIB_NULL;
    x86_pit *pit = LIB_NULL;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK;

    if (!failed) {
        test_port_assembly_reject_ports(LIB_TRUE);
        failed |= core_machine_pit_install_ports(machine, pit, 0x48u) !=
            LIB_STATUS_NO_MEMORY || !test_port_assembly_ports_rejected(4u);
        test_port_assembly_reject_ports(LIB_FALSE);
        /* Failed publication does not destroy the caller-owned chip. */
        failed |= x86_pit_write_register(pit, 3u, 0x34u) != LIB_STATUS_OK;
        for (lib_u16 port = 0x48u; port <= 0x4bu; ++port)
            failed |= test_core_machine_fixture_expect_port(machine, port, LIB_FALSE, LIB_FALSE) ||
                test_core_machine_fixture_expect_port(machine, port, LIB_TRUE, LIB_FALSE);
        failed |= core_machine_pit_install_ports(machine, pit, 0x48u) != LIB_STATUS_OK;
        for (lib_u16 port = 0x48u; port <= 0x4bu; ++port)
            failed |= test_core_machine_fixture_expect_port(machine, port, LIB_FALSE, port != 0x4bu) ||
                test_core_machine_fixture_expect_port(machine, port, LIB_TRUE, LIB_TRUE);
        failed |= x86_core_machine_remove_port_routes(machine, pit) != LIB_STATUS_OK;
        for (lib_u16 port = 0x48u; port <= 0x4bu; ++port)
            failed |= test_core_machine_fixture_expect_port(machine, port, LIB_FALSE, LIB_FALSE) ||
                test_core_machine_fixture_expect_port(machine, port, LIB_TRUE, LIB_FALSE);
    }
    x86_pit_destroy(pit);
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_pic_transaction(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const lib_u16 addresses[] = {0x20u, 0x21u, 0xa0u, 0xa1u};
    x86_core_machine *machine = LIB_NULL;
    core_machine_pic_bus *master = LIB_NULL, *slave = LIB_NULL;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) {
        test_port_assembly_reject_ports(LIB_TRUE);
        failed |= core_machine_pic_initialize(&master, &slave, machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_NO_MEMORY ||
            master != LIB_NULL || slave != LIB_NULL ||
            !test_port_assembly_ports_rejected(4u);
        test_port_assembly_reject_ports(LIB_FALSE);
        for (lib_size index = 0u; index < sizeof(addresses) / sizeof(addresses[0]); ++index)
            failed |= test_core_machine_fixture_expect_port(machine, addresses[index], LIB_FALSE, LIB_FALSE) ||
                test_core_machine_fixture_expect_port(machine, addresses[index], LIB_TRUE, LIB_FALSE);
        failed |= core_machine_pic_initialize(&master, &slave, machine,
            CORE_MACHINE_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK;
        for (lib_size index = 0u; index < sizeof(addresses) / sizeof(addresses[0]); ++index)
            failed |= test_core_machine_fixture_expect_port(machine, addresses[index], LIB_FALSE, LIB_TRUE) ||
                test_core_machine_fixture_expect_port(machine, addresses[index], LIB_TRUE, LIB_TRUE);
        if (master != LIB_NULL)
            failed |= x86_core_machine_remove_port_routes(machine, master) != LIB_STATUS_OK;
        if (slave != LIB_NULL)
            failed |= x86_core_machine_remove_port_routes(machine, slave) != LIB_STATUS_OK;
    }
    core_machine_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_fdc_transaction(lib_bool fail_chip)
{
    const core_machine_config config = {.memory_bytes = 2u * 1024u * 1024u};
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_bus *dma = LIB_NULL;
    core_machine_dma_request_binding request = {0};
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    test_port_assembly_board_observation observed;
    lib_i32 failed = core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        test_port_assembly_fdc_setup(board, &dma, &request) != LIB_STATUS_OK;

    if (!failed) {
        test_port_assembly_reject_ports(!fail_chip);
        failed |= test_port_assembly_fdc_try(board, media, &request, fail_chip) !=
            LIB_STATUS_NO_MEMORY;
        failed |= fail_chip ? test_port_assembly_submitted_port_count() != 0u :
            !test_port_assembly_ports_rejected(7u);
        test_port_assembly_reject_ports(LIB_FALSE);
        observed = test_port_assembly_board_capture(board);
        failed |= observed.fdc_configured || !observed.fdc_zero ||
            !observed.fdc_topology_zero || core_machine_dma_has_pending_request(dma);
        const lib_u16 reads[] = {0x3f4u, 0x3f5u, 0x3f7u, 0x3f0u};
        const lib_u16 writes[] = {0x3f2u, 0x3f5u, 0x3f7u};
        for (lib_size index = 0u; index < sizeof(reads) / sizeof(reads[0]); ++index)
            failed |= test_core_machine_fixture_expect_port(machine, reads[index], LIB_FALSE, LIB_FALSE);
        for (lib_size index = 0u; index < sizeof(writes) / sizeof(writes[0]); ++index)
            failed |= test_core_machine_fixture_expect_port(machine, writes[index], LIB_TRUE, LIB_FALSE);
        failed |= test_port_assembly_fdc_try(board, media, &request, LIB_FALSE) != LIB_STATUS_OK;
        observed = test_port_assembly_board_capture(board);
        failed |= !observed.fdc_configured;
    }
    x86_core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    return failed || port_assembly_fresh_default_create();
}

lib_i32 port_assembly_rtc_transaction(void)
{
    const core_machine_config machine_config = {.memory_bytes = 2u * 1024u * 1024u};
    const core_machine_rtc_cmos_config rtc_config = {
        .index_port = 0x70u, .data_port = 0x71u, .nmi_mask_bit = 0x80u,
        .irq = 8u, .ticks_per_second = 1000u, .default_count = 1u,
        .defaults = {{CORE_MACHINE_RTC_EQUIPMENT, 0x2fu}}
    };
    test_port_assembly_board_observation observed;
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        test_port_assembly_reject_ports(LIB_TRUE);
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_NO_MEMORY ||
            !test_port_assembly_ports_rejected(2u);
        test_port_assembly_reject_ports(LIB_FALSE);
        observed = test_port_assembly_board_capture(board);
        failed |= observed.rtc_configured || observed.rtc_present || !observed.rtc_config_zero ||
            test_core_machine_fixture_expect_port(machine, 0x70u, LIB_TRUE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x71u, LIB_FALSE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x71u, LIB_TRUE, LIB_FALSE);
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) != LIB_STATUS_OK;
        observed = test_port_assembly_board_capture(board);
        failed |= !observed.rtc_configured;
    }
    x86_core_machine_destroy(machine);
    return failed || port_assembly_fresh_default_create();
}

lib_i32 port_assembly_rtc_collision(void)
{
    const core_machine_config machine_config = {
        .memory_bytes = (2u * 1024u * 1024u)
    };
    const core_machine_rtc_cmos_config rtc_config = {
        .index_port = 0x0070u, .data_port = 0x0071u, .nmi_mask_bit = 0x80u,
        .irq = 8u, .ticks_per_second = 1000u
    };
    const x86_core_machine_port_route existing = {
        .address = 0x0071u, .read = port_assembly_read,
        .write = port_assembly_write
    };
    test_port_assembly_board_observation observed;
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        failed |= x86_core_machine_install_port_routes(machine, &existing, 1u) != LIB_STATUS_OK;
        failed |= core_machine_configure_rtc_cmos(board, &rtc_config) !=
                LIB_STATUS_INVALID_STATE;
        observed = test_port_assembly_board_capture(board);
        failed |= observed.rtc_present || observed.rtc_configured ||
            test_core_machine_fixture_expect_port(machine, 0x70u, LIB_TRUE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x71u, LIB_FALSE, LIB_TRUE) ||
            test_core_machine_fixture_expect_port(machine, 0x71u, LIB_TRUE, LIB_TRUE);
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static void port_assembly_parity_fault(void *owner, lib_u32 physical)
{
    (void)owner;
    (void)physical;
}

lib_i32 port_assembly_port_b_transaction(test_port_assembly_profile_attach attach)
{
    const core_machine_config config = {
        .memory_bytes = 512u * 1024u, .auxiliary_pit_present = LIB_TRUE,
        .auxiliary_pit_base_port = 0x48u
    };
    const core_machine_planar_parity_config parity = {
        .port = CORE_MACHINE_PC_AT_PORT_B, .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1
    };
    const x86_core_machine_memory_parity_config probe = {
        512u * 1024u, port_assembly_parity_fault
    };
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    test_port_assembly_board_observation observed;
    lib_u8 owner = 0u;
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        const core_machine_board_profile_binding incomplete = {0};
        failed |= core_machine_board_bind_profile(board, &incomplete) != LIB_STATUS_INVALID_ARGUMENT;
        observed = test_port_assembly_board_capture(board);
        failed |= observed.profile_bound;
        failed |= (attach != LIB_NULL ? attach(board, LIB_TRUE) :
            test_at_parity_try(board, &parity, LIB_TRUE)) != LIB_STATUS_NO_MEMORY;
        observed = test_port_assembly_board_capture(board);
        failed |= observed.parity_bound || observed.profile_bound ||
            test_core_machine_fixture_expect_port(machine, CORE_MACHINE_PC_AT_PORT_B, LIB_FALSE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, CORE_MACHINE_PC_AT_PORT_B, LIB_TRUE, LIB_FALSE);
        /* The exclusive public claim proves failed AT construction released
         * parity storage; no outer test reads the private allocation pointer. */
        failed |= x86_core_machine_install_memory_device_routes(machine, LIB_NULL,
            0u, LIB_NULL, &probe, &owner) != LIB_STATUS_OK ||
            x86_core_machine_remove_memory_device_routes(machine, &owner) != LIB_STATUS_OK;
        failed |= (attach != LIB_NULL ? attach(board, LIB_FALSE) :
            test_at_parity_try(board, &parity, LIB_FALSE)) != LIB_STATUS_OK ||
            test_core_machine_fixture_expect_port(machine, CORE_MACHINE_PC_AT_PORT_B, LIB_FALSE, LIB_TRUE) ||
            test_core_machine_fixture_expect_port(machine, CORE_MACHINE_PC_AT_PORT_B, LIB_TRUE, LIB_TRUE);
        if (!failed) failed |= test_port_assembly_refresh_count(board) ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            test_port_assembly_refresh_count(board) ||
            x86_core_machine_bus_write(machine, 0x43u, 0x74u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x41u, 2u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x41u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            test_port_assembly_refresh_count(board);
    }
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 port_assembly_hdc_transaction(core_machine_hdc_protocol protocol,
    lib_bool fail_chip, lib_bool busy_dma, lib_bool busy_port)
{
    const core_machine_config machine_config = {.memory_bytes = 2u * 1024u * 1024u};
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_bus *dma = LIB_NULL;
    test_port_assembly_board_observation observed;
    const core_machine_dma_channel_provider blocker = {0};
    core_machine_dma_request_binding blocker_binding = {0};
    port_assembly_probe_state conflict = {0};
    const x86_core_machine_port_route conflict_route = {
        .address = 0x1f3u, .read = port_assembly_read, .owner = &conflict
    };
    x86_core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    const lib_bool xebec = protocol == CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
    const lib_bool compaq = protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
    const lib_u16 first_port = xebec ? 0x320u : 0x1f0u;
    const lib_u16 last_port = xebec ? 0x323u : 0x1f7u;
    lib_i32 failed = core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&machine_config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) failed |= test_port_assembly_hdc_setup(board, media, protocol,
        &dma) != LIB_STATUS_OK;
    if (!failed && xebec && busy_dma) failed |= core_machine_dma_bind_channel(
        dma, 3u, &blocker, &blocker_binding, &blocker_binding) != LIB_STATUS_OK;
    if (!failed && busy_port) failed |= x86_core_machine_install_port_routes(machine,
        &conflict_route, 1u) != LIB_STATUS_OK;
    if (!failed) {
        const lib_bool reject_publication = !fail_chip && !busy_dma && !busy_port;

        test_port_assembly_reject_ports(reject_publication);
        failed |= test_port_assembly_hdc_try(board, media, protocol, fail_chip) !=
            (busy_dma || busy_port ? LIB_STATUS_INVALID_STATE : LIB_STATUS_NO_MEMORY);
        if (reject_publication)
            failed |= !test_port_assembly_ports_rejected(xebec ? 4u : compaq ? 10u : 9u);
        if (fail_chip) failed |= test_port_assembly_submitted_port_count() != 0u;
        test_port_assembly_reject_ports(LIB_FALSE);
        observed = test_port_assembly_board_capture(board);
        failed |= observed.hdc_configured || !observed.hdc_zero ||
            !observed.hdc_topology_zero ||
            test_core_machine_fixture_expect_port(machine, 0x1f0u, LIB_FALSE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x1f0u, LIB_TRUE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x3f6u, LIB_FALSE, LIB_FALSE) ||
            test_core_machine_fixture_expect_port(machine, 0x3f6u, LIB_TRUE, LIB_FALSE);
        for (lib_u16 port = first_port; port <= last_port; ++port)
            failed |= test_core_machine_fixture_expect_port(machine, port, LIB_FALSE,
                busy_port && port == conflict_route.address) ||
                test_core_machine_fixture_expect_port(machine, port, LIB_TRUE, LIB_FALSE);
        if (compaq) failed |= !observed.fdc_configured ||
            test_core_machine_fixture_expect_port(machine, 0x3f7u, LIB_FALSE, LIB_TRUE);
        if (xebec) failed |= !observed.hdc_dma_unbound ||
            core_machine_dma_has_pending_request(dma);
        if (busy_dma) {
            core_machine_dma_request_binding duplicate = {0};
            failed |= blocker_binding.core_token == 0u ||
                core_machine_dma_bind_channel(dma, 3u,
                    &blocker, &duplicate, &duplicate) != LIB_STATUS_INVALID_STATE;
        }
        if (busy_port) failed |= x86_core_machine_remove_port_routes(machine,
            &conflict) != LIB_STATUS_OK;
        if (!busy_dma) {
            failed |= test_port_assembly_hdc_try(board, media, protocol,
                LIB_FALSE) != LIB_STATUS_OK;
            observed = test_port_assembly_board_capture(board);
            failed |= !observed.hdc_configured;
            if (xebec) failed |= test_core_machine_fixture_expect_port(machine, 0x323u,
                LIB_FALSE, LIB_FALSE) ||
                test_core_machine_fixture_expect_port(machine, 0x323u, LIB_TRUE, LIB_TRUE);
            if (compaq && !failed) {
                lib_u32 fdc_value = 0u, hdc_value = 0u, combined = 0u;

                failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
                    x86_core_machine_reset(machine) != LIB_STATUS_OK ||
                    test_port_assembly_fdc_address_read(board, &fdc_value) != LIB_STATUS_OK ||
                    test_port_assembly_hdc_address_read(board, &hdc_value) != LIB_STATUS_OK ||
                    x86_core_machine_bus_read(machine, 0x3f7u, &combined) != LIB_STATUS_OK ||
                    combined != (fdc_value | hdc_value);
            }
        }
    }
    x86_core_machine_destroy(machine);
    core_machine_media_registry_destroy(media);
    if (failed) lib_c_fprintf(lib_c_stderr, "HDC rollback protocol=%u chip=%u\n",
        (lib_u32)protocol, (lib_u32)fail_chip);
    return failed || port_assembly_fresh_default_create();
}
