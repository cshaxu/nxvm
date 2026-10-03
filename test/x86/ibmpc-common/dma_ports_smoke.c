#include "x86/core/machine_interface.h"
#include "x86/ibmpc-common/dma_bus_interface.h"

static lib_i32 check_bus(lib_u8 controllers)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_dma_channel_provider provider = {0};
    core_machine *machine = LIB_NULL;
    core_machine_dma_bus *bus = LIB_NULL;
    core_machine_dma_request_binding request = {0};
    core_machine_dma_request_binding duplicate = {0};
    lib_u32 value = 0u;
    lib_i32 failed = 1;
    lib_u8 owner = 0u;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_dma_initialize(&bus, machine, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        bus != LIB_NULL ||
        core_machine_dma_initialize(&bus, machine, controllers) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 2u, &provider, &owner, &request) != LIB_STATUS_OK ||
        core_machine_dma_bind_channel(bus, 2u, &provider, &owner, &duplicate) != LIB_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    if (core_machine_bus_write(machine, 0x81u, 0x12u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x81u, &value) != LIB_STATUS_OK ||
        value != 0x12u ||
        core_machine_bus_write(machine, 0x0bu, 0x46u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0au, 2u) != LIB_STATUS_OK) goto done;
    if (controllers == 2u &&
        (core_machine_bus_write(machine, 0xd6u, 0xc0u) != LIB_STATUS_OK ||
         core_machine_bus_write(machine, 0xd4u, 0u) != LIB_STATUS_OK)) goto done;
    if (core_machine_bus_read(machine, 0xd0u, &value) !=
        (controllers == 2u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED)) goto done;
    core_machine_dma_request_assert(bus, &request);
    if (!core_machine_dma_has_pending_request(bus) ||
        core_machine_dma_get_signals(bus, 0u).requests != 4u) goto done;
    core_machine_dma_request_deassert(bus, &request);
    if (core_machine_dma_has_pending_request(bus)) goto done;
    core_machine_dma_reset(bus);
    if (core_machine_bus_read(machine, 0x81u, &value) != LIB_STATUS_OK ||
        value != 0u || request.core_token == 0u) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    core_machine_dma_finalize(bus);
    return failed;
}

lib_i32 main(void)
{
    return check_bus(1u) | check_bus(2u);
}
