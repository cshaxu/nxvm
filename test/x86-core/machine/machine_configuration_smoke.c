#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/machine.h"
#include "support/memory_alias_fixture.h"

static lib_status machine_configuration_port_read(void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    (void)owner;
    (void)port;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_i32 machine_configuration_expect(lib_status actual,
    lib_status expected)
{
    return actual == expected ? 0 : 1;
}

static lib_i32 machine_allocation_contract(void)
{
    const lib_size sizes[] = {2u * 1024u * 1024u, 16u * 1024u * 1024u};
    lib_i32 failed = 0;

    for (lib_size index = 0u; index < sizeof(sizes) / sizeof(sizes[0]); ++index) {
        const x86_core_machine_executor_config config = {
            .memory_bytes = sizes[index],
            .cpu_profile = X86_CORE_CPU_PROFILE_80386
        };
        for (lib_size reject = 0u; reject <= 1u; ++reject) {
            x86_core_machine_memory_test_allocation allocation = {
                reject != 0u, 0u
            };
            x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
            const lib_status status = x86_core_machine_neutral_create_with_test_allocation(
                &config, &allocation, LIB_NULL, &machine);

            failed |= allocation.attempts != 1u;
            if (reject != 0u) {
                failed |= status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL;
            } else {
                lib_size installed = 0u;
                failed |= status != LIB_STATUS_OK || machine == LIB_NULL;
                if (status == LIB_STATUS_OK) {
                    failed |= x86_core_machine_get_memory_bytes(machine, &installed) !=
                        LIB_STATUS_OK || installed != sizes[index];
                    x86_core_machine_destroy(machine);
                }
            }
        }
    }
    {
        const x86_core_machine_executor_config config = {
            .cpu_profile = X86_CORE_CPU_PROFILE_80386
        };
        x86_core_machine_memory_test_allocation allocation = {LIB_TRUE, 0u};
        x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;

        failed |= x86_core_machine_neutral_create_with_test_allocation(&config,
            &allocation, LIB_NULL, &machine) != LIB_STATUS_INVALID_ARGUMENT ||
            machine != LIB_NULL || allocation.attempts != 0u;
    }
    return failed;
}

static lib_i32 machine_address_decode_contract(void)
{
    const x86_core_machine_executor_config identity_config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    x86_core_machine_executor_config gated_config = {
        .memory_bytes = 4096u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .address_decode = {0x100u, 0x1ffu, 0x10u}
    };
    x86_core_machine *identity = LIB_NULL;
    x86_core_machine *gated = LIB_NULL;
    x86_core_machine *rejected;
    const lib_u8 first = 0x55u, second = 0x66u, third = 0x77u;
    lib_u8 value = 0u;
    lib_u32 clear_mask;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&identity_config, &identity) != LIB_STATUS_OK)
        goto done;
    rejected = identity;
    gated_config.address_decode.first = 0x200u;
    if (x86_core_machine_neutral_create(&gated_config, &rejected) !=
            LIB_STATUS_INVALID_ARGUMENT || rejected != LIB_NULL) goto done;
    gated_config.address_decode.first = 0x100u;
    if (x86_core_machine_neutral_create(&gated_config, &gated) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(identity) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(gated) != LIB_STATUS_OK ||
        x86_core_machine_reset(identity) != LIB_STATUS_OK ||
        x86_core_machine_reset(gated) != LIB_STATUS_OK) goto done;
    /* No board decode means identity, even above the old implicit PC gate. */
    if (x86_core_machine_memory_write(identity, 0u, &first, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(identity, 0x100000u, &second, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(identity, 0u, &value, 1u) != LIB_STATUS_OK ||
        value != first) goto done;
    if (x86_core_machine_memory_write(gated, 0x100u, &first, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        x86_core_machine_memory_write(gated, 0x1efu, &second, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(gated, 0x1ffu, &value, 1u) != LIB_STATUS_OK ||
        value != second ||
        x86_core_machine_memory_write(gated, 0x200u, &second, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(gated, 0x210u, &third, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(gated, 0x200u, &value, 1u) != LIB_STATUS_OK ||
        value != second) goto done;
    if (x86_core_machine_set_address_clear_mask(gated, 0u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(gated, 0x110u, &third, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(gated, 0x100u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        x86_core_machine_signal_address_clear_mask(gated, 0x10u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        x86_core_machine_set_address_clear_mask(gated, 0u) != LIB_STATUS_OK ||
        x86_core_machine_reset(gated) != LIB_STATUS_OK ||
        x86_core_machine_observe_address_clear_mask(gated, &clear_mask) != LIB_STATUS_OK ||
        clear_mask != 0x10u ||
        x86_core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != 0u) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(gated);
    x86_core_machine_destroy(identity);
    return failed;
}

/* Synthetic construction values prove copying, not hardware timing eligibility. */
static lib_i32 machine_transaction_configuration_copy(void)
{
    lib_i32 failed = 0;

    for (lib_u32 mode = 0u; mode < 2u; ++mode) {
        x86_core_machine_executor_config config = {
            .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80386,
            .retirement_time_contract = mode == 0u ?
                X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC : X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL,
            .time_axis = {X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 1000u},
            .transaction_contract = {
                .external_cycle_timing = {8u, 2u, 1u,
                    X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL, 0u, 0xffu},
                .dma_cycle_wait_quanta = 3u,
                .dma_cycle_bus_ready_gate_enabled = LIB_TRUE,
                .cpu_cycle_bus_ready_gate_enabled = LIB_TRUE,
                .cpu_prefetch_reservation_enabled = LIB_TRUE,
                .refresh_cycle_source_ticks = 7u
            }
        };
        x86_core_machine *machine = LIB_NULL;
        for (lib_size index = 0u; index < X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY; ++index) {
            config.transaction_contract.external_access_wait_windows[index] =
                (x86_core_machine_external_access_wait_window){
                    index % 2u ? X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT : X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY,
                    (lib_u32)(0x100u + 16u * index), (lib_u32)(0x10fu + 16u * index),
                    (lib_u32)index + 1u
                };
        }
        if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
        const x86_core_machine_transaction_contract *actual = &machine->transaction_contract;
        const x86_core_machine_transaction_contract *expected = &config.transaction_contract;
        failed |= machine->retirement_time_contract != config.retirement_time_contract ||
            actual->external_cycle_timing.page_bytes != expected->external_cycle_timing.page_bytes ||
            actual->external_cycle_timing.page_miss_ticks != expected->external_cycle_timing.page_miss_ticks ||
            actual->external_cycle_timing.page_hit_ticks != expected->external_cycle_timing.page_hit_ticks ||
            actual->external_cycle_timing.overlap_policy != expected->external_cycle_timing.overlap_policy ||
            actual->external_cycle_timing.first_eligible_address != expected->external_cycle_timing.first_eligible_address ||
            actual->external_cycle_timing.last_eligible_address != expected->external_cycle_timing.last_eligible_address ||
            actual->dma_cycle_wait_quanta != expected->dma_cycle_wait_quanta ||
            actual->dma_cycle_bus_ready_gate_enabled != LIB_TRUE ||
            actual->cpu_cycle_bus_ready_gate_enabled != LIB_TRUE ||
            actual->cpu_prefetch_reservation_enabled != LIB_TRUE ||
            actual->refresh_cycle_source_ticks != expected->refresh_cycle_source_ticks ||
            !machine->dma_cycle_bus_ready || !machine->cpu_cycle_bus_ready;
        for (lib_size index = 0u; index < X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY; ++index) {
            const x86_core_machine_external_access_wait_window *a = &actual->external_access_wait_windows[index];
            const x86_core_machine_external_access_wait_window *e = &expected->external_access_wait_windows[index];
            failed |= a->space != e->space || a->first_address != e->first_address ||
                a->last_address != e->last_address || a->wait_ticks != e->wait_ticks;
        }
        config.transaction_contract.dma_cycle_wait_quanta = 0u;
        failed |= actual->dma_cycle_wait_quanta != 3u;
        x86_core_machine_destroy(machine);
    }
    return failed;
}

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    x86_core_machine_port_provider port_provider = {
        machine_configuration_port_read, LIB_NULL
    };
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result run_result;
    lib_u8 halt = 0xf4u;
    lib_i32 failed = 0;

    failed |= machine_configuration_expect(x86_core_machine_neutral_create(&config, &machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_reset(machine),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(x86_core_machine_install_port_provider(
        machine, 0x300u, 0x300u, &port_provider, LIB_NULL), LIB_STATUS_OK);
    failed |= machine_configuration_expect(test_core_machine_fixture_register_reset_mapping(
        machine, 0xfffffff0u, 0x000ffff0u, 16u), LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_freeze_execution_providers(
        machine), LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_freeze_execution_providers(
        machine), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(x86_core_machine_install_port_provider(
        machine, 0x301u, 0x301u, &port_provider, LIB_NULL),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(x86_core_machine_bind_execution_provider(
        machine, LIB_NULL, LIB_NULL), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(x86_core_machine_reset(machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_run(machine, budget,
        &run_result), LIB_STATUS_OK);
    failed |= machine_configuration_expect(x86_core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);

    x86_core_machine_destroy(machine);
    if (failed != 0 || machine_address_decode_contract() ||
        machine_allocation_contract() || machine_transaction_configuration_copy()) return 1;
    lib_c_printf("LIFECYCLE-CONTRACT:OK\n");
    return 0;
}
