#include "app-mypc110/pc110_config_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/machine_interface.h"

static lib_bool config_route_execute(x86_core_machine *machine,
    const lib_u8 *code, lib_size bytes, lib_u64 instructions)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0u,
            [X86_CORE_CPU_DEBUG_EIP] = 0x1000u}
    };
    x86_core_machine_run_result result = {0};

    if (x86_core_machine_memory_write(machine, 0x1000u, code, bytes) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK)
        return LIB_FALSE;
    for (lib_u64 index = 0u; index < instructions; ++index) {
        if (x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.reason != X86_CORE_MACHINE_STOP_BUDGET)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    const lib_u8 unlock[] = {0xb8u, 0x00u, 0x80u, 0xe6u, 0x23u,
        0x86u, 0xe0u, 0xe6u, 0x22u, 0xe7u, 0x22u};
    const lib_u8 byte_only[] = {0xb0u, 0x00u, 0xe6u, 0x23u,
        0xb0u, 0x80u, 0xe6u, 0x22u, 0xe6u, 0x22u};
    const lib_u8 lock[] = {0xb8u, 0x00u, 0x01u, 0xe7u, 0x22u};
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    x86_core_machine *machine = LIB_NULL;
    x86_vl82c420 *chip = LIB_NULL;
    mypc110_firmware firmware = {0};
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    const lib_u8 patch = 0x5au;
    const lib_u8 locked_patch = 0xa5u;
    const lib_u8 instruction_patch = 0x90u;
    x86_core_machine_run_result result = {0};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0xc000u,
            [X86_CORE_CPU_DEBUG_EIP] = 0u}
    };
    lib_bool failed = LIB_FALSE;

    bios[0u] = 0x55u;
    bios[MYPC110_VIDEO_SHADOW_BYTES] = 0x33u;
    bios[2u * MYPC110_FIRMWARE_BANK_BYTES] = 0xf4u;
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        mypc110_firmware_create(bios, sizeof(bios), &firmware) != LIB_STATUS_OK ||
        x86_vl82c420_create(&chip) != LIB_STATUS_OK ||
        mypc110_config_initialize_cold_image(chip) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine,
            mypc110_firmware_provider(), &firmware) != LIB_STATUS_OK ||
        mypc110_firmware_install_routes(machine, &firmware) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_OK ||
        mypc110_config_install_routes(machine, &firmware, chip) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            !config_route_execute(machine, byte_only, sizeof(byte_only), 5u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            !config_route_execute(machine, unlock, sizeof(unlock), 5u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x81u ||
            x86_core_machine_bus_write(machine, 0x0022u, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x0bu ||
            !config_route_execute(machine, lock, sizeof(lock), 2u) ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0x0100u ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x53u ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7bu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x4cu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0x29u ||
            x86_core_machine_bus_write(machine, 0x00f9u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0x7fu ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0xbau ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x00u ||
            !config_route_execute(machine, lock, sizeof(lock), 2u) ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x80u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0076u, 0x5au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0074u, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x5au ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x29u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0x29u ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0024u, 0xb8u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x0025u, 0x6cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0x6cu ||
            !config_route_execute(machine, lock, sizeof(lock), 2u) ||
            x86_core_machine_bus_read(machine, 0x0025u, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xf4u ||
            x86_core_machine_memory_read(machine, 0x000e0000u,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xf4u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x00u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != patch ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x20u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &locked_patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != patch ||
            x86_core_machine_memory_read(machine,
                MYPC110_VIDEO_PHYSICAL_START + MYPC110_VIDEO_SHADOW_BYTES,
                &byte, 1u) != LIB_STATUS_OK || byte != 0x33u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x0cu) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00edu, 0x00u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, MYPC110_VIDEO_PHYSICAL_START,
                &instruction_patch, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != instruction_patch ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            x86_core_machine_bus_read(machine, 0xfc23u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xf023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xc023u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0x7fu ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0022u, &value) != LIB_STATUS_OK ||
            value != 0u ||
            x86_core_machine_bus_read(machine, 0x0023u, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x0074u, 0x7au) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_OK ||
            value != 0x53u ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xffu ||
            x86_core_machine_bus_write(machine, 0x00fbu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x00ecu, 0x03u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x00edu, &value) != LIB_STATUS_OK ||
            value != 0xccu ||
            x86_core_machine_memory_read(machine, MYPC110_VIDEO_PHYSICAL_START,
                &byte, 1u) != LIB_STATUS_OK || byte != 0xf4u;
    }
    if (machine != LIB_NULL && chip != LIB_NULL) {
        failed |= x86_core_machine_remove_port_routes(machine, &firmware) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0076u, &value) != LIB_STATUS_UNSUPPORTED;
    }
    x86_core_machine_destroy(machine);
    mypc110_firmware_destroy(&firmware);
    x86_vl82c420_destroy(chip);
    return failed ? 1 : 0;
}
