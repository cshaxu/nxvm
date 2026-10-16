#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/firmware_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/rom_mapping_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

typedef struct firmware_probe {
    x86_core_machine *machine;
    x86_core_machine_firmware_context *expired;
    lib_i32 configure_calls;
    lib_i32 reset_calls;
    lib_i32 after_run_calls;
    lib_i32 reentry_rejected;
    lib_status port_status;
    lib_u32 port_value;
    lib_u64 read_tick;
} firmware_probe;

static lib_status firmware_probe_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{ firmware_probe *probe = owner; (void)port; probe->read_tick = tick; if (probe->port_status != LIB_STATUS_OK) return probe->port_status; *out_value = probe->port_value; return LIB_STATUS_OK; }
static lib_status firmware_probe_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{ firmware_probe *probe = owner; (void)port; if (probe->port_status != LIB_STATUS_OK) return probe->port_status; probe->port_value = value; return LIB_STATUS_OK; }

typedef struct firmware_failed_probe {
    x86_core_machine_firmware_context *expired;
    lib_i32 configure_calls;
} firmware_failed_probe;

static lib_status firmware_probe_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    const lib_u8 code[] = { 0x90u, 0xf4u, 0x5au, 0xa5u };
    const lib_u8 reset_code[16u] = { 0x90u };
    lib_status status;

    if (probe == LIB_NULL || firmware == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    ++probe->configure_calls;
    probe->reentry_rejected =
        x86_core_machine_reset(probe->machine) == LIB_STATUS_INVALID_STATE &&
        x86_core_machine_register_immutable_rom_mapping(probe->machine, 0xffff0u,
            code, sizeof(code)) == LIB_STATUS_INVALID_STATE &&
        x86_core_machine_register_immutable_rom_mapping_alias(probe->machine,
            0xd0000u, 0xc0000u, sizeof(code)) == LIB_STATUS_INVALID_STATE;
    status = x86_core_machine_firmware_register_immutable_rom(firmware, 0xe0000u,
        code, sizeof(code));
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_firmware_register_immutable_rom_alias(firmware,
        0xe0001u, 0xf0000u, 2u);
    if (status != LIB_STATUS_OK) return status;
    /* A complete ordinary reset window lets Core derive the CPU-owned high
     * reset alias.  The test's E0000h source still owns its own bytes. */
    return x86_core_machine_firmware_register_immutable_rom(firmware, 0xffff0u,
        reset_code, sizeof(reset_code));
}

static lib_status firmware_failed_probe_configure(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    firmware_failed_probe *probe = (firmware_failed_probe *)opaque;
    const lib_u8 code[] = { 0x90u, 0xf4u, 0x5au, 0xa5u };

    if (probe == LIB_NULL || firmware == LIB_NULL ||
        x86_core_machine_firmware_register_immutable_rom(firmware, 0xe0000u,
            code, sizeof(code)) != LIB_STATUS_OK ||
        x86_core_machine_firmware_register_immutable_rom_alias(firmware,
            0xe0001u, 0xf0000u, 2u) != LIB_STATUS_OK) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    ++probe->configure_calls;
    probe->expired = firmware;
    return LIB_STATUS_INTERNAL_ERROR;
}

static lib_status firmware_failed_probe_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    (void)opaque;
    (void)firmware;
    return LIB_STATUS_INTERNAL_ERROR;
}

static lib_status firmware_probe_reset(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    lib_u8 value = 0x5au;

    if (probe == LIB_NULL) return LIB_STATUS_INTERNAL_ERROR;
    ++probe->reset_calls;
    probe->expired = firmware;
    if (x86_core_machine_request_stop(probe->machine) != LIB_STATUS_INVALID_STATE) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    if (probe->port_status != LIB_STATUS_OK) {
        return x86_core_machine_firmware_port_write(firmware, 0x00e0u, 0x05u);
    }
    return x86_core_machine_firmware_memory_write(firmware, 0x500u, &value,
        sizeof(value));
}

static lib_status firmware_probe_after_run(void *opaque,
    x86_core_machine_firmware_context *firmware)
{
    firmware_probe *probe = (firmware_probe *)opaque;
    lib_u8 value = 0u;
    lib_u32 port_value = 0xdeadbeefu;

    if (probe == LIB_NULL || x86_core_machine_firmware_memory_read(firmware,
            0x500u, &value, sizeof(value)) != LIB_STATUS_OK || value != 0x5au) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    ++probe->after_run_calls;
    probe->port_status = LIB_STATUS_INTERNAL_ERROR;
    if (x86_core_machine_firmware_port_read(firmware, 0x00e0u, &port_value) !=
            LIB_STATUS_INTERNAL_ERROR || port_value != 0xdeadbeefu ||
        x86_core_machine_firmware_port_write(firmware, 0x00e0u, 0x05u) !=
            LIB_STATUS_INTERNAL_ERROR) return LIB_STATUS_INTERNAL_ERROR;
    probe->port_status = LIB_STATUS_OK;
    if (x86_core_machine_firmware_port_read(firmware, 0x00e0u, &port_value) !=
            LIB_STATUS_OK || port_value != probe->port_value ||
        x86_core_machine_firmware_port_write(firmware, 0x00e0u, 0x05u) !=
            LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    probe->expired = firmware;
    return x86_core_machine_firmware_port_write(firmware, 0x03d8u, 0x05u);
}

static const x86_core_machine_firmware_provider firmware_probe_provider = {
    firmware_probe_configure,
    firmware_probe_reset,
    firmware_probe_after_run
};

static const x86_core_machine_firmware_provider firmware_failed_probe_provider = {
    firmware_failed_probe_configure,
    firmware_failed_probe_reset,
    LIB_NULL
};

lib_i32 main(void)
{
    const x86_nxvm_board_config config = { .memory_bytes = (2u * 1024u * 1024u) };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result = {0};
    firmware_probe probe = {0};
    firmware_failed_probe failed_probe = {0};
    x86_core_machine_memory_route route = X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    const lib_u8 prior_rom = 0xebu;
    lib_u8 value = 0u;
    lib_u32 port_value = 0u;
    lib_i32 failed = 0;
    lib_status run_status;
    x86_core_machine_lifecycle lifecycle = X86_CORE_MACHINE_INITIALIZED;
    x86_core_machine_time_observation time = {0};
    x86_core_machine_port_provider port_provider = {firmware_probe_port_read,
        firmware_probe_port_write};

    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_register_immutable_rom_mapping(machine, 0xd0000u,
        &prior_rom, sizeof(prior_rom)) != LIB_STATUS_OK;
    failed |= x86_core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
        &port_provider, &probe) != LIB_STATUS_OK;
    failed |= x86_core_machine_bind_firmware_provider(machine,
        &firmware_failed_probe_provider, &failed_probe) != LIB_STATUS_INTERNAL_ERROR;
    failed |= failed_probe.configure_calls != 1;
    failed |= !x86_core_machine_immutable_rom_mapping_contains(machine,
        0xd0000u, sizeof(prior_rom)) ||
        x86_core_machine_immutable_rom_mapping_contains(machine,
            0xe0000u, sizeof(prior_rom)) ||
        x86_core_machine_immutable_rom_mapping_contains(machine,
            0xf0000u, sizeof(prior_rom));
    failed |= x86_core_machine_firmware_memory_read(failed_probe.expired, 0xe0000u,
        &value, sizeof(value)) != LIB_STATUS_INVALID_STATE;
    probe.machine = machine;
    failed |= x86_core_machine_bind_firmware_provider(machine,
        &firmware_probe_provider, &probe) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_bind_firmware_provider(machine,
        &firmware_probe_provider, &probe) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= probe.configure_calls != 1 || probe.reset_calls != 1 ||
        !probe.reentry_rejected;
    failed |= x86_core_machine_firmware_memory_read(probe.expired, 0x500u, &value,
        sizeof(value)) != LIB_STATUS_INVALID_STATE;
    failed |= x86_core_machine_memory_read(machine, 0xe0000u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0x90u;
    failed |= x86_core_machine_memory_read(machine, 0xf0000u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0xf4u;
    value = 0u;
    failed |= x86_core_machine_memory_read(machine, 0xe0001u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0xf4u;
    failed |= x86_core_machine_memory_read(machine, 0xf0001u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0x5au;
    /* The alias length is exactly two bytes: F0002h falls through to RAM. */
    failed |= x86_core_machine_memory_query(machine,
        0xf0002u, 1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    /* The original E0000h source remains the sole owner of its bytes. */
    failed |= x86_core_machine_memory_read(machine, 0xe0002u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0x5au;
    failed |= x86_core_machine_memory_read(machine, 0xe0003u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0xa5u;
    value = 0u;
    failed |= x86_core_machine_memory_write(machine, 0xf0000u, &value, sizeof(value)) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_memory_read(machine, 0xf0000u, &value, sizeof(value)) !=
        LIB_STATUS_OK || value != 0xf4u;
    failed |= x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_STOPPED;
    probe.port_status = LIB_STATUS_INTERNAL_ERROR;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_INTERNAL_ERROR;
    failed |= x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_INITIALIZED;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_INVALID_STATE;
    probe.port_status = LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_STOPPED;
    probe.port_status = LIB_STATUS_UNSUPPORTED;
    failed |= x86_core_machine_request_stop(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_UNSUPPORTED;
    failed |= x86_core_machine_get_lifecycle(machine, &lifecycle) != LIB_STATUS_OK ||
        lifecycle != X86_CORE_MACHINE_INITIALIZED;
    probe.port_status = LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    run_status = x86_core_machine_run(machine, budget, &result);
    failed |= run_status != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || probe.after_run_calls != 1 ||
        x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks == 0u || probe.read_tick != time.elapsed_ticks;
    failed |= x86_core_machine_firmware_port_read(probe.expired, 0x03d8u, &port_value) !=
        LIB_STATUS_INVALID_STATE;
    x86_core_machine_destroy(machine);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "firmware-probe failed-configure=%d configure=%d reset=%d after=%d reentry=%d life=%d run=%d reason=%d\\n",
            failed_probe.configure_calls,
            probe.configure_calls, probe.reset_calls, probe.after_run_calls,
            probe.reentry_rejected, (int)lifecycle, (int)run_status, (int)result.reason);
        return 1;
    }
    lib_c_printf("%s\n", "FIRMWARE-CAPABILITY:OK");
    lib_c_printf("%s\n", "ROM-ALIAS-LIFECYCLE:OK");
    lib_c_printf("%s\n", "FIRMWARE-FAILURE-PROPAGATION:OK");
    return 0;
}
