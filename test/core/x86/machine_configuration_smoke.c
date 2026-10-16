#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/x86/machine_interface.h"
#include "../support/memory_alias_fixture.h"

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

static lib_i32 machine_address_decode_contract(void)
{
    const core_machine_executor_config identity_config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    core_machine_executor_config gated_config = {
        .memory_bytes = 4096u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .address_decode = {0x100u, 0x1ffu, 0x10u}
    };
    core_machine *identity = LIB_NULL;
    core_machine *gated = LIB_NULL;
    core_machine *rejected;
    const lib_u8 first = 0x55u, second = 0x66u, third = 0x77u;
    lib_u8 value = 0u;
    lib_u32 clear_mask;
    lib_i32 failed = 1;

    if (core_machine_neutral_create(&identity_config, &identity) != LIB_STATUS_OK)
        goto done;
    rejected = identity;
    gated_config.address_decode.first = 0x200u;
    if (core_machine_neutral_create(&gated_config, &rejected) !=
            LIB_STATUS_INVALID_ARGUMENT || rejected != LIB_NULL) goto done;
    gated_config.address_decode.first = 0x100u;
    if (core_machine_neutral_create(&gated_config, &gated) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(identity) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(gated) != LIB_STATUS_OK ||
        core_machine_reset(identity) != LIB_STATUS_OK ||
        core_machine_reset(gated) != LIB_STATUS_OK) goto done;
    /* No board decode means identity, even above the old implicit PC gate. */
    if (core_machine_memory_write(identity, 0u, &first, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(identity, 0x100000u, &second, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(identity, 0u, &value, 1u) != LIB_STATUS_OK ||
        value != first) goto done;
    if (core_machine_memory_write(gated, 0x100u, &first, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        core_machine_memory_write(gated, 0x1efu, &second, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(gated, 0x1ffu, &value, 1u) != LIB_STATUS_OK ||
        value != second ||
        core_machine_memory_write(gated, 0x200u, &second, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(gated, 0x210u, &third, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(gated, 0x200u, &value, 1u) != LIB_STATUS_OK ||
        value != second) goto done;
    if (core_machine_set_address_clear_mask(gated, 0u) != LIB_STATUS_OK ||
        core_machine_memory_write(gated, 0x110u, &third, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(gated, 0x100u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        core_machine_signal_address_clear_mask(gated, 0x10u) != LIB_STATUS_OK ||
        core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != first ||
        core_machine_set_address_clear_mask(gated, 0u) != LIB_STATUS_OK ||
        core_machine_reset(gated) != LIB_STATUS_OK ||
        core_machine_observe_address_clear_mask(gated, &clear_mask) != LIB_STATUS_OK ||
        clear_mask != 0x10u ||
        core_machine_memory_read(gated, 0x110u, &value, 1u) != LIB_STATUS_OK ||
        value != 0u) goto done;
    failed = 0;
done:
    core_machine_destroy(gated);
    core_machine_destroy(identity);
    return failed;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_executor_config config = { .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386 };
    core_machine_port_provider port_provider = {
        machine_configuration_port_read, LIB_NULL
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    lib_u8 halt = 0xf4u;
    lib_i32 failed = 0;

    failed |= machine_configuration_expect(core_machine_neutral_create(&config, &machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x300u, 0x300u, &port_provider, LIB_NULL), LIB_STATUS_OK);
    failed |= machine_configuration_expect(test_core_machine_fixture_register_reset_mapping(
        machine, 0xfffffff0u, 0x000ffff0u, 16u), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_freeze_execution_providers(
        machine), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_install_port_provider(
        machine, 0x301u, 0x301u, &port_provider, LIB_NULL),
        LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_bind_execution_provider(
        machine, LIB_NULL, LIB_NULL), LIB_STATUS_INVALID_STATE);
    failed |= machine_configuration_expect(core_machine_reset(machine),
        LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_run(machine, budget,
        &run_result), LIB_STATUS_OK);
    failed |= machine_configuration_expect(core_machine_memory_write(machine,
        0xffff0u, &halt, 1u), LIB_STATUS_OK);

    core_machine_destroy(machine);
    if (failed != 0 || machine_address_decode_contract()) return 1;
    lib_c_printf("LIFECYCLE-CONTRACT:OK\n");
    return 0;
}
